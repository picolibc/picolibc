/* dll_init.cc

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "sync.h"
#include "dll_init.h"
#include "environ.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "child_info.h"
#include "cygtls.h"
#include "exception.h"
#include <wchar.h>
#include <sys/reent.h>
#include <assert.h>
#include <tls_pbuf.h>

extern void __stdcall check_sanity_and_sync (per_process *);

#define fabort fork_info->abort

dll_list dlls;

muto dll_list::protect;

static bool dll_global_dtors_recorded;

/* Run destructors for all DLLs on exit. */
void
dll_global_dtors ()
{
  /* Don't attempt to call destructors if we're still in fork processing
     since that likely means fork is failing and everything will not have been
     set up.  */
  if (in_forkee)
    return;
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
  while (*++pfunc)
    (*pfunc) ();
}

/* Initialize an individual DLL */
int
dll::init ()
{
  int ret = 1;

#ifdef __i386__
  /* This should be a no-op.  Why didn't we just import this variable? */
  if (!p.envptr)
    p.envptr = &__cygwin_environ;
  else if (*(p.envptr) != __cygwin_environ)
    *(p.envptr) = __cygwin_environ;
#endif

  /* Don't run constructors or the "main" if we've forked. */
  if (!in_forkee)
    {
      /* global contructors */
      p.run_ctors ();

      /* entry point of dll (use main of per_process with null args...) */
      if (p.main)
	ret = p.main (0, 0, 0);
    }

  return ret;
}

/* Look for a dll based on the full path.

   CV, 2012-03-04: Per MSDN, If a DLL with the same module name is already
   loaded in memory, the system uses the loaded DLL, no matter which directory
   it is in. The system does not search for the DLL.  See
   http://msdn.microsoft.com/en-us/library/ms682586%28v=vs.85%29.aspx

   On 2012-02-08 I interpreted "module name" as "basename".  So the assumption
   was that the Windows Loader does not load another DLL with the same basename,
   if one such DLL is already loaded.  Consequentially I changed the code so
   that DLLs are only compared by basename.

   This assumption was obviously wrong, as the perl dynaloader proves.  It
   loads multiple DLLs with the same basename into memory, just from different
   locations.  This mechanism is broken when only comparing basenames in the
   below code.

   However, the original problem reported on 2012-02-07 was a result of
   a subtil difference between the paths returned by different calls to
   GetModuleFileNameW: Sometimes the path is a plain DOS path, sometimes
   it's preceeded by the long pathname prefix "\\?\".

   So I reverted the original change from 2012-02-08 and only applied the
   following fix: Check if the path is preceeded by a long pathname prefix,
   and, if so, drop it forthwith so that subsequent full path comparisons
   work as expected.

   At least that was the original idea.  In fact there are two case, linked
   and runtime loaded DLLs, which have to be distinguished:

   - Linked DLLs are loaded by only specifying the basename of the DLL and
     searching it using the system DLL search order as given in the
     aforementioned MSDN URL.

   - Runtime loaded DLLs are specified with the full path since that's how
     dlopen works.

   In effect, we have to be careful not to mix linked and loaded DLLs.
   For more info how this gets accomplished, see the comments at the start
   of dll_list::alloc, as well as the comment preceeding the definition of
   the in_load_after_fork bool later in the file. */
dll *
dll_list::operator[] (const PWCHAR name)
{
  dll *d = &start;
  while ((d = d->next) != NULL)
    if (!wcscasecmp (name, d->name))
      return d;

  return NULL;
}

/* Look for a dll based on the basename. */
dll *
dll_list::find_by_modname (const PWCHAR modname)
{
  dll *d = &start;
  while ((d = d->next) != NULL)
    if (!wcscasecmp (modname, d->modname))
      return d;

  return NULL;
}

#define RETRIES 1000

