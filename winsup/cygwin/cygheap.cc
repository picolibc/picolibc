/* cygheap.cc: Cygwin heap manager.

   Copyright 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <errno.h>
#include <fhandler.h>
#include <assert.h>
#include <stdlib.h>
#include "cygheap.h"
#include "heap.h"
#include "cygerrno.h"
#include "sync.h"

init_cygheap NO_COPY *cygheap;
void NO_COPY *cygheap_max = NULL;

static NO_COPY muto *cygheap_protect = NULL;

inline static void
init_cheap ()
{
  cygheap = (init_cygheap *) VirtualAlloc (NULL, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS);
  if (!cygheap)
    api_fatal ("Couldn't reserve space for cygwin's heap, %E");
  cygheap_max = cygheap + 1;
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
}

/* Copyright (C) 1997, 2000 DJ Delorie */

#define NBUCKETS 32
char *buckets[NBUCKETS] = {0};

#define N0 ((_cmalloc_entry *) NULL)
#define to_cmalloc(s) ((_cmalloc_entry *) (((char *) (s)) - (int) (N0->data)))

static void *_cmalloc (int size) __attribute ((regparm(1)));
static void *__stdcall _crealloc (void *ptr, int size) __attribute ((regparm(2)));

static void *__stdcall
_cmalloc (int size)
{
  _cmalloc_entry *rvc;
  int b, sz;

  /* Calculate "bit bucket" and size as a power of two. */
  for (b = 3, sz = 8; sz && sz < (size + 4); b++, sz <<= 1)
    continue;

  cygheap_protect->acquire ();
  if (buckets[b])
    {
      rvc = (_cmalloc_entry *) buckets[b];
      buckets[b] = rvc->ptr;
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
  rvc->ptr = buckets[b];
  buckets[b] = (char *) rvc;
  cygheap_protect->release ();
}

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

struct cygheap_entry
  {
    int type;
    struct cygheap_entry *next;
    char data[0];
  };

#define N ((cygheap_entry *) NULL)
#define tocygheap(s) ((cygheap_entry *) (((char *) (s)) - (int) (N->data)))

/* Called by fork or spawn to reallocate cygwin heap */
extern "C" void __stdcall
cygheap_fixup_in_child (HANDLE parent, bool execed)
{
  DWORD m, n;
  n = (DWORD) cygheap_max - (DWORD) cygheap;

  /* Reserve cygwin heap in same spot as parent */
  if (!VirtualAlloc (cygheap, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS))
    api_fatal ("Couldn't reserve space for cygwin's heap (%p) in child, cygheap, %E", cygheap);

  /* Allocate same amount of memory as parent */
  if (!VirtualAlloc (cygheap, n, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("Couldn't allocate space for child's heap %p, size %d, %E",
	       cygheap, n);

  /* Copy memory from the parent */
  m = 0;
  if (!ReadProcessMemory (parent, cygheap, cygheap, n, &m) || m != n)
    api_fatal ("Couldn't read parent's cygwin heap %d bytes != %d, %E",
	       n, m);

  cygheap_init ();

  if (execed)
    {
      /* Walk the allocated memory chain looking for orphaned memory from
	 previous execs */
      for (_cmalloc_entry *rvc = cygheap->chain; rvc; rvc = rvc->prev)
	{
	  cygheap_entry *ce = (cygheap_entry *) rvc->data;
	  if (rvc->b >= NBUCKETS || ce->type <= HEAP_1_START)
	    continue;
	  else if (ce->type < HEAP_1_MAX)
	    ce->type += HEAP_1_MAX;	/* Mark for freeing after next exec */
	  else
	    _cfree (ce);		/* Marked by parent for freeing in child */
	}
    }
}

inline static void *
creturn (cygheap_types x, cygheap_entry * c, int len)
{
  if (!c)
    {
      __seterrno ();
      return NULL;
    }
  c->type = x;
  if (cygheap_max < ((char *) c + len))
    cygheap_max = (char *) c + len;
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
