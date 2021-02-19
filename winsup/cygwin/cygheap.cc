/* cygheap.cc: Cygwin heap manager.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <assert.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "tty.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include "heap.h"
#include "sigproc.h"
#include "pinfo.h"
#include "registry.h"
#include "ntdll.h"
#include <unistd.h>
#include <wchar.h>
#include <sys/param.h>

static mini_cygheap NO_COPY cygheap_dummy =
{
  {__utf8_mbtowc}
};

init_cygheap NO_COPY *cygheap = (init_cygheap *) &cygheap_dummy;
void NO_COPY *cygheap_max;

extern "C" char  _cygheap_end[];

static NO_COPY muto cygheap_protect;

struct cygheap_entry
{
  int type;
  struct cygheap_entry *next;
  char data[0];
};

class tls_sentry
{
public:
  static muto lock;
  int destroy;
  void init ();
  bool acquired () {return lock.acquired ();}
  tls_sentry () {destroy = 0;}
  tls_sentry (DWORD wait) {destroy = lock.acquire (wait);}
  ~tls_sentry () {if (destroy) lock.release ();}
};

muto NO_COPY tls_sentry::lock;
static NO_COPY uint32_t nthreads;

#define THREADLIST_CHUNK 256

#define to_cmalloc(s) ((_cmalloc_entry *) (((char *) (s)) - offsetof (_cmalloc_entry, data)))

#define CFMAP_OPTIONS (SEC_RESERVE | PAGE_READWRITE)
#define MVMAP_OPTIONS (FILE_MAP_WRITE)

extern "C" {
static void __reg1 _cfree (void *);
static void *__stdcall _csbrk (int);
}

/* Called by fork or spawn to reallocate cygwin heap */
void __stdcall
cygheap_fixup_in_child (bool execed)
{
  cygheap_max = cygheap = (init_cygheap *) _cygheap_start;
  _csbrk ((char *) child_proc_info->cygheap_max - (char *) cygheap);
  child_copy (child_proc_info->parent, false, child_proc_info->silentfail (),
	      "cygheap", cygheap, cygheap_max, NULL);
  cygheap_init ();
  debug_fixup_after_fork_exec ();
  if (execed)
    {
      cygheap->hooks.next = NULL;
      cygheap->user_heap.base = NULL;		/* We can allocate the heap anywhere */
    }
  /* Walk the allocated memory chain looking for orphaned memory from
     previous execs or forks */
  for (_cmalloc_entry *rvc = cygheap->chain; rvc; rvc = rvc->prev)
    {
      cygheap_entry *ce = (cygheap_entry *) rvc->data;
      if (!rvc->ptr || rvc->b >= NBUCKETS || ce->type <= HEAP_1_START)
	continue;
      else if (ce->type > HEAP_2_MAX)
	_cfree (ce);		/* Marked for freeing in any child */
      else if (!execed)
	continue;
      else if (ce->type > HEAP_1_MAX)
	_cfree (ce);		/* Marked for freeing in execed child */
      else
	ce->type += HEAP_1_MAX;	/* Mark for freeing after next exec */
    }
}

void
init_cygheap::close_ctty ()
{
  debug_printf ("closing cygheap->ctty %p", cygheap->ctty);
  cygheap->ctty->close_with_arch ();
  cygheap->ctty = NULL;
}

/* Use absolute path of cygwin1.dll to derive the Win32 dir which
   is our installation_root.  Note that we can't handle Cygwin installation
   root dirs of more than 4K path length.  I assume that's ok...

   This function also generates the installation_key value.  It's a 64 bit
   hash value based on the path of the Cygwin DLL itself.  It's subsequently
   used when generating shared object names.  Thus, different Cygwin
   installations generate different object names and so are isolated from
   each other.

   Having this information, the installation key together with the
   installation root path is written to the registry.  The idea is that
   cygcheck can print the paths into which the Cygwin DLL has been
   installed for debugging purposes.

   Last but not least, the new cygwin properties datastructure is checked
   for the "disabled_key" value, which is used to determine whether the
   installation key is actually added to all object names or not.  This is
   used as a last resort for debugging purposes, usually.  However, there
   could be another good reason to re-enable object name collisions between
   multiple Cygwin DLLs, which we're just not aware of right now.  Cygcheck
   can be used to change the value in an existing Cygwin DLL binary. */