/* Allocate space for a dll struct. */
dll *
dll_list::alloc (HINSTANCE h, per_process *p, dll_type type)
{
  /* Called under loader lock conditions so this function can't be called
     multiple times in parallel.  A static buffer is safe. */
  static WCHAR buf[NT_MAX_PATH];
  GetModuleFileNameW (h, buf, NT_MAX_PATH);
  PWCHAR name = buf;
  if (!wcsncmp (name, L"\\\\?\\", 4))
    {
      name += 4;
      if (!wcsncmp (name, L"UNC\\", 4))
	{
	  name += 2;
	  *name = L'\\';
	}
    }
  DWORD namelen = wcslen (name);
  PWCHAR modname = wcsrchr (name, L'\\') + 1;

  guard (true);
  /* Already loaded?  For linked DLLs, only compare the basenames.  Linked
     DLLs are loaded using just the basename and the default DLL search path.
     The Windows loader picks up the first one it finds.  */
  dll *d = (type == DLL_LINK) ? dlls.find_by_modname (modname) : dlls[name];
  if (d)
    {
      /* We only get here in the forkee. */
      if (d->handle != h)
	fabort ("%W: Loaded to different address: parent(%p) != child(%p)",
		name, d->handle, h);
      /* If this DLL has been linked against, and the full path differs, try
	 to sanity check if this is the same DLL, just in another path. */
      else if (type == DLL_LINK && wcscasecmp (name, d->name)
	       && (d->p.data_start != p->data_start
		   || d->p.data_start != p->data_start
		   || d->p.bss_start != p->bss_start
		   || d->p.bss_end != p->bss_end
		   || d->p.ctors != p->ctors
		   || d->p.dtors != p->dtors))
      	fabort ("\nLoaded different DLL with same basename in forked child,\n"
		"parent loaded: %W\n"
		" child loaded: %W\n"
		"The DLLs differ, so it's not safe to run the forked child.\n"
		"Make sure to remove the offending DLL before trying again.",
		d->name, name);
      d->p = p;
    }
  else
    {
      d = (dll *) cmalloc (HEAP_2_DLL,
			   sizeof (*d) + (namelen * sizeof (*name)));
      /* Now we've allocated a block of information.  Fill it in with the
	 supplied info about this DLL. */
      wcscpy (d->name, name);
      d->modname = d->name + (modname - name);
      d->handle = h;
      d->count = 0;	/* Reference counting performed in dlopen/dlclose. */
      d->has_dtors = true;
      d->p = p;
      d->ndeps = 0;
      d->deps = NULL;
      d->image_size = ((pefile*)h)->optional_hdr ()->SizeOfImage;
      d->preferred_base = (void*) ((pefile*)h)->optional_hdr()->ImageBase;
      d->type = type;
      append (d);
      if (type == DLL_LOAD)
	loaded_dlls++;
    }
  guard (false);
#ifdef __i386__
  assert (p->envptr != NULL);
#endif
  return d;
}

void
dll_list::append (dll* d)
{
  if (end == NULL)
    end = &start;	/* Point to "end" of dll chain. */
  end->next = d;	/* Standard linked list stuff. */
  d->next = NULL;
  d->prev = end;
  end = d;
}

void dll_list::populate_deps (dll* d)
{
  tmp_pathbuf tp;

  PWCHAR wmodname = tp.w_get ();
  pefile* pef = (pefile*) d->handle;
  PIMAGE_DATA_DIRECTORY dd = pef->idata_dir (IMAGE_DIRECTORY_ENTRY_IMPORT);
  /* Annoyance: calling crealloc with a NULL pointer will use the
     wrong heap and crash, so we have to replicate some code */
  long maxdeps;
  if (!d->ndeps)
    {
      maxdeps = 4;
      d->deps = (dll**) cmalloc (HEAP_2_DLL, maxdeps*sizeof (dll*));
    }
  else
    {
      maxdeps = d->ndeps;
    }
  for (PIMAGE_IMPORT_DESCRIPTOR id=
	(PIMAGE_IMPORT_DESCRIPTOR) pef->rva (dd->VirtualAddress);
      dd->Size && id->Name;
      id++)
    {
      char* modname = pef->rva (id->Name);
      sys_mbstowcs (wmodname, NT_MAX_PATH, modname);
      if (dll* dep = find_by_modname (wmodname))
	{
	  if (d->ndeps >= maxdeps)
	    {
	      maxdeps = 2*(1+maxdeps);
	      d->deps = (dll**) crealloc (d->deps, maxdeps*sizeof (dll*));
	    }
	  d->deps[d->ndeps++] = dep;
	}
    }

  /* add one to differentiate no deps from unknown */
  d->ndeps++;
}


