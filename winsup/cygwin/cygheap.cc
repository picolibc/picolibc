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

void *cygheap = NULL;
void *cygheap_max = NULL;

static NO_COPY muto *cygheap_protect = NULL;

extern "C" void __stdcall
cygheap_init ()
{
  cygheap_protect = new_muto (FALSE, "cygheap_protect");
}

inline static void
init_cheap ()
{
  cygheap = VirtualAlloc (NULL, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS);
  if (!cygheap)
    api_fatal ("Couldn't reserve space for cygwin's heap, %E");
  cygheap_max = (((char **) cygheap) + 1);
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
  if (needalloc && !VirtualAlloc (lastheap, (DWORD) sbs, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("couldn't commit memory for cygwin heap, %E");

  return lastheap;
}

/* Copyright (C) 1997, 2000 DJ Delorie */

#define NBUCKETS 32
char *buckets[NBUCKETS] = {0};
int bucket2size[NBUCKETS] = {0};

static inline int
size2bucket (int size)
{
  int rv = 0x1f;
  int bit = ~0x10;
  int i;

  if (size < 4)
    size = 4;
  size = (size + 3) & ~3;

  for (i = 0; i < 5; i++)
    {
      if (bucket2size[rv & bit] >= size)
	rv &= bit;
      bit >>= 1;
    }
  return rv;
}

static inline void
init_buckets ()
{
  unsigned b;
  for (b = 0; b < NBUCKETS; b++)
    bucket2size[b] = (1 << b);
}

struct _cmalloc_entry
{
  union
  {
    DWORD b;
    char *ptr;
  };
  struct _cmalloc_entry *prev;
  char data[0];
};


#define N0 ((_cmalloc_entry *) NULL)
#define to_cmalloc(s) ((_cmalloc_entry *) (((char *) (s)) - (int) (N0->data)))
#define cygheap_chain ((_cmalloc_entry **)cygheap)

static void *__stdcall
_cmalloc (int size)
{
  _cmalloc_entry *rvc;
  int b;

  if (bucket2size[0] == 0)
    init_buckets ();

  b = size2bucket (size);
  cygheap_protect->acquire ();
  if (buckets[b])
    {
      rvc = (_cmalloc_entry *) buckets[b];
      buckets[b] = rvc->ptr;
      rvc->b = b;
    }
  else
    {
      size = bucket2size[b] + sizeof (_cmalloc_entry);
      rvc = (_cmalloc_entry *) _csbrk (size);

      rvc->b = b;
      rvc->prev = *cygheap_chain;
      *cygheap_chain = rvc;
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
      int oldsize = bucket2size[to_cmalloc (ptr)->b];
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
    api_fatal ("Couldn't reserve space for cygwin's heap in child, %E");

  /* Allocate same amount of memory as parent */
  if (!VirtualAlloc (cygheap, n, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("Couldn't allocate space for child's heap %p, size %d, %E",
	       cygheap, n);

  /* Copy memory from the parent */
  m = 0;
  n = (DWORD) pagetrunc (n + 4095);
  if (!ReadProcessMemory (parent, cygheap, cygheap, n, &m) ||
      m != n)
    api_fatal ("Couldn't read parent's cygwin heap %d bytes != %d, %E",
	       n, m);

  if (!execed)
    return;		/* Forked.  Nothing extra to do. */

  /* Walk the allocated memory chain looking for orphaned memory from
     previous execs */
  for (_cmalloc_entry *rvc = *cygheap_chain; rvc; rvc = rvc->prev)
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

static void *__stdcall
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
    memset (c->data, 0, size);
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
