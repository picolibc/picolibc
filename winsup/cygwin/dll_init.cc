/* dll_init.cc

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "dll_init.h"
#include "environ.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "cygtls.h"
#include <wchar.h>
#include <sys/reent.h>

extern void __stdcall check_sanity_and_sync (per_process *);

dll_list dlls;

static bool dll_global_dtors_recorded;

/* Run destructors for all DLLs on exit. */
void
dll_global_dtors ()
{
  int recorded = dll_global_dtors_recorded;
  dll_global_dtors_recorded = false;
  if (recorded && dlls.start.next)
    for (dll *d = dlls.end; d != &dlls.start; d = d->prev)
      d->run_dtors ();
}

/* Run all constructors associated with a dll */
void
per_module::run_ctors ()
{
  void (**pfunc)() = ctors;

  /* Run ctors backwards, so skip the first entry and find how many
    there are, then run them.  */

  if (pfunc)
    {
      int i;
      for (i = 1; pfunc[i]; i++);

      for (int j = i - 1; j > 0; j--)
	(pfunc[j]) ();
    }
}

/* Run all destructors associated with a dll */
void
per_module::run_dtors ()
{
  void (**pfunc)() = dtors;
  for (int i = 1; pfunc[i]; i++)
    (pfunc[i]) ();
}

/* Initialize an individual DLL */
int
dll::init ()
{
  int ret = 1;

  /* Why didn't we just import this variable? */
  *(p.envptr) = __cygwin_environ;

  /* Don't run constructors or the "main" if we've forked. */
  if (!in_forkee)
    {
      /* global contructors */
      p.run_ctors ();

      /* entry point of dll (use main of per_process with null args...) */
      if (p.main)
	ret = (*(p.main)) (0, 0, 0);
    }

  return ret;
}

/* Look for a dll based on name */
dll *
dll_list::operator[] (const PWCHAR name)
{
  dll *d = &start;
  while ((d = d->next) != NULL)
    if (!wcscasecmp (name, d->name))
      return d;

  return NULL;
}

#define RETRIES 1000

/* Allocate space for a dll struct. */
dll *
dll_list::alloc (HINSTANCE h, per_process *p, dll_type type)
{
  WCHAR name[NT_MAX_PATH];
  DWORD namelen = GetModuleFileNameW (h, name, sizeof (name));

  /* Already loaded? */
  dll *d = dlls[name];
  if (d)
    {
      if (!in_forkee)
	d->count++;	/* Yes.  Bump the usage count. */
      return d;		/* Return previously allocated pointer. */
    }

  /* FIXME: Change this to new at some point. */
  d = (dll *) cmalloc (HEAP_2_DLL, sizeof (*d) + (namelen * sizeof (*name)));

  /* Now we've allocated a block of information.  Fill it in with the supplied
     info about this DLL. */
  d->count = 1;
  wcscpy (d->name, name);
  d->handle = h;
  d->has_dtors = true;
  d->p = p;
  d->type = type;
  if (end == NULL)
    end = &start;	/* Point to "end" of dll chain. */
  end->next = d;	/* Standard linked list stuff. */
  d->next = NULL;
  d->prev = end;
  end = d;
  tot++;
  if (type == DLL_LOAD)
    loaded_dlls++;
  return d;
}

dll *
dll_list::find (void *retaddr)
{
  MEMORY_BASIC_INFORMATION m;
  if (!VirtualQuery (retaddr, &m, sizeof m))
    return NULL;
  HMODULE h = (HMODULE) m.AllocationBase;

  dll *d = &start;
  while ((d = d->next))
    if (d->handle == h)
      break;
  return d;
}

/* Detach a DLL from the chain. */
void
dll_list::detach (void *retaddr)
{
  dll *d;
  if (!myself || exit_state || !(d = find (retaddr)))
    return;
  if (d->count <= 0)
    system_printf ("WARNING: trying to detach an already detached dll ...");
  if (--d->count == 0)
    {
      __cxa_finalize (d);
      d->run_dtors ();
      d->prev->next = d->next;
      if (d->next)
	d->next->prev = d->prev;
      if (d->type == DLL_LOAD)
	loaded_dlls--;
      if (end == d)
	end = d->prev;
      cfree (d);
    }
}