void
dll_list::topsort ()
{
  /* Anything to do? */
  if (!end || end == &start)
    return;

  /* make sure we have all the deps available */
  dll* d = &start;
  dll** dlopen_deps = NULL;
  long maxdeps = 4;
  long dlopen_ndeps = 0;

  if (loaded_dlls > 0)
    dlopen_deps = (dll**) cmalloc (HEAP_2_DLL, maxdeps*sizeof (dll*));

  while ((d = d->next))
    {
      if (!d->ndeps)
        {
          /* Ensure that all dlopen'd DLLs depend on previously dlopen'd DLLs.
             This prevents topsort from reversing the order of dlopen'd DLLs on
             calls to fork. */
          if (d->type == DLL_LOAD)
            {
              /* Initialise d->deps with all previously dlopen'd DLLs. */
              if (dlopen_ndeps)
                {
                  d->ndeps = dlopen_ndeps;
                  d->deps = (dll**) cmalloc (HEAP_2_DLL,
                                             dlopen_ndeps*sizeof (dll*));
                  memcpy (d->deps, dlopen_deps, dlopen_ndeps*sizeof (dll*));
                }
              /* Add this DLL to the list of previously dlopen'd DLLs. */
              if (dlopen_ndeps >= maxdeps)
                {
                  maxdeps = 2*(1+maxdeps);
                  dlopen_deps = (dll**) crealloc (dlopen_deps,
						  maxdeps*sizeof (dll*));
                }
              dlopen_deps[dlopen_ndeps++] = d;
            }
          populate_deps (d);
        }
    }

  if (loaded_dlls > 0)
    cfree (dlopen_deps);

  /* unlink head and tail pointers so the sort can rebuild the list */
  d = start.next;
  start.next = end = NULL;
  topsort_visit (d, true);

  /* clear node markings made by the sort */
  d = &start;
  while ((d = d->next))
    {
#ifdef DEBUGGING
      paranoid_printf ("%W", d->modname);
      for (int i = 1; i < -d->ndeps; i++)
	paranoid_printf ("-> %W", d->deps[i - 1]->modname);
#endif

      /* It would be really nice to be able to keep this information
	 around for next time, but we don't have an easy way to
	 invalidate cached dependencies when a module unloads. */
      d->ndeps = 0;
      cfree (d->deps);
      d->deps = NULL;
    }
}

/* A recursive in-place topological sort. The result is ordered so that
   dependencies of a dll appear before it in the list.

   NOTE: this algorithm is guaranteed to terminate with a "partial
   order" of dlls but does not do anything smart about cycles: an
   arbitrary dependent dll will necessarily appear first. Perhaps not
   surprisingly, Windows ships several dlls containing dependency
   cycles, including SspiCli/RPCRT4.dll and a lovely tangle involving
   USP10/LPK/GDI32/USER32.dll). Fortunately, we don't care about
   Windows DLLs here, and cygwin dlls should behave better */
