/* heap.cc: Cygwin heap manager.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include "cygerrno.h"
#include "sigproc.h"
#include "pinfo.h"
#include "heap.h"
#include "shared_info.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "registry.h"
#include "cygwin_version.h"

#define assert(x)

static unsigned page_const;

extern "C" size_t getpagesize ();

#define MINHEAP_SIZE (4 * 1024 * 1024)

/* Initialize the heap at process start up.  */

void
heap_init ()
{
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we don't care where it ends up.  */

  page_const = system_info.dwPageSize;
  if (!cygheap->user_heap.base)
    {
      cygheap->user_heap.chunk = cygwin_shared->heap_chunk_size ();
      while (cygheap->user_heap.chunk >= MINHEAP_SIZE)
	{
	  /* Initialize page mask and default heap size.  Preallocate a heap
	   * to assure contiguous memory.  */
	  cygheap->user_heap.ptr = cygheap->user_heap.top =
	  cygheap->user_heap.base =
	    VirtualAlloc (NULL, cygheap->user_heap.chunk, MEM_RESERVE, PAGE_NOACCESS);
	  if (cygheap->user_heap.base)
	    break;
	  cygheap->user_heap.chunk -= 1 * 1024 * 1024;
	}
      if (cygheap->user_heap.base == NULL)
	api_fatal ("unable to allocate heap, heap_chunk_size %d, %E",
		   cygheap->user_heap.chunk);
    }
  else
    {
      DWORD chunk = cygheap->user_heap.chunk;	/* allocation chunk */
      /* total size commited in parent */
      DWORD allocsize = (char *) cygheap->user_heap.top -
			(char *) cygheap->user_heap.base;
      /* round up by chunk size */
      DWORD reserve_size = chunk * ((allocsize + (chunk - 1)) / chunk);

      /* Loop until we've managed to reserve an adequate amount of memory. */
      char *p;
MEMORY_BASIC_INFORMATION m;
(void) VirtualQuery (cygheap->user_heap.base, &m, sizeof (m));
      for (;;)
	{
	  p = (char *) VirtualAlloc (cygheap->user_heap.base, reserve_size,
				     MEM_RESERVE, PAGE_READWRITE);
	  if (p)
	    break;
	  if ((reserve_size -= page_const) <= allocsize)
	    break;
	}
      if (p == NULL)
{
system_printf ("unable to allocate heap %p, chunk %u, reserve %u, alloc %u, %E",
cygheap->user_heap.base, cygheap->user_heap.chunk,
reserve_size, allocsize);
system_printf ("base %p mem alloc base %p, state %p, size %d, %E",
cygheap->user_heap.base, m.AllocationBase, m.State, m.RegionSize);
error_start_init ("h:/gdbtest/gdb.exe < con > con"); try_to_debug ();
	api_fatal ("unable to allocate heap %p, chunk %u, reserve %u, alloc %u, %E",
		   cygheap->user_heap.base, cygheap->user_heap.chunk,
		   reserve_size, allocsize);
}
      if (p != cygheap->user_heap.base)
	api_fatal ("heap allocated but not at %p", cygheap->user_heap.base);
      if (!VirtualAlloc (cygheap->user_heap.base, allocsize, MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("MEM_COMMIT failed, %E");
    }

  debug_printf ("heap base %p, heap top %p", cygheap->user_heap.base,
		cygheap->user_heap.top);
  page_const--;
  malloc_init ();
}

#define pround(n) (((size_t)(n) + page_const) & ~page_const)

/* FIXME: This function no longer handles "split heaps". */

extern "C" void *
sbrk (int n)
{
  sigframe thisframe (mainthread);
  char *newtop, *newbrk;
  unsigned commitbytes, newbrksize;

  if (n == 0)
    return cygheap->user_heap.ptr;			/* Just wanted to find current cygheap->user_heap.ptr address */

  newbrk = (char *) cygheap->user_heap.ptr + n;	/* Where new cygheap->user_heap.ptr will be */
  newtop = (char *) pround (newbrk);		/* Actual top of allocated memory -
						   on page boundary */

  if (newtop == cygheap->user_heap.top)
    goto good;

  if (n < 0)
    {						/* Freeing memory */
      assert (newtop < cygheap->user_heap.top);
      n = (char *) cygheap->user_heap.top - newtop;
      if (VirtualFree (newtop, n, MEM_DECOMMIT)) /* Give it back to OS */
	goto good;				/*  Didn't take */
      else
	goto err;
    }

  assert (newtop > cygheap->user_heap.top);

  /* Need to grab more pages from the OS.  If this fails it may be because
   * we have used up previously reserved memory.  Or, we're just plumb out
   * of memory.  */
  commitbytes = pround (newtop - (char *) cygheap->user_heap.top);
  if (VirtualAlloc (cygheap->user_heap.top, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL)
    goto good;

  /* Couldn't allocate memory.  Maybe we can reserve some more.
     Reserve either the maximum of the standard cygwin_shared->heap_chunk_size () or the requested
     amount.  Then attempt to actually allocate it.  */

  if ((newbrksize = cygheap->user_heap.chunk) < commitbytes)
    newbrksize = commitbytes;

  if ((VirtualAlloc (cygheap->user_heap.top, newbrksize, MEM_RESERVE, PAGE_NOACCESS) != NULL) &&
      (VirtualAlloc (cygheap->user_heap.top, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL))
    goto good;

err:
  set_errno (ENOMEM);
  return (void *) -1;

good:
  void *oldbrk = cygheap->user_heap.ptr;
  cygheap->user_heap.ptr = newbrk;
  cygheap->user_heap.top = newtop;
  return oldbrk;
}
