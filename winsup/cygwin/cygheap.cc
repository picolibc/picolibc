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
#include "cygheap.h"
#include "heap.h"
#include "cygerrno.h"

inline static void
init_cheap ()
{
  cygheap = VirtualAlloc (NULL, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS);
  if (!cygheap)
    api_fatal ("Couldn't reserve space for cygwin's heap, %E");
  cygheap_max = cygheap;
}

#define pagetrunc(x) ((void *) (((DWORD) (x)) & ~(4096 - 1)))
static void *__stdcall
_csbrk (int sbs)
{
  void *lastheap;
  if (!cygheap)
    init_cheap ();
  lastheap = cygheap_max;
  (char *) cygheap_max += sbs;
  void *heapalign = (void *) pagetrunc (lastheap);
  int needalloc = sbs && ((heapalign == lastheap) || heapalign != pagetrunc (cygheap_max));
  if (needalloc && !VirtualAlloc (lastheap, (DWORD) sbs, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("couldn't commit memory for cygwin heap, %E");

  return lastheap;
}

/* Copyright (C) 1997, 2000 DJ Delorie */

char *buckets[32] = {0};
int bucket2size[32] = {0};

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
  for (b = 0; b < 32; b++)
    bucket2size[b] = (1 << b);
}

static void *__stdcall
_cmalloc (int size)
{
  char *rv;
  int b;

  if (bucket2size[0] == 0)
    init_buckets ();

  b = size2bucket (size);
  if (buckets[b])
    {
      rv = buckets[b];
      buckets[b] = *(char **) rv;
      return rv;
    }

  size = bucket2size[b] + 4;
  rv = (char *) _csbrk (size);

  *(int *) rv = b;
  rv += 4;
  return rv;
}

static void __stdcall
_cfree (void *ptr)
{
  int b = *(int *) ((char *) ptr - 4);
  *(char **) ptr = buckets[b];
  buckets[b] = (char *) ptr;
}

static void *__stdcall
_crealloc (void *ptr, int size)
{
  void *newptr;
  if (ptr == NULL)
    newptr = _cmalloc (size);
  else
    {
      int oldsize = bucket2size[*(int *) ((char *) ptr - 4)];
      if (size <= oldsize)
	return ptr;
      newptr = _cmalloc (size);
      memcpy (newptr, ptr, oldsize);
      _cfree (ptr);
    }
  return newptr;
}

/* End Copyright (C) 1997 DJ Delorie */

void *cygheap = NULL;
void *cygheap_max = NULL;

#define sizeof_cygheap(n) ((n) + sizeof(cygheap_entry))

struct cygheap_entry
  {
    cygheap_types type;
    char data[0];
  };

#define N ((cygheap_entry *) NULL)
#define tocygheap(s) ((cygheap_entry *) (((char *) (s)) - (int) (N->data)))

void
cygheap_init ()
{
  if (!cygheap)
    init_cheap ();
}

extern "C" void __stdcall
cygheap_fixup_in_child (HANDLE parent)
{
  DWORD m, n;
  n = (DWORD) cygheap_max - (DWORD) cygheap;
  if (!VirtualAlloc (cygheap, CYGHEAPSIZE, MEM_RESERVE, PAGE_NOACCESS))
    api_fatal ("Couldn't reserve space for cygwin's heap in child, %E");

  if (!VirtualAlloc (cygheap, n, MEM_COMMIT, PAGE_READWRITE))
    api_fatal ("Couldn't allocate space for child's heap %p, size %d, %E",
	       cygheap, n);
  m = 0;
  n = (DWORD) pagetrunc (n + 4095);
  if (!ReadProcessMemory (parent, cygheap, cygheap, n, &m) ||
      m != n)
    api_fatal ("Couldn't read parent's cygwin heap %d bytes != %d, %E",
	       n, m);
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
  return (void *) c->data;
}

extern "C" void *__stdcall
cmalloc (cygheap_types x, DWORD n)
{
  cygheap_entry *c;
  c = (cygheap_entry *) _cmalloc (sizeof_cygheap (n));
  if (!c)
    system_printf ("cmalloc returned NULL");
  return creturn (x, c, n);
}

extern "C" void *__stdcall
crealloc (void *s, DWORD n)
{
  if (s == NULL)
    return cmalloc (HEAP_STR, n);	// kludge

  assert (!inheap (s));
  cygheap_entry *c = tocygheap (s);
  cygheap_types t = c->type;
  c = (cygheap_entry *) _crealloc (c, sizeof_cygheap (n));
  if (!c)
    system_printf ("crealloc returned NULL");
  return creturn (t, c, n);
}

extern "C" void __stdcall
cfree (void *s)
{
  assert (!inheap (s));
  (void) _cfree (tocygheap (s));
}

extern "C" void *__stdcall
ccalloc (cygheap_types x, DWORD n, DWORD size)
{
  cygheap_entry *c;
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
  char *p = (char *) cmalloc (HEAP_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  return p;
}

extern "C" char *__stdcall
cstrdup1 (const char *s)
{
  char *p = (char *) cmalloc (HEAP_1_STR, strlen (s) + 1);
  if (!p)
    return NULL;
  strcpy (p, s);
  return p;
}
