/* heap.cc: Cygwin heap manager.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"

#define assert(x)

static unsigned page_const;

#define MINHEAP_SIZE (4 * 1024 * 1024)

/* Initialize the heap at process start up.  */
void
heap_init ()
{
  const DWORD alloctype = MEM_RESERVE;
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we don't care where it ends up.  */

  page_const = system_info.dwPageSize;
  if (!cygheap->user_heap.base)
    {
      cygheap->user_heap.chunk = cygwin_shared->heap_chunk_size ();
      /* For some obscure reason Vista and 2003 sometimes reserve space after
	 calls to CreateProcess overlapping the spot where the heap has been
	 allocated.  This apparently spoils fork.  The behaviour looks quite
	 arbitrary.  Experiments on Vista show a memory size of 0x37e000 or
	 0x1fd000 overlapping the usual heap by at most 0x1ed000.  So what
	 we do here is to allocate the heap with an extra slop of (by default)
	 0x200000 and set the appropriate pointers to the start of the heap
	 area + slop.  A forking child then creates its heap at the new start
	 address and without the slop factor.  Since this is not entirely
	 foolproof we add a registry setting "heap_slop_in_mb" so the slop
	 factor can be influenced by the user if the need arises. */
      cygheap->user_heap.slop = cygwin_shared->heap_slop_size ();
      while (cygheap->user_heap.chunk >= MINHEAP_SIZE)
	{
	  /* Initialize page mask and default heap size.  Preallocate a heap
	   * to assure contiguous memory.  */
	  cygheap->user_heap.base =
	    VirtualAlloc (NULL, cygheap->user_heap.chunk
	    			+ cygheap->user_heap.slop,
			  alloctype, PAGE_NOACCESS);
	  if (cygheap->user_heap.base)
	    break;
	  cygheap->user_heap.chunk -= 1 * 1024 * 1024;
	}
      if (cygheap->user_heap.base == NULL)
	api_fatal ("unable to allocate heap, heap_chunk_size %p, slop %p, %E",
		   cygheap->user_heap.chunk, cygheap->user_heap.slop);
      cygheap->user_heap.base = (void *) ((char *) cygheap->user_heap.base
      						   + cygheap->user_heap.slop);
      cygheap->user_heap.ptr = cygheap->user_heap.top = cygheap->user_heap.base;
      cygheap->user_heap.max = (char *) cygheap->user_heap.base
			       + cygheap->user_heap.chunk;
    }
  else
    {
      DWORD chunk = cygheap->user_heap.chunk;	/* allocation chunk */
      /* total size commited in parent */
      DWORD allocsize = (char *) cygheap->user_heap.top -
			(char *) cygheap->user_heap.base;

      /* Loop until we've managed to reserve an adequate amount of memory. */
      char *p;
      DWORD reserve_size = chunk * ((allocsize + (chunk - 1)) / chunk);
      while (1)
	{
	  p = (char *) VirtualAlloc (cygheap->user_heap.base, reserve_size,
				     alloctype, PAGE_READWRITE);
	  if (p)
	    break;
	  if ((reserve_size -= page_const) < allocsize)
	    break;
	}
      if (!p && in_forkee && !fork_info->handle_failure (GetLastError ()))
	api_fatal ("couldn't allocate heap, %E, base %p, top %p, "
		   "reserve_size %d, allocsize %d, page_const %d",
		   cygheap->user_heap.base, cygheap->user_heap.top,
		   reserve_size, allocsize, page_const);
      if (p != cygheap->user_heap.base)
	api_fatal ("heap allocated at wrong address %p (mapped) != %p (expected)", p, cygheap->user_heap.base);
      if (allocsize && !VirtualAlloc (cygheap->user_heap.base, allocsize, MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("MEM_COMMIT failed, %E");
    }

  debug_printf ("heap base %p, heap top %p", cygheap->user_heap.base,
		cygheap->user_heap.top);
  page_const--;
  // malloc_init ();
}

#define pround(n) (((size_t)(n) + page_const) & ~page_const)

/* FIXME: This function no longer handles "split heaps". */

extern "C" void *
sbrk (int n)
{
  char *newtop, *newbrk;
  unsigned commitbytes, newbrksize;

  if (n == 0)
    return cygheap->user_heap.ptr;		/* Just wanted to find current cygheap->user_heap.ptr address */

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

  /* Find the number of bytes to commit, rounded up to the nearest page. */
  commitbytes = pround (newtop - (char *) cygheap->user_heap.top);

  /* Need to grab more pages from the OS.  If this fails it may be because
     we have used up previously reserved memory.  Or, we're just plumb out
     of memory.  Only attempt to commit memory that we know we've previously
     reserved.  */
  if (newtop <= cygheap->user_heap.max)
    {
      if (VirtualAlloc (cygheap->user_heap.top, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL)
	goto good;
    }

  /* Couldn't allocate memory.  Maybe we can reserve some more.
     Reserve either the maximum of the standard cygwin_shared->heap_chunk_size ()
     or the requested amount.  Then attempt to actually allocate it.  */
  if ((newbrksize = cygheap->user_heap.chunk) < commitbytes)
    newbrksize = commitbytes;

   if ((VirtualAlloc (cygheap->user_heap.top, newbrksize, MEM_RESERVE, PAGE_NOACCESS)
	|| VirtualAlloc (cygheap->user_heap.top, newbrksize = commitbytes, MEM_RESERVE, PAGE_NOACCESS))
       && VirtualAlloc (cygheap->user_heap.top, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL)
     {
	cygheap->user_heap.max = (char *) cygheap->user_heap.max + pround (newbrksize);
	goto good;
     }

err:
  set_errno (ENOMEM);
  return (void *) -1;

good:
  void *oldbrk = cygheap->user_heap.ptr;
  cygheap->user_heap.ptr = newbrk;
  cygheap->user_heap.top = newtop;
  return oldbrk;
}
