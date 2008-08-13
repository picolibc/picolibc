/* dll_init.cc

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006  Red Hat, Inc.

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

extern void __stdcall check_sanity_and_sync (per_process *);

dll_list NO_COPY dlls;

static bool dll_global_dtors_recorded;

/* Run destructors for all DLLs on exit. */
void
dll_global_dtors ()
{
  int recorded = dll_global_dtors_recorded;
  dll_global_dtors_recorded = false;
  if (recorded && dlls.start.next)
    for (dll *d = dlls.end; d != &dlls.start; d = d->prev)
      d->p.run_dtors ();
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

/* Allocate space for a dll struct contiguous with the just-loaded dll. */
dll *
dll_list::alloc (HINSTANCE h, per_process *p, dll_type type)
{
  WCHAR name[NT_MAX_PATH];
  DWORD namelen = GetModuleFileNameW (h, name, sizeof (name));

  /* Already loaded? */
  dll *d = dlls[name];
  if (d)
    {
      d->count++;	/* Yes.  Bump the usage count. */
      return d;		/* Return previously allocated pointer. */
    }

  SYSTEM_INFO s1;
  GetSystemInfo (&s1);

  int i;
  void *s = p->bss_end;
  DWORD n;
  MEMORY_BASIC_INFORMATION m;
  /* Search for space after the DLL */
  for (i = 0; i <= RETRIES; i++, s = (char *) m.BaseAddress + m.RegionSize)
    {
      if (!VirtualQuery (s, &m, sizeof (m)))
	return NULL;	/* Can't do it. */
      if (m.State == MEM_FREE)
	{
	  /* Couldn't find any.  Uh oh.  FIXME: Issue an error? */
	  if (i == RETRIES)
	    return NULL;	/* Oh well.  Couldn't locate free space. */

	  /* Ensure that this is rounded to the nearest page boundary.
	     FIXME: Should this be ensured by VirtualQuery? */
	  n = (DWORD) m.BaseAddress;
	  DWORD r = n % s1.dwAllocationGranularity;

	  if (r)
	    n = ((n - r) + s1.dwAllocationGranularity);

	  /* First reserve the area of memory, then commit it. */
	  if (VirtualAlloc ((void *) n, sizeof (dll), MEM_RESERVE, PAGE_READWRITE))
	    d = (dll *) VirtualAlloc ((void *) n, sizeof (dll), MEM_COMMIT,
				      PAGE_READWRITE);
	  if (d)
	    break;
	}
    }

  /* Did we succeed? */
  if (d == NULL)
    {			/* Nope. */
#ifdef DEBUGGING
      system_printf ("VirtualAlloc failed, %E");
#endif
      __seterrno ();
      return NULL;
    }

  /* Now we've allocated a block of information.  Fill it in with the supplied
     info about this DLL. */
  d->count = 1;
  d->namelen = namelen;
  wcscpy (d->name, name);
  d->handle = h;
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

/* Detach a DLL from the chain. */
void
dll_list::detach (void *retaddr)
{
  if (!myself || exit_state)
    return;
  MEMORY_BASIC_INFORMATION m;
  if (!VirtualQuery (retaddr, &m, sizeof m))
    return;
  HMODULE h = (HMODULE) m.AllocationBase;

  dll *d = &start;
  while ((d = d->next))
    if (d->handle != h)
      continue;
    else if (d->count <= 0)
      system_printf ("WARNING: trying to detach an already detached dll ...");
    else if (--d->count == 0)
      {
	d->p.run_dtors ();
	d->prev->next = d->next;
	if (d->next)
	  d->next->prev = d->prev;
	if (d->type == DLL_LOAD)
	  loaded_dlls--;
	if (end == d)
	  end = d->prev;
	VirtualFree (d, 0, MEM_RELEASE);
	break;
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
	if (!(mb.State == MEM_RESERVE && mb.AllocationProtect == PAGE_NOACCESS &&
	    (((void *) start < cygheap->user_heap.base
	      || (void *) start > cygheap->user_heap.top) &&
	     ((void *) start < (void *) cygheap
	      | (void *) start > (void *) ((char *) cygheap + CYGHEAPSIZE)))))
	  continue;
	if (!VirtualFree ((void *) start, 0, MEM_RELEASE))
	  api_fatal ("couldn't release memory %p(%d) for '%W' alignment, %E\n",
		     start, size, name);
      }
}

/* Reload DLLs after a fork.  Iterates over the list of dynamically loaded DLLs
   and attempts to load them in the same place as they were loaded in the parent. */
void
dll_list::load_after_fork (HANDLE parent, dll *first)
{
  int try2 = 0;
  dll d;

  void *next = first;
  while (next)
    {
      DWORD nb;
      /* Read the dll structure from the parent. */
      if (!ReadProcessMemory (parent, next, &d, sizeof (dll), &nb) ||
	  nb != sizeof (dll))
	return;

      /* We're only interested in dynamically loaded dlls.
	 Hopefully, this function wouldn't even have been called unless
	 the parent had some of those. */
      if (d.type == DLL_LOAD)
	{
	  bool unload = true;
	  HMODULE h = LoadLibraryExW (d.name, NULL, DONT_RESOLVE_DLL_REFERENCES);

	  if (!h)
	    system_printf ("can't reload %W", d.name);
	  /* See if DLL will load in proper place.  If so, free it and reload
	     it the right way.
	     It sort of stinks that we can't invert the order of the FreeLibrary
	     and LoadLibrary since Microsoft documentation seems to imply that that
	     should do what we want.  However, since the library was loaded above,
	     the second LoadLibrary does not execute it's startup code unless it
	     is first unloaded. */
	  else if (h == d.handle)
	    {
	      if (unload)
		{
		  FreeLibrary (h);
		  LoadLibraryW (d.name);
		}
	    }
	  else if (try2)
	    api_fatal ("unable to remap %W to same address as parent(%p) != %p",
		       d.name, d.handle, h);
	  else
	    {
	      /* It loaded in the wrong place.  Dunno why this happens but it always
		 seems to happen when there are multiple DLLs attempting to load into
		 the same address space.  In the "forked" process, the second DLL always
		 loads into a different location. */
	      FreeLibrary (h);
	      /* Block all of the memory up to the new load address. */
	      reserve_upto (d.name, (DWORD) d.handle);
	      try2 = 1;		/* And try */
	      continue;		/*  again. */
	    }
	  /* If we reached here, and try2 is set, then there is a lot of memory to
	     release. */
	  if (try2)
	    {
	      release_upto (d.name, (DWORD) d.handle);
	      try2 = 0;
	    }
	}
      next = d.next;	/* Get the address of the next DLL. */
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

  /* Windows apparently installs a bunch of exception handlers prior to
     this function getting called and one of them may trip before cygwin
     gets to it.  So, install our own exception handler only.
     FIXME: It is possible that we may have to save state of the
     previous exception handler chain and restore it, if problems
     are noted. */
  _my_tls.init_exception_handler (_cygtls::handle_exceptions);

  if (p == NULL)
    p = &__cygwin_user_data;
  else
    *(p->impure_ptr_ptr) = __cygwin_user_data.impure_ptr;

  bool linked = !in_forkee && !cygwin_finished_initializing;

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

/* OBSOLETE: This function is obsolescent and will go away in the
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
    retaddr = (HANDLE) _my_tls.retaddr ();
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
