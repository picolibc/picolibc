/* cygheap_malloc.h: Cygwin heap manager allocation functions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGHEAP_MALLOC_H
#define _CYGHEAP_MALLOC_H

#undef cfree

enum cygheap_types
{
  HEAP_FHANDLER,
  HEAP_STR,
  HEAP_ARGV,
  HEAP_BUF,
  HEAP_MOUNT,
  HEAP_SIGS,
  HEAP_ARCHETYPES,
  HEAP_TLS,
  HEAP_COMMUNE,
  HEAP_USER,
  HEAP_1_START,
  HEAP_1_HOOK,
  HEAP_1_STR,
  HEAP_1_ARGV,
  HEAP_1_BUF,
  HEAP_1_EXEC,
  HEAP_1_MAX = 100,
  HEAP_2_STR,
  HEAP_2_DLL,
  HEAP_MMAP,
  HEAP_2_MAX = 200,
  HEAP_3_FHANDLER
};

extern "C" {
void cfree (void *);
void *cmalloc (cygheap_types, size_t);
void *crealloc (void *, size_t);
void *ccalloc (cygheap_types, size_t, size_t);
void *cmalloc_abort (cygheap_types, size_t);
void *crealloc_abort (void *, size_t);
void *ccalloc_abort (cygheap_types, size_t, size_t);
PWCHAR cwcsdup (PCWSTR);
PWCHAR cwcsdup1 (PCWSTR);
char *cstrdup (const char *);
char *cstrdup1 (const char *);
void cfree_and_set (char *&, char * = NULL);
}

#endif /*_CYGHEAP_MALLOC_H*/
