/* cygheap_malloc.h: Cygwin heap manager allocation functions.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011,
   2014 Red Hat, Inc.

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
void __reg1 cfree (void *);
void *__reg2 cmalloc (cygheap_types, size_t);
void *__reg2 crealloc (void *, size_t);
void *__reg3 ccalloc (cygheap_types, size_t, size_t);
void *__reg2 cmalloc_abort (cygheap_types, size_t);
void *__reg2 crealloc_abort (void *, size_t);
void *__reg3 ccalloc_abort (cygheap_types, size_t, size_t);
PWCHAR __reg1 cwcsdup (PCWSTR);
PWCHAR __reg1 cwcsdup1 (PCWSTR);
char *__reg1 cstrdup (const char *);
char *__reg1 cstrdup1 (const char *);
void __reg2 cfree_and_set (char *&, char * = NULL);
}

#endif /*_CYGHEAP_MALLOC_H*/