void
init_cygheap::init_installation_root ()
{
  ptrdiff_t len = 0;

  if (!GetModuleFileNameW (cygwin_hmodule, installation_root_buf, PATH_MAX))
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "GetModuleFileNameW(%p, %p, %u), %E",
	       cygwin_hmodule, installation_root_buf, PATH_MAX);

  /* We don't care if fetching the final pathname fails, it's non-fatal and
     the path returned by GetModuleFileNameW is still valid. */
  HANDLE h;
  h = CreateFileW (installation_root_buf, GENERIC_READ, FILE_SHARE_VALID_FLAGS,
		   &sec_none, OPEN_EXISTING, 0, 0);
  if (h != INVALID_HANDLE_VALUE)
    {
      GetFinalPathNameByHandleW (h, installation_root_buf, PATH_MAX,
				 FILE_NAME_NORMALIZED);
      CloseHandle (h);
    }

  PWCHAR p = installation_root_buf;
  if (wcsncasecmp (p, L"\\\\", 2))	/* Normal drive letter path */
    {
      len = 4;
      memmove (p + 4, p, PATH_MAX - 4);
      p = wcpncpy (p, L"\\\\?\\", 4);
    }
  else
    {
      bool unc = false;
      if (wcsncmp (p + 2, L"?\\", 2))	/* No long path prefix, so UNC path. */
	{
	  len = 6;
	  memmove (p + 6, p, PATH_MAX - 6);
	  p = wcpncpy (p, L"\\??\\UN", 6);
	  *p = L'C';
	  unc = true;
	}
      else if (!wcsncmp (p + 4, L"UNC\\", 4)) /* Native NT UNC path. */
	unc = true;
      if (unc)
	{
	  p = wcschr (p + 2, L'\\');    /* Skip server name */
	  if (p)
	    p = wcschr (p + 1, L'\\');  /* Skip share name */
	}
    }
  installation_root_buf[1] = L'?';
  RtlInitEmptyUnicodeString (&installation_key, installation_key_buf,
			     sizeof installation_key_buf);
  RtlInt64ToHexUnicodeString (hash_path_name (0, installation_root_buf),
			      &installation_key, FALSE);

  /* Strip off last path component ("\\cygwin1.dll") */
  PWCHAR w = wcsrchr (installation_root_buf, L'\\');
  if (w)
    {
      *w = L'\0';
      w = wcsrchr (installation_root_buf, L'\\');
    }
  if (!w)
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "Invalid DLL path");

  /* Copy result into installation_dir before stripping off "bin" dir and
     revert to Win32 path.  This path is added to the Windows environment
     in build_env.  See there for a description. */
  wcpncpy (installation_dir_buf, installation_root_buf + len, PATH_MAX);

  if (len == 4)		/* Local path */
    ;
  else if (len == 6)	/* UNC path */
    installation_dir_buf[0] = L'\\';
  else			/* Long, prefixed path */
    installation_dir_buf[1] = L'\\';

  /* If w < p, the Cygwin DLL resides in the root dir of a drive or network
     path.  In that case, if we strip off yet another backslash, the path
     becomes invalid.  We avoid that here so that the DLL also works in this
     scenario.  The /usr/bin and /usr/lib default mounts will probably point
     to something non-existing, but that's life. */
  if (w > p)
    *w = L'\0';

  RtlInitUnicodeString (&installation_root, installation_root_buf);
  RtlInitUnicodeString (&installation_dir, installation_dir_buf);

  for (int i = 1; i >= 0; --i)
    {
      reg_key r (i, KEY_WRITE, _WIDE (CYGWIN_INFO_INSTALLATIONS_NAME),
		 NULL);
      if (NT_SUCCESS (r.set_string (installation_key_buf,
				    installation_root_buf)))
	break;
    }
}

