/* heap.cc: Cygwin heap manager.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

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
#include "ntdll.h"
#include <sys/param.h>

#define assert(x)

static unsigned page_const;

#define MINHEAP_SIZE (4 * 1024 * 1024)

static uintptr_t
eval_start_address ()
{
  /* Starting with Vista, Windows performs heap ASLR.  This spoils the entire
     region below 0x20000000 for us, because that region is used by Windows
     to randomize heap and stack addresses.  Therefore we put our heap into a
     safe region starting at 0x20000000.  This should work right from the start
     in 99% of the cases. */
  uintptr_t start_address = 0x20000000L;
  if ((uintptr_t) NtCurrentTeb () >= 0xbf000000L)
    {
      /* However, if we're running on a /3GB enabled 32 bit system or on
	 a 64 bit system, and the executable is large address aware, then
	 we know that we have spare 1 Gig (32 bit) or even 2 Gigs (64 bit)
	 virtual address space.  This memory region is practically unused
	 by Windows, only PEB and TEBs are allocated top-down here.  We use
	 the current TEB address as very simple test that this is a large
	 address aware executable.
	 The above test for an address beyond 0xbf000000 is supposed to
	 make sure that we really have 3GB on a 32 bit system.  XP and
	 later support smaller large address regions, but then it's not
	 that interesting for us to use it for the heap.
	 If the region is big enough, the heap gets allocated at its
	 start.  What we get are 0.999 or 1.999 Gigs of free contiguous
	 memory for heap, thread stacks, and shared memory regions. */
      start_address = 0x80000000L;
    }
  return start_address;
}

static unsigned
eval_initial_heap_size ()
{
  PIMAGE_DOS_HEADER dosheader;
  PIMAGE_NT_HEADERS32 ntheader;
  unsigned size;

  dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
  ntheader = (PIMAGE_NT_HEADERS32) ((PBYTE) dosheader + dosheader->e_lfanew);
  /* LoaderFlags is an obsolete DWORD member of the PE/COFF file header.
     It's value is ignored by the loader, so we're free to use it for
     Cygwin.  If it's 0, we default to the usual 384 Megs.  Otherwise,
     we use it as the default initial heap size in megabyte.  Valid values
     are between 4 and 2048 Megs. */
  size = ntheader->OptionalHeader.LoaderFlags;
  if (size == 0)
    size = 384;
  else if (size < 4)
    size = 4;
  else if (size > 2048)
    size = 2048;
  return size << 20;
}

/* Initialize the heap at process start up.  */
void
heap_init ()
{
  const DWORD alloctype = MEM_RESERVE;
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we (almost) don't care where it ends up.  */

  page_const = wincap.page_size ();
  if (!cygheap->user_heap.base)
    {
      uintptr_t start_address = eval_start_address ();
      PVOID largest_found = NULL;
      size_t largest_found_size = 0;
      SIZE_T ret;
      MEMORY_BASIC_INFORMATION mbi;

      cygheap->user_heap.chunk = eval_initial_heap_size ();
      do
	{
	  cygheap->user_heap.base = VirtualAlloc ((LPVOID) start_address,
						  cygheap->user_heap.chunk,
						  alloctype, PAGE_NOACCESS);
	  if (cygheap->user_heap.base)
	    break;

	  /* Ok, so we are at the 1% which didn't work with 0x20000000 out
	     of the box.  What we do now is to search for the next free
	     region which matches our desired heap size.  While doing that,
	     we keep track of the largest region we found, including the
	     region starting at 0x20000000. */
	  while ((ret = VirtualQuery ((LPCVOID) start_address, &mbi,
				      sizeof mbi)) != 0)
	    {
	      if (mbi.State == MEM_FREE)
		{
		  if (mbi.RegionSize >= cygheap->user_heap.chunk)
		    break;
		  if (mbi.RegionSize > largest_found_size)
		    {
		      largest_found = mbi.BaseAddress;
		      largest_found_size = mbi.RegionSize;
		    }
		}
	      /* Since VirtualAlloc only reserves at allocation granularity
		 boundaries, we round up here, too.  Otherwise we might end
		 up at a bogus page-aligned address. */
	      start_address = roundup2 (start_address + mbi.RegionSize,
					wincap.allocation_granularity ());
	    }
	  if (!ret)
	    {
	      /* In theory this should not happen.  But if it happens, we have
		 collected the information about the largest available region
		 in the above loop.  So, next we squeeze the heap into that
		 region, unless it's smaller than the minimum size. */
	      if (largest_found_size >= MINHEAP_SIZE)
		{
		  cygheap->user_heap.chunk = largest_found_size;
		  cygheap->user_heap.base =
			VirtualAlloc (largest_found, cygheap->user_heap.chunk,
				      alloctype, PAGE_NOACCESS);
		}
	      /* Last resort (but actually we are probably broken anyway):
		 Use the minimal heap size and let the system decide. */
	      if (!cygheap->user_heap.base)
		{
		  cygheap->user_heap.chunk = MINHEAP_SIZE;
		  cygheap->user_heap.base =
			VirtualAlloc (NULL, cygheap->user_heap.chunk,
				      alloctype, PAGE_NOACCESS);
		}
	    }
	}
      while (!cygheap->user_heap.base && ret);
      if (cygheap->user_heap.base == NULL)
	api_fatal ("unable to allocate heap, heap_chunk_size %p, %E",
		   cygheap->user_heap.chunk);
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
      if (!p && in_forkee && !fork_info->abort (NULL))
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
