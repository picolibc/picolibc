/* dll_init.cc

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include "winsup.h"
#include "exceptions.h"
#include "dll_init.h"

extern void __stdcall check_sanity_and_sync (per_process *);

dll_list NO_COPY dlls;

static NO_COPY int in_forkee = 0;
/* local variables */

//-----------------------------------------------------------------------------

static int dll_global_dtors_recorded = 0;

/* Run destructors for all DLLs on exit. */
static void
dll_global_dtors()
{
  for (dll *d = dlls.istart (DLL_ANY); d; d = dlls.inext ())
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

      for (int j = i - 1; j > 0; j-- )
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
dll_list::operator[] (const char *name)
{
  dll *d = &start;
  while ((d = d->next) != NULL)
    if (strcasematch (name, d->name))
      return d;

  return NULL;
}

#define RETRIES 100

/* Allocate space for a dll struct after the just-loaded dll. */
dll *
dll_list::alloc (HINSTANCE h, per_process *p, dll_type type)
{
  char name[MAX_PATH + 1];
  DWORD namelen = GetModuleFileName (h, name, sizeof (name));

  /* Already loaded? */
  dll *d = dlls[name];
  if (d)
    {
      d->count++;	/* Yes.  Bump the usage count. */
      return d;		/* Return previously allocated pointer. */
    }

  int i;
  void *s = p->bss_end;
  MEMORY_BASIC_INFORMATION m;
  /* Search for space after the DLL */
  for (i = 0; i <= RETRIES; i++)
    {
      if (!VirtualQuery (s, &m, sizeof (m)))
	return NULL;	/* Can't do it. */
      if (m.State == MEM_FREE)
	break;
      s = (char *) m.BaseAddress + m.RegionSize;
    }

  /* Couldn't find any.  Uh oh.  FIXME: Issue an error? */
  if (i == RETRIES)
    return NULL; /* Oh well */

  SYSTEM_INFO s1;
  GetSystemInfo (&s1);

  /* Need to do the shared memory thing since W95 can't allocate in
     the shared memory region otherwise. */
  HANDLE h1 = CreateFileMapping (INVALID_HANDLE_VALUE, &sec_none_nih,
				 PAGE_READWRITE, 0, sizeof (dll), NULL);

  DWORD n = (DWORD) m.BaseAddress;
  n = ((n - (n % s1.dwAllocationGranularity)) + s1.dwAllocationGranularity);
  d = (dll *) MapViewOfFileEx (h1, FILE_MAP_WRITE, 0, 0, 0, (void *) n);
  CloseHandle (h1);

  /* Now we've allocated a block of information.  Fill it in with the supplied
     info about this DLL. */
  d->count = 1;
  d->namelen = namelen;
  strcpy (d->name, name);
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
dll_list::detach (dll *d)
{
  if (d->count <= 0)
    system_printf ("WARNING: try to detach an already detached dll ...\n");
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
      UnmapViewOfFile (d);
    }
}

/* Initialization called by dll_crt0_1. */
void
dll_list::init ()
{
  debug_printf ("here");
  /* Make sure that destructors are called on exit. */
  if (!dll_global_dtors_recorded)
    {
      atexit (dll_global_dtors);
      dll_global_dtors_recorded = 1;
    }

  /* Walk the dll chain, initializing each dll */
  dll *d = &start;
  while ((d = d->next))
    d->init ();
}

#define A64K (64 * 1024)

/* Mark every memory address up to "here" as reserved.  This may force
   Windows NT to load a DLL in the next available, lowest slot. */
static void
reserve_upto (const char *name, DWORD here)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;
  for (DWORD start = 0x10000; start < here; start += size)
    if (!VirtualQuery ((void *) start, &mb, sizeof (mb)))
      size = 64 * 1024;
    else
      {
	size = A64K * ((mb.RegionSize + A64K - 1) / A64K);
	start = A64K * (((DWORD) mb.BaseAddress + A64K - 1) / A64K);

	if (start + size > here)
	  size = here - start;
	if (mb.State == MEM_FREE &&
	    !VirtualAlloc ((void *) start, size, MEM_RESERVE, PAGE_NOACCESS))
	  api_fatal ("couldn't allocate memory %p(%d) for '%s' alignment, %E\n",
		     start, size, name);
      }
}

/* Release all of the memory previously allocated by "upto" above.
   Note that this may also free otherwise reserved memory.  If that becomes
   a problem, we'll have to keep track of the memory that we reserve above. */
static void
release_upto (const char *name, DWORD here)
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
	    ((void *) start < user_data->heapbase || (void *) start > user_data->heaptop)))
	  continue;
	if (!VirtualFree ((void *) start, 0, MEM_RELEASE))
	  api_fatal ("couldn't release memory %p(%d) for '%s' alignment, %E\n",
		     start, size, name);
      }
}

#define MAX_DLL_SIZE (sizeof (dll))
/* Reload DLLs after a fork.  Iterates over the list of dynamically loaded DLLs
   and attempts to load them in the same place as they were loaded in the parent. */
void
dll_list::load_after_fork (HANDLE parent, dll *first)
{
  in_forkee = 1;
  int try2 = 0;
  dll d;

  void *next = first; 
  while (next)
    {
      DWORD nb;
      /* Read the dll structure from the parent. */
      if (!ReadProcessMemory (parent, next, &d, MAX_DLL_SIZE, &nb) ||
	  nb != MAX_DLL_SIZE)
	return;
      /* We're only interested in dynamically loaded dlls.
	 Hopefully, this function wouldn't even have been called unless
	 the parent had some of those. */
      if (d.type == DLL_LOAD)
	{
	  HMODULE h = LoadLibraryEx (d.name, NULL, DONT_RESOLVE_DLL_REFERENCES);

	  /* See if DLL will load in proper place.  If so, free it and reload
	     it the right way.
	     It sort of stinks that we can't invert the order of the FreeLibrary
	     and LoadLibrary since Microsoft documentation seems to imply that that
	     should do what we want.  However, since the library was loaded above,
	     The second LoadLibrary does not execute it's startup code unless it
	     is first unloaded. */
	  if (h == d.handle)
	    {
	      FreeLibrary (h);
	      LoadLibrary (d.name);
	    }
	  else if (try2)
	    api_fatal ("unable to remap %s to same address as parent -- %p", d.name, h);
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
  in_forkee = 0;
}

extern "C" int
dll_dllcrt0 (HMODULE h, per_process *p)
{
  if (p == NULL)
    p = &__cygwin_user_data;
  else
    *(p->impure_ptr_ptr) = __cygwin_user_data.impure_ptr;

  /* Partially initialize Cygwin guts for non-cygwin apps. */
  if (dynamically_loaded && user_data->magic_biscuit == 0)
    dll_crt0 (p);

  if (p)
    check_sanity_and_sync (p);

  dll_type type;

  /* If this function is called before cygwin has finished
     initializing, then the DLL must be a cygwin-aware DLL
     that was explicitly linked into the program rather than
     a dlopened DLL. */
  if (!cygwin_finished_initializing)
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
  if (!d || (cygwin_finished_initializing && !d->init ()))
    return -1;
  
  return (DWORD) d;
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
cygwin_detach_dll (dll *d)
{
  dlls.detach (d);
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
  extern char ***main_environ;
  for (dll *d = dlls.istart (DLL_ANY); d; d = dlls.inext ())
    {
	*(d->p.envptr) = __cygwin_environ;
    }
  *main_environ = __cygwin_environ;
}
