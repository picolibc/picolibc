/* heap.h: Cygwin heap manager definitions.

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Heap management. */
void heap_init (void);
void malloc_init (void);

#define inheap(s) (brk && ((char *) (s) >= (char *) brkbase) && ((char *) (s) <= (char *) brktop))

#define brksize ((char *) user_data->heaptop - (char *) user_data->heapbase)
#define brk (user_data->heapptr)
#define brkbase (user_data->heapbase)
#define brktop (user_data->heaptop)
#define brkchunk (cygwin_shared->heap_chunk_size ())