void __stdcall
cygheap_init ()
{
  cygheap_protect.init ("cygheap_protect");
  if (cygheap == &cygheap_dummy)
    {
      cygheap = (init_cygheap *) memset (_cygheap_start, 0,
					 sizeof (*cygheap));
      cygheap_max = cygheap;
      _csbrk (sizeof (*cygheap));
      /* Initialize bucket_val.  The value is the max size of a block
         fitting into the bucket.  The values are powers of two and their
	 medians: 12, 16, 24, 32, 48, 64, ...  On 64 bit, start with 24 to
	 accommodate bigger size of struct cygheap_entry.
	 With NBUCKETS == 40, the maximum block size is 6291456/12582912.
	 The idea is to have better matching bucket sizes (not wasting
	 space) without trading in performance compared to the old powers
	 of 2 method. */
#ifdef __x86_64__
      unsigned sz[2] = { 16, 24 };	/* sizeof cygheap_entry == 16 */
#else
      unsigned sz[2] = { 8, 12 };	/* sizeof cygheap_entry == 8 */
#endif
      for (unsigned b = 1; b < NBUCKETS; b++, sz[b & 1] <<= 1)
	cygheap->bucket_val[b] = sz[b & 1];
      /* Default locale settings. */
      cygheap->locale.mbtowc = __utf8_mbtowc;
      /* Set umask to a sane default. */
      cygheap->umask = 022;
      cygheap->rlim_core = RLIM_INFINITY;
    }
  if (!cygheap->fdtab)
    cygheap->fdtab.init ();
  if (!cygheap->sigs)
    sigalloc ();
  cygheap->init_tls_list ();
}

/* Initial Cygwin heap setup.
   Called by root process of a Cygwin process tree. */
void
setup_cygheap ()
{
  cygheap_init ();
  cygheap->user.init ();
  cygheap->init_installation_root (); /* Requires user.init! */
  cygheap->pg.init ();
}

#define nextpage(x) ((char *) roundup2 ((uintptr_t) (x), \
					wincap.allocation_granularity ()))
#define allocsize(x) ((SIZE_T) nextpage (x))
#ifdef DEBUGGING
#define somekinda_printf debug_printf
#else
#define somekinda_printf malloc_printf
#endif

static void *__stdcall
_csbrk (int sbs)
{
  void *prebrk = cygheap_max;
  char *newbase = nextpage (prebrk);
  cygheap_max = (char *) cygheap_max + sbs;
  if (!sbs || (newbase >= cygheap_max) || (cygheap_max <= _cygheap_end))
    /* nothing to do */;
  else
    {
      if (prebrk <= _cygheap_end)
	newbase = _cygheap_end;

      SIZE_T adjsbs = allocsize ((char *) cygheap_max - newbase);
      if (adjsbs && !VirtualAlloc (newbase, adjsbs, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))
	{
	  MEMORY_BASIC_INFORMATION m;
	  if (!VirtualQuery (newbase, &m, sizeof m))
	    system_printf ("couldn't get memory info, %E");
	  somekinda_printf ("Couldn't reserve/commit %ld bytes of space for cygwin's heap, %E",
			    adjsbs);
	  somekinda_printf ("AllocationBase %p, BaseAddress %p, RegionSize %lx, State %x\n",
			    m.AllocationBase, m.BaseAddress, m.RegionSize, m.State);
	  __seterrno ();
	  cygheap_max = (char *) cygheap_max - sbs;
	  return NULL;
	}
    }

  return prebrk;
}

/* Copyright (C) 1997, 2000 DJ Delorie */

static void *__reg1 _cmalloc (unsigned size);
static void *__reg2 _crealloc (void *ptr, unsigned size);