/* Initialization for all linked DLLs, called by dll_crt0_1. */
void
dll_list::init ()
{
  /* Walk the dll chain, initializing each dll */
  dll *d = &start;
  dll_global_dtors_recorded = d->next != NULL;
  while ((d = d->next))
    d->init ();
}

#define A64K (64 * 1024)

/* Mark every memory address up to "here" as reserved.  This may force
   Windows NT to load a DLL in the next available, lowest slot. */
static void
reserve_upto (const PWCHAR name, DWORD here)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;
  for (DWORD start = 0x10000; start < here; start += size)
    if (!VirtualQuery ((void *) start, &mb, sizeof (mb)))
      size = A64K;
    else
      {
	size = A64K * ((mb.RegionSize + A64K - 1) / A64K);
	start = A64K * (((DWORD) mb.BaseAddress + A64K - 1) / A64K);

	if (start + size > here)
	  size = here - start;
	if (mb.State == MEM_FREE &&
	    !VirtualAlloc ((void *) start, size, MEM_RESERVE, PAGE_NOACCESS))
	  api_fatal ("couldn't allocate memory %p(%d) for '%W' alignment, %E\n",
		     start, size, name);
      }
}

/* Release all of the memory previously allocated by "upto" above.
   Note that this may also free otherwise reserved memory.  If that becomes
   a problem, we'll have to keep track of the memory that we reserve above. */
static void
release_upto (const PWCHAR name, DWORD here)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;
  for (DWORD start = 0x10000; start < here; start += size)
    if (!VirtualQuery ((void *) start, &mb, sizeof (mb)))
      size = 64 * 1024;
    else
      {
	size = mb.RegionSize;
	if (!(mb.State == MEM_RESERVE && mb.AllocationProtect == PAGE_NOACCESS
	    && (((void *) start < cygheap->user_heap.base
		 || (void *) start > cygheap->user_heap.top)
		 && ((void *) start < (void *) cygheap
		     || (void *) start
			> (void *) ((char *) cygheap + CYGHEAPSIZE)))))
	  continue;
	if (!VirtualFree ((void *) start, 0, MEM_RELEASE))
	  api_fatal ("couldn't release memory %p(%d) for '%W' alignment, %E\n",
		     start, size, name);
      }
}

/* Reload DLLs after a fork.  Iterates over the list of dynamically loaded
   DLLs and attempts to load them in the same place as they were loaded in the
   parent. */
void
dll_list::load_after_fork (HANDLE parent)
{
  for (dll *d = &dlls.start; (d = d->next) != NULL; )
    if (d->type == DLL_LOAD)
      for (int i = 0; i < 2; i++)
	{
	  /* See if DLL will load in proper place.  If so, free it and reload
	     it the right way.
	     It stinks that we can't invert the order of the initial LoadLibrary
	     and FreeLibrar  since Microsoft documentation seems to imply that
	     should do what we want.  However, once a library is loaded as
	     above, the second LoadLibrary will not execute its startup code
	     unless it is first unloaded. */
	  HMODULE h = LoadLibraryExW (d->name, NULL, DONT_RESOLVE_DLL_REFERENCES);

	  if (!h)
	    system_printf ("can't reload %W, %E", d->name);
	  else
	    {
	      FreeLibrary (h);
	      if (h == d->handle)
		h = LoadLibraryW (d->name);
	    }
	  /* If we reached here on the second iteration of the for loop
	     then there is a lot of memory to release. */
	  if (i > 0)
	    release_upto (d->name, (DWORD) d->handle);
	  if (h == d->handle)
	    break;		/* Success */

	  if (i > 0)
	    /* We tried once to relocate the dll and it failed. */
	    api_fatal ("unable to remap %W to same address as parent: %p != %p",
		       d->name, d->handle, h);

	  /* Dll loaded in the wrong place.  Dunno why this happens but it
	     always seems to happen when there are multiple DLLs attempting to
	     load into the same address space.  In the "forked" process, the
	     second DLL always loads into a different location. So, block all
	     of the memory up to the new load address and try again. */
	  reserve_upto (d->name, (DWORD) d->handle);
	}
  in_forkee = false;
}

struct dllcrt0_info
{
  HMODULE h;
  per_process *p;
  int res;
  dllcrt0_info (HMODULE h0, per_process *p0): h(h0), p(p0) {}
};

