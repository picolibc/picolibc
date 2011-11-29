/* cygheap_malloc.h: Cygwin heap manager allocation functions.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Red Hat, Inc.

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
void __stdcall cfree (void *) __attribute__ ((regparm(1)));
void *__stdcall cmalloc (cygheap_types, DWORD) __attribute__ ((regparm(2)));
void *__stdcall crealloc (void *, DWORD) __attribute__ ((regparm(2)));
void *__stdcall ccalloc (cygheap_types, DWORD, DWORD) __attribute__ ((regparm(3)));
void *__stdcall cmalloc_abort (cygheap_types, DWORD) __attribute__ ((regparm(2)));
void *__stdcall crealloc_abort (void *, DWORD) __attribute__ ((regparm(2)));
void *__stdcall ccalloc_abort (cygheap_types, DWORD, DWORD) __attribute__ ((regparm(3)));
PWCHAR __stdcall cwcsdup (const PWCHAR) __attribute__ ((regparm(1)));
PWCHAR __stdcall cwcsdup1 (const PWCHAR) __attribute__ ((regparm(1)));
char *__stdcall cstrdup (const char *) __attribute__ ((regparm(1)));
char *__stdcall cstrdup1 (const char *) __attribute__ ((regparm(1)));
void __stdcall cfree_and_set (char *&, char * = NULL) __attribute__ ((regparm(2)));
}

#endif /*_CYGHEAP_MALLOC_H*/