static void *__reg1
_cmalloc (unsigned size)
{
  _cmalloc_entry *rvc;
  unsigned b;

  /* Calculate "bit bucket". */
  for (b = 1; b < NBUCKETS && cygheap->bucket_val[b] < size; b++)
    continue;
  if (b >= NBUCKETS)
    return NULL;

  cygheap_protect.acquire ();
  if (cygheap->buckets[b])
    {
      rvc = (_cmalloc_entry *) cygheap->buckets[b];
      cygheap->buckets[b] = rvc->ptr;
      rvc->b = b;
    }
  else
    {
      rvc = (_cmalloc_entry *) _csbrk (cygheap->bucket_val[b]
				       + sizeof (_cmalloc_entry));
      if (!rvc)
	{
	  cygheap_protect.release ();
	  return NULL;
	}

      rvc->b = b;
      rvc->prev = cygheap->chain;
      cygheap->chain = rvc;
    }
  cygheap_protect.release ();
  return rvc->data;
}

static void __reg1
_cfree (void *ptr)
{
  cygheap_protect.acquire ();
  _cmalloc_entry *rvc = to_cmalloc (ptr);
  unsigned b = rvc->b;
  rvc->ptr = cygheap->buckets[b];
  cygheap->buckets[b] = (char *) rvc;
  cygheap_protect.release ();
}

static void *__reg2
_crealloc (void *ptr, unsigned size)
{
  void *newptr;
  if (ptr == NULL)
    newptr = _cmalloc (size);
  else
    {
      unsigned oldsize = cygheap->bucket_val[to_cmalloc (ptr)->b];
      if (size <= oldsize)
	return ptr;
      newptr = _cmalloc (size);
      if (newptr)
	{
	  memcpy (newptr, ptr, oldsize);
	  _cfree (ptr);
	}
    }
  return newptr;
}

/* End Copyright (C) 1997 DJ Delorie */

#define sizeof_cygheap(n) ((n) + sizeof (cygheap_entry))

#define tocygheap(s) ((cygheap_entry *) (((char *) (s)) - offsetof (cygheap_entry, data)))

inline static void *
creturn (cygheap_types x, cygheap_entry * c, unsigned len, const char *fn = NULL)
{
  if (c)
    /* nothing to do */;
  else if (fn)
    api_fatal ("%s would have returned NULL", fn);
  else
    {
      set_errno (ENOMEM);
      return NULL;
    }
  c->type = x;
  char *cend = ((char *) c + sizeof (*c) + len);
  if (cygheap_max < cend)
    cygheap_max = cend;
  return (void *) c->data;
}

inline static void *
cmalloc (cygheap_types x, size_t n, const char *fn)
{
  cygheap_entry *c;
  c = (cygheap_entry *) _cmalloc (sizeof_cygheap (n));
  return creturn (x, c, n, fn);
}

extern "C" void *
cmalloc (cygheap_types x, size_t n)
{
  return cmalloc (x, n, NULL);
}

extern "C" void *
cmalloc_abort (cygheap_types x, size_t n)
{
  return cmalloc (x, n, "cmalloc");
}

inline static void *
crealloc (void *s, size_t n, const char *fn)
{
  if (s == NULL)
    return cmalloc (HEAP_STR, n);	// kludge

  assert (!inheap (s));
  cygheap_entry *c = tocygheap (s);
  cygheap_types t = (cygheap_types) c->type;
  c = (cygheap_entry *) _crealloc (c, sizeof_cygheap (n));
  return creturn (t, c, n, fn);
}

extern "C" void *__reg2
crealloc (void *s, size_t n)
{
  return crealloc (s, n, NULL);
}

extern "C" void *__reg2
crealloc_abort (void *s, size_t n)
{
  return crealloc (s, n, "crealloc");
}

extern "C" void __reg1
cfree (void *s)
{
  assert (!inheap (s));
  _cfree (tocygheap (s));
}

extern "C" void __reg2
cfree_and_set (char *&s, char *what)
{
  if (s && s != almost_null)
    cfree (s);
  s = what;
}