extern "C" int
dll_dllcrt0 (HMODULE h, per_process *p)
{
  dllcrt0_info x (h, p);

  if (_my_tls.isinitialized ())
    dll_dllcrt0_1 (&x);
  else
    _my_tls.call ((DWORD (*) (void *, void *)) dll_dllcrt0_1, &x);
  return x.res;
}

void
dll_dllcrt0_1 (VOID *x)
{
  HMODULE& h = ((dllcrt0_info *)x)->h;
  per_process*& p = ((dllcrt0_info *)x)->p;
  int& res = ((dllcrt0_info *)x)->res;

  /* Make sure that our exception handler is installed.
     That should always be the case but this just makes sure.

     At some point, we may want to just remove this code since
     the exception handler should be guaranteed to be installed.
     I'm leaving it in until potentially after the release of
     1.7.1 */
  _my_tls.init_exception_handler (_cygtls::handle_exceptions);

  if (p == NULL)
    p = &__cygwin_user_data;
  else
    *(p->impure_ptr_ptr) = __cygwin_user_data.impure_ptr;

  bool linked = !in_forkee && !cygwin_finished_initializing;

  /* Broken DLLs built against Cygwin versions 1.7.0-49 up to 1.7.0-57
     override the cxx_malloc pointer in their DLL initialization code,
     when loaded either statically or dynamically.  Because this leaves
     a stale pointer into demapped memory space if the DLL is unloaded
     by a call to dlclose, we prevent this happening for dynamically
     loaded DLLS in dlopen by saving and restoring cxx_malloc around
     the call to LoadLibrary, which invokes the DLL's startup sequence.
     Modern DLLs won't even attempt to override the pointer when loaded
     statically, but will write their overrides directly into the
     struct it points to.  With all modern DLLs, this will remain the
     default_cygwin_cxx_malloc struct in cxx.cc, but if any broken DLLs
     are in the mix they will have overridden the pointer and subsequent
     overrides will go into their embedded cxx_malloc structs.  This is
     almost certainly not a problem as they can never be unloaded, but
     if we ever did want to do anything about it, we could check here to
     see if the pointer had been altered in the early parts of the DLL's
     startup, and if so copy back the new overrides and reset it here.
     However, that's just a note for the record; at the moment, we can't
     see any need to worry about this happening.  */

  /* Partially initialize Cygwin guts for non-cygwin apps. */
  if (dynamically_loaded && user_data->magic_biscuit == 0)
    dll_crt0 (p);
  else
    check_sanity_and_sync (p);

  dll_type type;

  /* If this function is called before cygwin has finished
     initializing, then the DLL must be a cygwin-aware DLL
     that was explicitly linked into the program rather than
     a dlopened DLL. */
  if (linked)
    type = DLL_LINK;
  else
    {
      type = DLL_LOAD;
      dlls.reload_on_fork = 1;
    }

  /* Allocate and initialize space for the DLL. */
  dll *d = dlls.alloc (h, p, type);

  /* If d == NULL, then something is broken.
     Otherwise, if we've finished initializing, it's ok to
     initialize the DLL.  If we haven't finished initializing,
     it may not be safe to call the dll's "main" since not
     all of cygwin's internal structures may have been set up. */
  if (!d || (!linked && !d->init ()))
    res = -1;
  else
    res = (DWORD) d;
}

/* OBSOLETE: This function is obsolete and will go away in the
   future.  Cygwin can now handle being loaded from a noncygwin app
   using the same entry point. */

extern "C" int
dll_noncygwin_dllcrt0 (HMODULE h, per_process *p)
{
  return dll_dllcrt0 (h, p);
}

extern "C" void
cygwin_detach_dll (dll *)
{
  HANDLE retaddr;
  if (_my_tls.isinitialized ())
    retaddr = (void *) _my_tls.retaddr ();
  else
    retaddr = __builtin_return_address (0);
  dlls.detach (retaddr);
}

extern "C" void
dlfork (int val)
{
  dlls.reload_on_fork = val;
}

/* Called from various places to update all of the individual
   ideas of the environ block.  Explain to me again why we didn't
   just import __cygwin_environ? */
void __stdcall
update_envptrs ()
{
  for (dll *d = dlls.istart (DLL_ANY); d; d = dlls.inext ())
    *(d->p.envptr) = __cygwin_environ;
  *main_environ = __cygwin_environ;
}
