/* heap.cc: Cygwin heap manager.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

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
#include "tty.h"
#include "shared_info.h"

#define assert(x)

static unsigned page_const = 0;

static  __inline__ int
getpagesize(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int)si.dwPageSize;
}

/* Initialize the heap at process start up.  */

void
heap_init ()
{
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we don't care where it ends up.  */

  page_const = getpagesize();
  if (brkbase)
    {
      DWORD chunk = brkchunk;	/* allocation chunk */
      /* total size commited in parent */
      DWORD allocsize = (char *) brktop - (char *) brkbase;
      /* round up by chunk size */
      DWORD reserve_size = chunk * ((allocsize + (chunk - 1)) / chunk);

      /* Loop until we've managed to reserve an adequate amount of memory. */
      char *p;
      for (;;)
	{
	  p = (char *) VirtualAlloc (brkbase, reserve_size,
				     MEM_RESERVE, PAGE_READWRITE);
	  if (p)
	    break;
	  if ((reserve_size -= page_const) <= allocsize)
	    break;
	}
      if (p == NULL)
	api_fatal ("1. unable to allocate heap, heap_chunk_size %d, pid %d, %E",
		   brkchunk, myself->pid);
      if (p != brkbase)
	api_fatal ("heap allocated but not at %p", brkbase);
      if (! VirtualAlloc (brkbase, allocsize, MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("MEM_COMMIT failed, %E");
    }
  else
    {
      /* Initialize page mask and default heap size.  Preallocate a heap
       * to assure contiguous memory.  */
      brk = brktop = brkbase = VirtualAlloc(NULL, brkchunk, MEM_RESERVE, PAGE_NOACCESS);
      if (brkbase == NULL)
	api_fatal ("2. unable to allocate heap, heap_chunk_size %d, %E",
		   brkchunk);
    }

  page_const--;
  malloc_init ();
}

#define pround(n) (((size_t)(n) + page_const) & ~page_const)

/* FIXME: This function no longer handles "split heaps". */

extern "C" void *
_sbrk(int n)
{
  char *newtop, *newbrk;
  unsigned commitbytes, newbrksize;

  if (n == 0)
    return brk;			/* Just wanted to find current brk address */

  newbrk = (char *) brk + n;	/* Where new brk will be */
  newtop = (char *) pround (newbrk); /* Actual top of allocated memory -
				   on page boundary */

  if (newtop == brktop)
    goto good;

  if (n < 0)
    {				/* Freeing memory */
      assert(newtop < brktop);
      n = (char *) brktop - newtop;
      if (VirtualFree(newtop, n, MEM_DECOMMIT)) /* Give it back to OS */
	goto good;		/*  Didn't take */
      else
	goto err;
    }

  assert(newtop > brktop);

  /* Need to grab more pages from the OS.  If this fails it may be because
   * we have used up previously reserved memory.  Or, we're just plumb out
   * of memory.  */
  commitbytes = pround (newtop - (char *) brktop);
  if (VirtualAlloc(brktop, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL)
    goto good;

  /* Couldn't allocate memory.  Maybe we can reserve some more.
     Reserve either the maximum of the standard brkchunk or the requested
     amount.  Then attempt to actually allocate it.  */

  if ((newbrksize = brkchunk) < commitbytes)
    newbrksize = commitbytes;

  if ((VirtualAlloc(brktop, newbrksize, MEM_RESERVE, PAGE_NOACCESS) != NULL) &&
      (VirtualAlloc(brktop, commitbytes, MEM_COMMIT, PAGE_READWRITE) != NULL))
    goto good;

err:
  set_errno (ENOMEM);
  return (void *) -1;

good:
  void *oldbrk = brk;
  brk = newbrk;
  brktop = newtop;
  return oldbrk;
}
