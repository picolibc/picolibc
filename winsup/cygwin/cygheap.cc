/* cygheap.cc: Cygwin heap manager.

   Copyright 2000, 2001 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "security.h"
#include "fhandler.h"
#include "dtable.h"
#include "path.h"
#include "cygheap.h"
#include "child_info.h"
#include "heap.h"
#include "cygerrno.h"
#include "sync.h"
#include "shared_info.h"

init_cygheap NO_COPY *cygheap;
void NO_COPY *cygheap_max = NULL;

static NO_COPY muto *cygheap_protect = NULL;

struct cygheap_entry
  {
    int type;
    struct cygheap_entry *next;
    char data[0];
  };

#define NBUCKETS (sizeof (cygheap->buckets) / sizeof (cygheap->buckets[0]))
#define N0 ((_cmalloc_entry *) NULL)
#define to_cmalloc(s) ((_cmalloc_entry *) (((char *) (s)) - (int) (N0->data)))

#define CFMAP_OPTIONS (SEC_RESERVE | PAGE_READWRITE)
#define MVMAP_OPTIONS (FILE_MAP_WRITE)

extern "C" {
static void __stdcall _cfree (void *ptr) __attribute__((regparm(1)));
extern void *_cygheap_start;
}

inline static void
init_cheap ()
{
  cygheap = (init_cygheap *) VirtualAlloc ((void *) &_cygheap_start, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS);
  if (!cygheap)
    {
      MEMORY_BASIC_INFORMATION m;
      if (!VirtualQuery ((LPCVOID) &_cygheap_start, &m, sizeof m))
	system_printf ("couldn't get memory info, %E");
      small_printf ("AllocationBase %p, BaseAddress %p, RegionSize %p, State %p\n",
		    m.AllocationBase, m.BaseAddress, m.RegionSize, m.State);
      api_fatal ("Couldn't reserve space for cygwin's heap, %E");
    }
  cygheap_max = cygheap + 1;
}

void __stdcall
cygheap_setup_for_child (child_info *ci)
{
  void *newcygheap;
  cygheap_protect->acquire ();
  unsigned n = (char *) cygheap_max - (char *) cygheap;
  ci->cygheap_h = CreateFileMapping (INVALID_HANDLE_VALUE, &sec_none,
				     CFMAP_OPTIONS, 0, CYGHEAPSIZE, NULL);
  newcygheap = MapViewOfFileEx (ci->cygheap_h, MVMAP_OPTIONS, 0, 0, 0, NULL);
  if (!VirtualAlloc (newcygheap, n, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("couldn't allocate new cygwin heap for child, %E");
  memcpy (newcygheap, cygheap, n);
  UnmapViewOfFile (newcygheap);
  ci->cygheap = cygheap;
  ci->cygheap_max = cygheap_max;
  ProtectHandle1 (ci->cygheap_h, passed_cygheap_h);
  cygheap_protect->release ();
  return;
}

void __stdcall
cygheap_setup_for_child_cleanup (child_info *ci)
{
  ForceCloseHandle1 (ci->cygheap_h, passed_cygheap_h);
}

/* Called by fork or spawn to reallocate cygwin heap */
void __stdcall
cygheap_fixup_in_child (child_info *ci, bool execed)
{
  cygheap = ci->cygheap;
  cygheap_max = ci->cygheap_max;
  void *addr = iswinnt ? cygheap : NULL;
  void *newaddr;

  newaddr = MapViewOfFileEx (ci->cygheap_h, MVMAP_OPTIONS, 0, 0, 0, addr);
  if (newaddr != cygheap)
    {
      if (!newaddr)
	newaddr = MapViewOfFileEx (ci->cygheap_h, MVMAP_OPTIONS, 0, 0, 0, NULL);
      DWORD n = (DWORD) cygheap_max - (DWORD) cygheap;
      /* Reserve cygwin heap in same spot as parent */
      if (!VirtualAlloc (cygheap, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS))
      {
	MEMORY_BASIC_INFORMATION m;
	memset (&m, 0, sizeof m);
	if (!VirtualQuery ((LPCVOID) cygheap, &m, sizeof m))
	  system_printf ("couldn't get memory info, %E");

	small_printf ("m.AllocationBase %p, m.BaseAddress %p, m.RegionSize %p, m.State %p\n",
		      m.AllocationBase, m.BaseAddress, m.RegionSize, m.State);
	api_fatal ("Couldn't reserve space for cygwin's heap (%p <%p>) in child, %E", cygheap, newaddr);
      }

      /* Allocate same amount of memory as parent */
      if (!VirtualAlloc (cygheap, n, MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("Couldn't allocate space for child's heap %p, size %d, %E",
		   cygheap, n);
      memcpy (cygheap, newaddr, n);
      UnmapViewOfFile (newaddr);
    }

  ForceCloseHandle1 (ci->cygheap_h, passed_cygheap_h);

  cygheap_init ();

  if (execed)
    {
      cygheap->heapbase = NULL;		/* We can allocate the heap anywhere */
      /* Walk the allocated memory chain looking for orphaned memory from
	 previous execs */
      for (_cmalloc_entry *rvc = cygheap->chain; rvc; rvc = rvc->prev)
	{
	  cygheap_entry *ce = (cygheap_entry *) rvc->data;
	  if (!rvc->ptr || rvc->b >= NBUCKETS || ce->type <= HEAP_1_START)
	    continue;
	  else if (ce->type < HEAP_1_MAX)
	    ce->type += HEAP_1_MAX;	/* Mark for freeing after next exec */
	  else
	    _cfree (ce);		/* Marked by parent for freeing in child */
	}
    }
}

#define pagetrunc(x) ((void *) (((DWORD) (x)) & ~(4096 - 1)))

static void *__stdcall
_csbrk (int sbs)
{
  void *lastheap;
  bool needalloc;

  if (cygheap)
    needalloc = 0;
  else
    {
      init_cheap ();
      needalloc = 1;
    }

  lastheap = cygheap_max;
  (char *) cygheap_max += sbs;
  void *heapalign = (void *) pagetrunc (lastheap);

  if (!needalloc)
    needalloc = sbs && ((heapalign == lastheap) || heapalign != pagetrunc (cygheap_max));
  if (needalloc && !VirtualAlloc (lastheap, (DWORD) sbs ?: 1, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("couldn't commit memory for cygwin heap, %E");

  return lastheap;
}

extern "C" void __stdcall
cygheap_init ()
{
  cygheap_protect = new_muto (FALSE, "cygheap_protect");
  _csbrk (0);
  if (!cygheap->fdtab)
    cygheap->fdtab.init ();
}

/* Copyright (C) 1997, 2000 DJ Delorie */

static void *_cmalloc (int size) __attribute ((regparm(1)));
static void *__stdcall _crealloc (void *ptr, int size) __attribute ((regparm(2)));

static void *__stdcall
_cmalloc (int size)
{
  _cmalloc_entry *rvc;
  unsigned b, sz;

  /* Calculate "bit bucket" and size as a power of two. */
  for (b = 3, sz = 8; sz && sz < (size + sizeof (_cmalloc_entry));
       b++, sz <<= 1)
    continue;

  cygheap_protect->acquire ();
  if (cygheap->buckets[b])
    {
      rvc = (_cmalloc_entry *) cygheap->buckets[b];
      cygheap->buckets[b] = rvc->ptr;
      rvc->b = b;
    }
  else
    {
      size = sz + sizeof (_cmalloc_entry);
      rvc = (_cmalloc_entry *) _csbrk (size);

      rvc->b = b;
      rvc->prev = cygheap->chain;
      cygheap->chain = rvc;
    }
  cygheap_protect->release ();
  return rvc->data;
}

static void __stdcall
_cfree (void *ptr)
{
  cygheap_protect->acquire ();
  _cmalloc_entry *rvc = to_cmalloc (ptr);
  DWORD b = rvc->b;
  rvc->ptr = cygheap->buckets[b];
  cygheap->buckets[b] = (char *) rvc;
  cygheap_protect->release ();
}

static void *__stdcall _crealloc (void *ptr, int size) __attribute__((regparm(2)));
static void *__stdcall
_crealloc (void *ptr, int size)
{
  void *newptr;
  if (ptr == NULL)
    newptr = _cmalloc (size);
  else
    {
      int oldsize = 1 << to_cmalloc (ptr)->b;
      if (size <= oldsize)
	return ptr;
      newptr = _cmalloc (size);
      memcpy (newptr, ptr, oldsize);
      _cfree (ptr);
    }
  return newptr;
}

/* End Copyright (C) 1997 DJ Delorie */

#define sizeof_cygheap(n) ((n) + sizeof(cygheap_entry))

#define N ((cygheap_entry *) NULL)
#define tocygheap(s) ((cygheap_entry *) (((char *) (s)) - (int) (N->data)))

inline static void *
creturn (cygheap_types x, cygheap_entry * c, int len)
{
  if (!c)
    {
      __seterrno ();
      return NULL;
    }
  c->type = x;
  char *cend = ((char *) c + sizeof (*c) + len);
  if (cygheap_max < cend)
    cygheap_max = cend;
  MALLOC_CHECK;
  return (void *) c->data;
}

extern "C" void *__stdcall
cmalloc (cygheap_types x, DWORD n)
{
  cygheap_entry *c;
  MALLOC_CHECK;
  c = (cygheap_entry *) _cmalloc (sizeof_cygheap (n));
  if (!c)
    system_printf ("cmalloc returned NULL");
  return creturn (x, c, n);
}

extern "C" void *__stdcall
crealloc (void *s, DWORD n)
{
  MALLOC_CHECK;
  if (s == NULL)
    return cmalloc (HEAP_STR, n);	// kludge

  assert (!inheap (s));
  cygheap_entry *c = tocygheap (s);
  cygheap_types t = (cygheap_types) c->type;
  c = (cygheap_entry *) _crealloc (c, sizeof_cygheap (n));
  if (!c)
    system_printf ("crealloc returned NULL");
  return creturn (t, c, n);
}

extern "C" void __stdcall
cfree (void *s)
{
  MALLOC_CHECK;
  assert (!inheap (s));
  (void) _cfree (tocygheap (s));
  MALLOC_CHECK;
}

extern "C" void *__stdcall
ccalloc (cygheap_types x, DWORD n, DWORD size)
{
  cygheap_entry *c;
  MALLOC_CHECK;
  c = (cygheap_entry *) _cmalloc (sizeof_cygheap (n * size));
  if (c)
    memset (c->data, 0, n * size);
  if (!c)
    system_printf ("ccalloc returned NULL");
  return creturn (x, c, n);
}

extern "C" char *__stdcall
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

extern "C" char *__stdcall
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

bool
init_cygheap::etc_changed ()
{
  bool res = 0;

  if (!etc_changed_h)
    {
      path_conv pwd ("/etc");
      etc_changed_h = FindFirstChangeNotification (pwd, FALSE,
					      FILE_NOTIFY_CHANGE_LAST_WRITE);
      if (etc_changed_h == INVALID_HANDLE_VALUE)
	system_printf ("Can't open /etc for checking, %E", (char *) pwd,
	    	       etc_changed_h);
      else if (!DuplicateHandle (hMainProc, etc_changed_h, hMainProc,
	    			 &etc_changed_h, 0, TRUE,
				 DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	{
	  system_printf ("Can't inherit /etc handle, %E", (char *) pwd,
	      		 etc_changed_h);
	  etc_changed_h = INVALID_HANDLE_VALUE;
	}
    }

   if (etc_changed_h != INVALID_HANDLE_VALUE
       && WaitForSingleObject (etc_changed_h, 0) == WAIT_OBJECT_0)
     {
       (void) FindNextChangeNotification (etc_changed_h);
       res = 1;
     }

  return res;
}

void
cygheap_root::set (const char *posix, const char *native)
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
}

cygheap_user::~cygheap_user ()
{
#if 0
  if (pname)
    cfree (pname);
  if (plogsrv)
    cfree (plogsrv);
  if (pdomain)
    cfree (pdomain);
  if (psid)
    cfree (psid);
#endif
}

void
cygheap_user::set_name (const char *new_name)
{
  if (pname)
    cfree (pname);
  pname = cstrdup (new_name ? new_name : "");
}

void
cygheap_user::set_logsrv (const char *new_logsrv)
{
  if (plogsrv)
    cfree (plogsrv);
  plogsrv = (new_logsrv && *new_logsrv) ? cstrdup (new_logsrv) : NULL;
}

void
cygheap_user::set_domain (const char *new_domain)
{
  if (pdomain)
    cfree (pdomain);
  pdomain = (new_domain && *new_domain) ? cstrdup (new_domain) : NULL;
}

BOOL
cygheap_user::set_sid (PSID new_sid)
{
  if (!new_sid)
    {
      if (psid)
	cfree (psid);
      psid = NULL;
      return TRUE;
    }
  else
    {
      if (!psid)
	psid = cmalloc (HEAP_STR, MAX_SID_LEN);
      return CopySid (MAX_SID_LEN, psid, new_sid);
    }
}