void
dll_list::topsort_visit (dll* d, bool seek_tail)
{
  /* Recurse to the end of the dll chain, then visit nodes as we
     unwind. We do this because once we start visiting nodes we can no
     longer trust any _next_ pointers.

     We "mark" visited nodes (to avoid revisiting them) by negating
     ndeps (undone once the sort completes). */
  if (seek_tail && d->next)
    topsort_visit (d->next, true);

  if (d->ndeps > 0)
    {
      d->ndeps = -d->ndeps;
      for (long i = 1; i < -d->ndeps; i++)
	topsort_visit (d->deps[i - 1], false);

      append (d);
    }
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
  /* Don't attempt to call destructors if we're still in fork processing
     since that likely means fork is failing and everything will not have been
     set up.  */
  if (!myself || in_forkee)
    return;
  guard (true);
  if ((d = find (retaddr)))
    {
      /* Ensure our exception handler is enabled for destructors */
      exception protect;
      /* Call finalize function if we are not already exiting */
      if (!exit_state)
	__cxa_finalize (d->handle);
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
  guard (false);
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


/* Reserve the chunk of free address space starting _here_ and (usually)
   covering at least _dll_size_ bytes. However, we must take care not
   to clobber the dll's target address range because it often overlaps.
 */
static PVOID
reserve_at (const PWCHAR name, PVOID here, PVOID dll_base, DWORD dll_size)
{
  DWORD size;
  MEMORY_BASIC_INFORMATION mb;

  if (!VirtualQuery (here, &mb, sizeof (mb)))
    fabort ("couldn't examine memory at %p while mapping %W, %E", here, name);
  if (mb.State != MEM_FREE)
    return 0;

  size = mb.RegionSize;

  // don't clobber the space where we want the dll to land
  caddr_t end = (caddr_t) here + size;
  caddr_t dll_end = (caddr_t) dll_base + dll_size;
  if (dll_base < here && dll_end > (caddr_t) here)
      here = (PVOID) dll_end; // the dll straddles our left edge
  else if (dll_base >= here && (caddr_t) dll_base < end)
      end = (caddr_t) dll_base; // the dll overlaps partly or fully to our right

  size = end - (caddr_t) here;
  if (!VirtualAlloc (here, size, MEM_RESERVE, PAGE_NOACCESS))
    fabort ("couldn't allocate memory %p(%d) for '%W' alignment, %E\n",
	    here, size, name);
  return here;
}

/* Release the memory previously allocated by "reserve_at" above. */
static void
release_at (const PWCHAR name, PVOID here)
{
  if (!VirtualFree (here, 0, MEM_RELEASE))
    fabort ("couldn't release memory %p for '%W' alignment, %E\n",
	    here, name);
}

/* Step 1: Reserve memory for all DLL_LOAD dlls. This is to prevent
   anything else from taking their spot as we compensate for Windows
   randomly relocating things.

   NOTE: because we can't depend on LoadLibraryExW to do the right
   thing, we have to do a vanilla VirtualAlloc instead. One possible
   optimization might attempt a LoadLibraryExW first, in case it lands
   in the right place, but then we have to find a way of tracking
   which dlls ended up needing VirtualAlloc after all.  */
void
dll_list::reserve_space ()
{
  for (dll* d = dlls.istart (DLL_LOAD); d; d = dlls.inext ())
    if (!VirtualAlloc (d->handle, d->image_size, MEM_RESERVE, PAGE_NOACCESS))
      fabort ("address space needed by '%W' (%p) is already occupied",
	      d->modname, d->handle);
}

/* We need the in_load_after_fork flag so dll_dllcrt0_1 can decide at fork
   time if this is a linked DLL or a dynamically loaded DLL.  In either case,
   both, cygwin_finished_initializing and in_forkee are true, so they are not
   sufficient to discern the situation. */
static bool NO_COPY in_load_after_fork;

/* Reload DLLs after a fork.  Iterates over the list of dynamically loaded
   DLLs and attempts to load them in the same place as they were loaded in the
   parent. */
void
dll_list::load_after_fork (HANDLE parent)
{
  // moved to frok::child for performance reasons:
  // dll_list::reserve_space();

  in_load_after_fork = true;
  load_after_fork_impl (parent, dlls.istart (DLL_LOAD), 0);
  in_load_after_fork = false;
}

static int const DLL_RETRY_MAX = 6;
void dll_list::load_after_fork_impl (HANDLE parent, dll* d, int retries)
{
  /* Step 2: For each dll which did not map at its preferred base
     address in the parent, try to coerce it to land at the same spot
     as before. If not, unload it, reserve the memory around it, and
     try again. Use recursion to remember blocked regions address
     space so we can release them later.

     We DONT_RESOLVE_DLL_REFERENCES at first in case the DLL lands in
     the wrong spot;

     NOTE: This step skips DLLs which loaded at their preferred address in
     the parent because they should behave (we already verified that their
     preferred address in the child is available). However, this may fail
     with ASLR active, because the ASLR base address will usually not equal
     the preferred base recorded in the dll. In this case, we should make
     the LoadLibraryExW call unconditional.
   */
  for ( ; d; d = dlls.inext ())
    if (d->handle != d->preferred_base)
      {
	/* See if the DLL will load in proper place. If not, unload it,
	   reserve the memory around it, and try again.

	   If this is the first attempt, we need to release the
	   dll's protective reservation from step 1
	 */
	if (!retries && !VirtualFree (d->handle, 0, MEM_RELEASE))
	  fabort ("unable to release protective reservation for %W (%p), %E",
		  d->modname, d->handle);

	HMODULE h = LoadLibraryExW (d->name, NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (!h)
	  fabort ("unable to create interim mapping for %W, %E", d->name);
	if (h != d->handle)
	  {
	    sigproc_printf ("%W loaded in wrong place: %p != %p",
			    d->modname, h, d->handle);
	    FreeLibrary (h);
	    PVOID reservation = reserve_at (d->modname, h,
					    d->handle, d->image_size);
	    if (!reservation)
	      fabort ("unable to block off %p to prevent %W from loading there",
		      h, d->modname);

	    if (retries < DLL_RETRY_MAX)
	      load_after_fork_impl (parent, d, retries+1);
	    else
	       fabort ("unable to remap %W to same address as parent (%p) - try running rebaseall",
		       d->modname, d->handle);

	    /* once the above returns all the dlls are mapped; release
	       the reservation and continue unwinding */
	    sigproc_printf ("releasing blocked space at %p", reservation);
	    release_at (d->modname, reservation);
	    return;
	  }
      }

  /* Step 3: try to load each dll for real after either releasing the
     protective reservation (for well-behaved dlls) or unloading the
     interim mapping (for rebased dlls) . The dll list is sorted in
     dependency order, so we shouldn't pull in any additional dlls
     outside our control.  */
  for (dll *d = dlls.istart (DLL_LOAD); d; d = dlls.inext ())
    {
      if (d->handle == d->preferred_base)
	{
	  if (!VirtualFree (d->handle, 0, MEM_RELEASE))
	    fabort ("unable to release protective reservation for %W (%p), %E",
		    d->modname, d->handle);
	}
      else
	{
	  /* Free the library using our parent's handle: it's identical
	     to ours or we wouldn't have gotten this far */
	  if (!FreeLibrary (d->handle))
	    fabort ("unable to unload interim mapping of %W, %E",
		    d->modname);
	}
      HMODULE h = LoadLibraryW (d->name);
      if (!h)
	fabort ("unable to map %W, %E", d->name);
      if (h != d->handle)
	fabort ("unable to map %W to same address as parent: %p != %p",
		d->modname, d->handle, h);
      /* Fix OS reference count. */
      for (int cnt = 1; cnt < d->count; ++cnt)
	LoadLibraryW (d->name);
    }
}

struct dllcrt0_info
{
  HMODULE h;
  per_process *p;
  PVOID res;
  dllcrt0_info (HMODULE h0, per_process *p0): h (h0), p (p0) {}
};

extern "C" PVOID
dll_dllcrt0 (HMODULE h, per_process *p)
{
  if (dynamically_loaded)
    return (PVOID) 1;
  dllcrt0_info x (h, p);
  dll_dllcrt0_1 (&x);
  return x.res;
}

void
dll_dllcrt0_1 (VOID *x)
{
  HMODULE& h = ((dllcrt0_info *) x)->h;
  per_process*& p = ((dllcrt0_info *) x)->p;
  PVOID& res = ((dllcrt0_info *) x)->res;

  if (p == NULL)
    p = &__cygwin_user_data;
  else
    {
      *(p->impure_ptr_ptr) = __cygwin_user_data.impure_ptr;
      _pei386_runtime_relocator (p);
    }

  bool linked = !cygwin_finished_initializing && !in_load_after_fork;

  /* Broken DLLs built against Cygwin versions 1.7.0-49 up to 1.7.0-57
     override the cxx_malloc pointer in their DLL initialization code,
     when loaded either statically or dynamically.  Because this leaves
     a stale pointer into demapped memory space if the DLL is unloaded
     by a call to dlclose, we prevent this happening for dynamically
     loaded DLLs in dlopen by saving and restoring cxx_malloc around
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
    res = (PVOID) -1;
  else
    res = (PVOID) d;
}

#ifdef __i386__
/* OBSOLETE: This function is obsolete and will go away in the
   future.  Cygwin can now handle being loaded from a noncygwin app
   using the same entry point. */
extern "C" int
dll_noncygwin_dllcrt0 (HMODULE h, per_process *p)
{
  return (int) dll_dllcrt0 (h, p);
}
#endif /* __i386__ */

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

#ifdef __i386__
/* Called from various places to update all of the individual
   ideas of the environ block.  Explain to me again why we didn't
   just import __cygwin_environ? */
void __stdcall
update_envptrs ()
{
  for (dll *d = dlls.istart (DLL_ANY); d; d = dlls.inext ())
    if (*(d->p.envptr) != __cygwin_environ)
      *(d->p.envptr) = __cygwin_environ;
  *main_environ = __cygwin_environ;
}
#endif