inline static void *
ccalloc (cygheap_types x, size_t n, size_t size, const char *fn)
{
  cygheap_entry *c;
  n *= size;
  c = (cygheap_entry *) _cmalloc (sizeof_cygheap (n));
  if (c)
    memset (c->data, 0, n);
  return creturn (x, c, n, fn);
}

extern "C" void *__reg3
ccalloc (cygheap_types x, size_t n, size_t size)
{
  return ccalloc (x, n, size, NULL);
}

extern "C" void *__reg3
ccalloc_abort (cygheap_types x, size_t n, size_t size)
{
  return ccalloc (x, n, size, "ccalloc");
}

extern "C" PWCHAR __reg1
cwcsdup (PCWSTR s)
{
  PWCHAR p = (PWCHAR) cmalloc (HEAP_STR, (wcslen (s) + 1) * sizeof (WCHAR));
  if (!p)
    return NULL;
  wcpcpy (p, s);
  return p;
}

extern "C" PWCHAR __reg1
cwcsdup1 (PCWSTR s)
{
  PWCHAR p = (PWCHAR) cmalloc (HEAP_1_STR, (wcslen (s) + 1) * sizeof (WCHAR));
  if (!p)
    return NULL;
  wcpcpy (p, s);
  return p;
}

extern "C" char *__reg1
cstrdup (const char *s)
{
  char *p = (char *) cmalloc (HEAP_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  return p;
}

extern "C" char *__reg1
cstrdup1 (const char *s)
{
  char *p = (char *) cmalloc (HEAP_1_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  return p;
}

void
cygheap_root::set (const char *posix, const char *native, bool caseinsensitive)
{
  if (*posix == '/' && posix[1] == '\0')
    {
      if (m)
	{
	  cfree (m);
	  m = NULL;
	}
      return;
    }
  if (!m)
    m = (struct cygheap_root_mount_info *) ccalloc (HEAP_MOUNT, 1, sizeof (*m));
  strcpy (m->posix_path, posix);
  m->posix_pathlen = strlen (posix);
  if (m->posix_pathlen >= 1 && m->posix_path[m->posix_pathlen - 1] == '/')
    m->posix_path[--m->posix_pathlen] = '\0';

  strcpy (m->native_path, native);
  m->native_pathlen = strlen (native);
  if (m->native_pathlen >= 1 && m->native_path[m->native_pathlen - 1] == '\\')
    m->native_path[--m->native_pathlen] = '\0';
  m->caseinsensitive = caseinsensitive;
}

cygheap_user::~cygheap_user ()
{
}

void
cygheap_user::set_name (const char *new_name)
{
  bool allocated = !!pname;

  if (allocated)
    {
      /* Windows user names are case-insensitive.  Here we want the correct
	 username, though, even if it only differs by case. */
      if (!strcmp (new_name, pname))
	return;
      cfree (pname);
    }

  pname = cstrdup (new_name ? new_name : "");
  if (!allocated)
    return;		/* Initializing.  Don't bother with other stuff. */

  cfree_and_set (homedrive);
  cfree_and_set (homepath);
  cfree_and_set (plogsrv);
  cfree_and_set (pdomain);
  cfree_and_set (pwinname);
}

void
init_cygheap::init_tls_list ()
{
  if (threadlist)
    memset (cygheap->threadlist, 0, cygheap->sthreads * sizeof (cygheap->threadlist[0]));
  else
    {
      sthreads = THREADLIST_CHUNK;
      threadlist = (threadlist_t *)
		   ccalloc_abort (HEAP_TLS, cygheap->sthreads,
				  sizeof (cygheap->threadlist[0]));
    }
  tls_sentry::lock.init ("thread_tls_sentry");
}

void
init_cygheap::add_tls (_cygtls *t)
{
  cygheap->user.reimpersonate ();
  tls_sentry here (INFINITE);
  if (nthreads >= cygheap->sthreads)
    {
      threadlist = (threadlist_t *)
	crealloc_abort (threadlist, (sthreads += THREADLIST_CHUNK)
			* sizeof (threadlist[0]));
#if 0
      memset (threadlist + nthreads, 0,
	      THREADLIST_CHUNK * sizeof (threadlist[0]));
#endif
    }

  /* Create a mutex to lock the thread's _cygtls area.  This is required for
     the following reason:  The thread's _cygtls area is on the thread's
     own stack.  Thus, when the thread exits, its _cygtls area is automatically
     destroyed by the OS.  Thus, when this happens while the signal thread
     still utilizes the thread's _cygtls area, things go awry.

     The following methods take this into account:

     - The thread mutex is generally only locked under tls_sentry locking.
     - remove_tls, called from _cygtls::remove, locks the mutex before
       removing the threadlist entry and _cygtls::remove then unlocks and
       destroyes the mutex.
     - find_tls, called from several places but especially from the signal
       thread, will lock the mutex on exit and the caller can access the
       _cygtls area locked.  Always make sure to unlock the mutex when the
       _cygtls area isn't needed anymore. */
  threadlist[nthreads].thread = t;
  threadlist[nthreads].mutex = CreateMutexW (&sec_none_nih, FALSE, NULL);
  if (!threadlist[nthreads].mutex)
    api_fatal ("Can't create per-thread mutex, %E");
  ++nthreads;
}

HANDLE __reg3
init_cygheap::remove_tls (_cygtls *t)
{
  HANDLE mutex = NULL;

  tls_sentry here (INFINITE);
  if (here.acquired ())
    {
      for (uint32_t i = 0; i < nthreads; i++)
	if (t == threadlist[i].thread)
	  {
	    mutex = threadlist[i].mutex;
	    WaitForSingleObject (mutex, INFINITE);
	    if (i < --nthreads)
	      threadlist[i] = threadlist[nthreads];
	    debug_only_printf ("removed %p element %u", this, i);
	    break;
	  }
    }
  /* Leave with locked mutex.  The calling function is responsible for
     unlocking the mutex. */
  return mutex;
}

threadlist_t __reg2 *
init_cygheap::find_tls (_cygtls *tls)
{
  tls_sentry here (INFINITE);

  threadlist_t *t = NULL;
  int ix = -1;
  while (++ix < (int) nthreads)
    {
      if (!threadlist[ix].thread->tid
	  || !threadlist[ix].thread->initialized)
	;
      if (threadlist[ix].thread == tls)
	{
	  t = &threadlist[ix];
	  break;
	}
    }
  /* Leave with locked mutex.  The calling function is responsible for
     unlocking the mutex. */
  if (t)
    WaitForSingleObject (t->mutex, INFINITE);
  return t;
}

threadlist_t __reg3 *
init_cygheap::find_tls (int sig, bool& issig_wait)
{
  debug_printf ("sig %d\n", sig);
  tls_sentry here (INFINITE);

  threadlist_t *t = NULL;
  issig_wait = false;

  int ix = -1;
  /* Scan thread list looking for valid signal-delivery candidates */
  while (++ix < (int) nthreads)
    {
      /* Only pthreads have tid set to non-0. */
      if (!threadlist[ix].thread->tid
	  || !threadlist[ix].thread->initialized)
	;
      else if (sigismember (&(threadlist[ix].thread->sigwait_mask), sig))
	{
	  t = &cygheap->threadlist[ix];
	  issig_wait = true;
	  break;
	}
      else if (!t && !sigismember (&(threadlist[ix].thread->sigmask), sig))
	  t = &cygheap->threadlist[ix];
    }
  /* Leave with locked mutex.  The calling function is responsible for
     unlocking the mutex. */
  if (t)
    WaitForSingleObject (t->mutex, INFINITE);
  return t;
}

/* Called from profil.c to sample all non-main thread PC values for profiling */
extern "C" void
cygheap_profthr_all (void (*profthr_byhandle) (HANDLE))
{
  for (uint32_t ix = 0; ix < nthreads; ix++)
    {
      _cygtls *tls = cygheap->threadlist[ix].thread;
      if (tls->tid)
	profthr_byhandle (tls->tid->win32_obj_id);
    }
}
