/* cygheap.h: Cygwin heap manager.

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef cfree

enum cygheap_types
{
  HEAP_FHANDLER,
  HEAP_STR,
  HEAP_ARGV,
  HEAP_BUF,
  HEAP_1_START,
  HEAP_1_STR,
  HEAP_1_ARGV,
  HEAP_1_BUF,
  HEAP_1_EXEC
};

#define CYGHEAPSIZE ((1000 * sizeof (fhandler_union)) + (2 * 65536))

extern HANDLE cygheap;
extern HANDLE cygheap_max;

#define incygheap(s) (cygheap && ((char *) (s) >= (char *) cygheap) && ((char *) (s) <= ((char *) cygheap) + CYGHEAPSIZE))

void cygheap_init ();
extern "C" {
void __stdcall cfree (void *);
void __stdcall cygheap_fixup_in_child (HANDLE);
void *__stdcall cmalloc (cygheap_types, DWORD);
void *__stdcall crealloc (void *, DWORD);
void *__stdcall ccalloc (cygheap_types, DWORD, DWORD);
char *__stdcall cstrdup (const char *);
char *__stdcall cstrdup1 (const char *);
}
