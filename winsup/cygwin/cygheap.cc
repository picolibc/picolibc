/* cygheap.cc: Cygwin heap manager.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
   2011, 2012, 2013, 2014 Red Hat, Inc.

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
  {__utf8_mbtowc, __utf8_wctomb}
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
  child_copy (child_proc_info->parent, false, "cygheap", cygheap, cygheap_max, NULL);
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

   Last but not least, the new cygwin properties datastrcuture is checked
   for the "disabled_key" value, which is used to determine whether the
   installation key is actually added to all object names or not.  This is
   used as a last resort for debugging purposes, usually.  However, there
   could be another good reason to re-enable object name collisions between
   multiple Cygwin DLLs, which we're just not aware of right now.  Cygcheck
   can be used to change the value in an existing Cygwin DLL binary. */
void
init_cygheap::init_installation_root ()
{
  if (!GetModuleFileNameW (cygwin_hmodule, installation_root, PATH_MAX))
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "GetModuleFileNameW(%p, %p, %u), %E",
	       cygwin_hmodule, installation_root, PATH_MAX);
  PWCHAR p = installation_root;
  if (wcsncasecmp (p, L"\\\\", 2))	/* Normal drive letter path */
    {
      p = wcpcpy (p, L"\\??\\");
      GetModuleFileNameW (cygwin_hmodule, p, PATH_MAX - 4);
    }
  else
    {
      bool unc = false;
      if (wcsncmp (p + 2, L"?\\", 2))	/* No long path prefix, so UNC path. */
	{
	  p = wcpcpy (p, L"\\??\\UN");
	  GetModuleFileNameW (cygwin_hmodule, p, PATH_MAX - 6);
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
  installation_root[1] = L'?';

  RtlInitEmptyUnicodeString (&installation_key, installation_key_buf,
			     sizeof installation_key_buf);
  RtlInt64ToHexUnicodeString (hash_path_name (0, installation_root),
			      &installation_key, FALSE);

  PWCHAR w = wcsrchr (installation_root, L'\\');
  if (w)
    {
      *w = L'\0';
      w = wcsrchr (installation_root, L'\\');
    }
  if (!w)
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "Invalid DLL path");
  /* If w < p, the Cygwin DLL resides in the root dir of a drive or network
     path.  In that case, if we strip off yet another backslash, the path
     becomes invalid.  We avoid that here so that the DLL also works in this
     scenario.  The /usr/bin and /usr/lib default mounts will probably point
     to something non-existing, but that's life. */
  if (w > p)
    *w = L'\0';

  for (int i = 1; i >= 0; --i)
    {
      reg_key r (i, KEY_WRITE, _WIDE (CYGWIN_INFO_INSTALLATIONS_NAME),
		 NULL);
      if (NT_SUCCESS (r.set_string (installation_key_buf,
				    installation_root)))
	break;
    }

  if (cygwin_props.disable_key)
    {
      installation_key.Length = 0;
      installation_key.Buffer[0] = L'\0';
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
      cygheap->locale.wctomb = __utf8_wctomb;
      strcpy (cygheap->locale.charset, "UTF-8");
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
  MALLOC_CHECK;
  return (void *) c->data;
}

inline static void *
cmalloc (cygheap_types x, size_t n, const char *fn)
{
  cygheap_entry *c;
  MALLOC_CHECK;
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
  MALLOC_CHECK;
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
  MALLOC_CHECK;
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
  MALLOC_CHECK;
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
  MALLOC_CHECK;
  PWCHAR p = (PWCHAR) cmalloc (HEAP_STR, (wcslen (s) + 1) * sizeof (WCHAR));
  if (!p)
    return NULL;
  wcpcpy (p, s);
  MALLOC_CHECK;
  return p;
}

extern "C" PWCHAR __reg1
cwcsdup1 (PCWSTR s)
{
  MALLOC_CHECK;
  PWCHAR p = (PWCHAR) cmalloc (HEAP_1_STR, (wcslen (s) + 1) * sizeof (WCHAR));
  if (!p)
    return NULL;
  wcpcpy (p, s);
  MALLOC_CHECK;
  return p;
}

extern "C" char *__reg1
cstrdup (const char *s)
{
  MALLOC_CHECK;
  char *p = (char *) cmalloc (HEAP_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  MALLOC_CHECK;
  return p;
}

extern "C" char *__reg1
cstrdup1 (const char *s)
{
  MALLOC_CHECK;
  char *p = (char *) cmalloc (HEAP_1_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  MALLOC_CHECK;
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
      threadlist = (_cygtls **) ccalloc_abort (HEAP_TLS, cygheap->sthreads,
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
      threadlist = (_cygtls **)
	crealloc_abort (threadlist, (sthreads += THREADLIST_CHUNK)
			* sizeof (threadlist[0]));
      // memset (threadlist + nthreads, 0, THREADLIST_CHUNK * sizeof (threadlist[0]));
    }

  threadlist[nthreads++] = t;
}

void
init_cygheap::remove_tls (_cygtls *t, DWORD wait)
{
  tls_sentry here (wait);
  if (here.acquired ())
    {
      for (uint32_t i = 0; i < nthreads; i++)
	if (t == threadlist[i])
	  {
	    if (i < --nthreads)
	      threadlist[i] = threadlist[nthreads];
	    debug_only_printf ("removed %p element %u", this, i);
	    break;
	  }
    }
}

_cygtls __reg3 *
init_cygheap::find_tls (int sig, bool& issig_wait)
{
  debug_printf ("sig %d\n", sig);
  tls_sentry here (INFINITE);

  static int NO_COPY ix;

  _cygtls *t = NULL;
  issig_wait = false;

  myfault efault;
  if (efault.faulted ())
    threadlist[ix]->remove (INFINITE);
  else
    {
      ix = -1;
      /* Scan thread list looking for valid signal-delivery candidates */
      while (++ix < (int) nthreads)
	if (!threadlist[ix]->tid)
	  continue;
	else if (sigismember (&(threadlist[ix]->sigwait_mask), sig))
	  {
	    t = cygheap->threadlist[ix];
	    issig_wait = true;
	    goto out;
	  }
	else if (!t && !sigismember (&(threadlist[ix]->sigmask), sig))
	  t = cygheap->threadlist[ix];
    }
out:
  return t;
}
