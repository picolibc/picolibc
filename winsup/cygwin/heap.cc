/* heap.cc: Cygwin heap manager.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include "cygerrno.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "heap.h"
#include "shared_info.h"
#include "security.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

#define assert(x)

static unsigned page_const;

extern "C" size_t getpagesize ();

/* Initialize the heap at process start up.  */

void
heap_init ()
{
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we don't care where it ends up.  */

  page_const = system_info.dwPageSize;
  if (cygheap->heapbase)
    {
      DWORD chunk = cygwin_shared->heap_chunk_size ();	/* allocation chunk */
      /* total size commited in parent */
      DWORD allocsize = (char *) cygheap->heaptop - (char *) cygheap->heapbase;
      /* round up by chunk size */
      DWORD reserve_size = chunk * ((allocsize + (chunk - 1)) / chunk);

      /* Loop until we've managed to reserve an adequate amount of memory. */
      char *p;
      for (;;)
	{
	  p = (char *) VirtualAlloc (cygheap->heapbase, reserve_size,
				     MEM_RESERVE, PAGE_READWRITE);
	  if (p)
	    break;
	  if ((reserve_size -= page_const) <= allocsize)
	    break;
	}
      if (p == NULL)
	api_fatal ("1. unable to allocate heap %p, heap_chunk_size %d, pid %d, %E",
		   cygheap->heapbase, cygwin_shared->heap_chunk_size (), myself->pid);
      if (p != cygheap->heapbase)
	api_fatal ("heap allocated but not at %p", cygheap->heapbase);
      if (! VirtualAlloc (cygheap->heapbase, allocsize, MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("MEM_COMMIT failed, %E");
    }
  else
    {
      /* Initialize page mask and default heap size.  Preallocate a heap
       * to assure contiguous memory.  */
      cygheap->heapptr = cygheap->heaptop = cygheap->heapbase =
	VirtualAlloc(NULL, cygwin_shared->heap_chunk_size (), MEM_RESERVE,
	    	     PAGE_NOACCESS);
      if (cygheap->heapbase == NULL)
	api_fatal ("2. unable to allocate heap, heap_chunk_size %d, %E",
		   cygwin_shared->heap_chunk_size ());
    }

  debug_printf ("heap base %p, heap top %p", cygheap->heapbase,
      		cygheap->heaptop);
  page_const--;
  malloc_init ();
}

#define pround(n) (((size_t)(n) + page_const) & ~page_const)

/* FIXME: This function no longer handles "split heaps". */

extern "C" void *
_sbrk(int n)
{
  sigframe thisframe (mainthread);
  char *newtop, *newbrk;
  unsigned commitbytes, newbrksize;

  if (n == 0)
    return cygheap->heapptr;			/* Just wanted to find current cygheap->heapptr address */

  newbrk = (char *) cygheap->heapptr + n;	/* Where new cygheap->heapptr will be */
  newtop = (char *) pround (newbrk);		/* Actual top of allocated memory -
						   on page boundary */

  if (newtop == cygheap->heaptop)
    goto good;

  if (n < 0)
    {						/* Freeing memory */
      assert(newtop < cygheap->heaptop);
      n = (char *) cygheap->heaptop - newtop;
      if (VirtualFree(newtop, n, MEM_DECOMMIT)) /* Give it back to OS */
	goto good;				/*  Didn't take */
      else
	goto err;
    }

  assert(newtop > cygheap->heaptop);

  /* Need to grab more pages from the OS.  If this fails it may be because
   * we have used up previously reserved memory.  Or, we're just plumb out
   * of memory.  */
  commitbytes = pround (newtop - (char *) cygheap->heaptop);
  if (VirtualAlloc(cygheap->heaptop, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL)
    goto good;

  /* Couldn't allocate memory.  Maybe we can reserve some more.
     Reserve either the maximum of the standard cygwin_shared->heap_chunk_size () or the requested
     amount.  Then attempt to actually allocate it.  */

  if ((newbrksize = cygwin_shared->heap_chunk_size ()) < commitbytes)
    newbrksize = commitbytes;

  if ((VirtualAlloc(cygheap->heaptop, newbrksize, MEM_RESERVE, PAGE_NOACCESS) != NULL) &&
      (VirtualAlloc(cygheap->heaptop, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL))
    goto good;

err:
  set_errno (ENOMEM);
  return (void *) -1;

good:
  void *oldbrk = cygheap->heapptr;
  cygheap->heapptr = newbrk;
  cygheap->heaptop = newtop;
  return oldbrk;
}
