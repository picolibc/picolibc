/* heap.h: Cygwin heap manager definitions.

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "perprocess.h"

/* Heap management. */
void heap_init ();
void malloc_init ();

#define inheap(s) \
  (cygheap->heapptr && ((char *) (s) >= (char *) cygheap->heapbase) \
   && ((char *) (s) <= (char *) cygheap->heaptop))
