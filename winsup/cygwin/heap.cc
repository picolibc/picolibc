/* heap.cc: Cygwin heap manager.

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

static ptrdiff_t page_const;

/* Minimum size of the base heap. */
#define MINHEAP_SIZE (4 * 1024 * 1024)
/* Chunksize of subsequent heap reservations. */
#define RAISEHEAP_SIZE (1 * 1024 * 1024)

static uintptr_t
eval_start_address ()
{
#ifdef __x86_64__
  /* On 64 bit, we choose a fixed address outside the 32 bit area.  The
     executable starts at 0x1:00400000L, the Cygwin DLL starts at
     0x1:80040000L, other rebased DLLs are located in the region from
     0x2:00000000L up to 0x4:00000000L, -auto-image-based DLLs are located
     in the region from 0x4:00000000L up to 0x6:00000000L.  Leave another
     8 Gigs slack space, so lets start the heap at 0x8:00000000L. */
  uintptr_t start_address = 0x800000000L;
#else
  /* Windows performs heap ASLR.  This spoils the entire region below
     0x20000000 for us, because that region is used by Windows to randomize
     heap and stack addresses.  Therefore we put our heap into a safe region
     starting at 0x20000000.  This should work right from the start in 99%
     of the cases. */
  uintptr_t start_address = 0x20000000L;
  MEMORY_BASIC_INFORMATION mbi;

  if (VirtualQuery ((void *) 0xbf000000L, &mbi, sizeof mbi))
    {
      /* However, if we're running on a /3GB enabled 32 bit system or on
	 a 64 bit system, and the executable is large address aware, then
	 we know that we have spare 1 Gig (32 bit) or even 2 Gigs (64 bit)
	 virtual address space.  This memory region is practically unused
	 by Windows, only PEB and TEBs are allocated top-down here.

	 We used to use the current TEB address as very simple test that
	 this is a large address aware executable, but that fails on W10
	 WOW64 because the main TEB is apparently commited in the lower
	 2 Gigs these days.

	 The above test for address 0xbf000000 is supposed to make sure
	 that we really have 3GB on a 32 bit system.  Windows supports
	 smaller large address regions, but then it's not that interesting
	 for us to use it for the heap.  If the region is big enough, the
	 heap gets allocated at its start.  What we get are 0.999 or 1.999
	 Gigs of free contiguous memory for heap, thread stacks, and shared
	 memory regions. */
      start_address = 0x80000000L;
    }
#endif
  return start_address;
}

static SIZE_T
eval_initial_heap_size ()
{
  PIMAGE_DOS_HEADER dosheader;
  PIMAGE_NT_HEADERS ntheader;
  SIZE_T size;

  dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
  ntheader = (PIMAGE_NT_HEADERS) ((PBYTE) dosheader + dosheader->e_lfanew);
  /* LoaderFlags is an obsolete DWORD member of the PE/COFF file header.
     It's value is ignored by the loader, so we're free to use it for
     Cygwin.  If it's 0, we default to the usual 384 Megs on 32 bit and
     512 on 64 bit.  Otherwise, we use it as the default initial heap size
     in megabyte.  Valid values are between 4 and 2048/8388608 Megs. */

  size = ntheader->OptionalHeader.LoaderFlags;
#ifdef __x86_64__
  if (size == 0)
    size = 512;
  else if (size < 4)
    size = 4;
  else if (size > 8388608)
    size = 8388608;
#else
  if (size == 0)
    size = 384;
  else if (size < 4)
    size = 4;
  else if (size > 2048)
    size = 2048;
#endif
  return size << 20;
}

/* Initialize the heap at process start up.  */
void
user_heap_info::init ()
{
  const DWORD alloctype = MEM_RESERVE;
  /* If we're the forkee, we must allocate the heap at exactly the same place
     as our parent.  If not, we (almost) don't care where it ends up.  */

  page_const = wincap.page_size ();
  if (!base)
    {
      uintptr_t start_address = eval_start_address ();
      PVOID largest_found = NULL;
      SIZE_T largest_found_size = 0;
      SIZE_T ret;
      MEMORY_BASIC_INFORMATION mbi;

      chunk = eval_initial_heap_size ();
      do
	{
	  base = VirtualAlloc ((LPVOID) start_address, chunk, alloctype,
			       PAGE_NOACCESS);
	  if (base)
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
		  if (mbi.RegionSize >= chunk)
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
		  chunk = largest_found_size;
		  base = VirtualAlloc (largest_found, chunk, alloctype,
				       PAGE_NOACCESS);
		}
	      /* Last resort (but actually we are probably broken anyway):
		 Use the minimal heap size and let the system decide. */
	      if (!base)
		{
		  chunk = MINHEAP_SIZE;
		  base = VirtualAlloc (NULL, chunk, alloctype, PAGE_NOACCESS);
		}
	    }
	}
      while (!base && ret);
      if (base == NULL)
	api_fatal ("unable to allocate heap, heap_chunk_size %ly, %E",
		   chunk);
      ptr = top = base;
      max = (char *) base + chunk;
    }
  else
    {
      /* total size commited in parent */
      SIZE_T allocsize = (char *) top - (char *) base;

      /* Loop until we've managed to reserve an adequate amount of memory. */
      SIZE_T reserve_size = chunk * ((allocsize + (chunk - 1)) / chunk);

      /* With ptmalloc3 there's a good chance that there has been no memory
	 allocated on the heap.  If we don't check that, reserve_size will
	 be 0 and from there, the below loop will end up overallocating due
	 to integer overflow. */
      if (!reserve_size)
	reserve_size = chunk;

      char *p;
      while (1)
	{
	  p = (char *) VirtualAlloc (base, reserve_size, alloctype,
				     PAGE_READWRITE);
	  if (p)
	    break;
	  if ((reserve_size -= page_const) < allocsize)
	    break;
	}
      if (!p && in_forkee && !fork_info->abort (NULL))
	api_fatal ("couldn't allocate heap, %E, base %p, top %p, "
		   "reserve_size %ld, allocsize %ld, page_const %d",
		   base, top,
		   reserve_size, allocsize, page_const);
      if (p != base)
	api_fatal ("heap allocated at wrong address %p (mapped) "
		   "!= %p (expected)", p, base);
      if (allocsize && !VirtualAlloc (base, allocsize,
				      MEM_COMMIT, PAGE_READWRITE))
	api_fatal ("MEM_COMMIT failed, %E");
    }

  /* CV 2012-05-21: Moved printing heap size here from strace::activate.
     The value printed in strace.activate was always wrong, because at the
     time it's called, cygheap points to cygheap_dummy.  Above all, the heap
     size has not been evaluated yet, except in a forked child.  Since
     heap_init is called early, the heap size is printed pretty much at the
     start of the strace output, so there isn't anything lost. */
  debug_printf ("heap base %p, heap top %p, heap size %ly (%lu)",
		base, top, chunk, chunk);
  page_const--;
}

#define pround(n) (((size_t)(n) + page_const) & ~page_const)
/* Linux defines n to be intptr_t, newlib defines it to be ptrdiff_t.
   It shouldn't matter much, though, since the function is not standarized
   and sizeof(ptrdiff_t) == sizeof(intptr_t) anyway. */
extern "C" void *
sbrk (ptrdiff_t n)
{
  return cygheap->user_heap.sbrk (n);
}

void __reg2 *
user_heap_info::sbrk (ptrdiff_t n)
{
/* FIXME: This function no longer handles "split heaps". */

  char *newtop, *newbrk;
  SIZE_T commitbytes, newbrksize, reservebytes;

  if (n == 0)
    return ptr;					/* Just wanted to find current ptr
						   address */

  newbrk = (char *) ptr + n;			/* Where new cptr will be */
  newtop = (char *) pround (newbrk);		/* Actual top of allocated memory -
						   on page boundary */

  if (newtop == top)
    goto good;

  if (n < 0)
    {						/* Freeing memory */
      assert (newtop < top);
      n = (char *) top - newtop;
      /* FIXME: This doesn't work if we cross a virtual memory reservation
	 border.  If that happens, we have to free the space in multiple
	 VirtualFree calls, aligned to the former reservation borders. */
      if (VirtualFree (newtop, n, MEM_DECOMMIT)) /* Give it back to OS */
	goto good;
      goto err;					/*  Didn't take */
    }

  assert (newtop > top);

  /* Find the number of bytes to commit, rounded up to the nearest page. */
  commitbytes = pround (newtop - (char *) top);

  /* Need to grab more pages from the OS.  If this fails it may be because
     we have used up previously reserved memory.  Or, we're just plumb out
     of memory.  Only attempt to commit memory that we know we've previously
     reserved.  */
  if (newtop <= max)
    {
      if (VirtualAlloc (top, commitbytes, MEM_COMMIT, PAGE_READWRITE))
	goto good;
      goto err;
    }

  /* The remainder of the existing heap is too small to fulfill the memory
     request.  We have to extend the heap, so we reserve some more memory
     and then commit the remainder of the old heap, if any, and the rest of
     the required space from the extended heap. */

  /* For subsequent chunks following the base heap, reserve either 1 Megs
     per chunk, or the requested amount if it's bigger than 1 Megs. */
  reservebytes = commitbytes - ((char *) max - (char *) top);
  commitbytes -= reservebytes;
  if ((newbrksize = RAISEHEAP_SIZE) < reservebytes)
    newbrksize = reservebytes;

  if (VirtualAlloc (max, newbrksize, MEM_RESERVE, PAGE_NOACCESS)
      || VirtualAlloc (max, newbrksize = reservebytes, MEM_RESERVE,
		       PAGE_NOACCESS))
    {
      /* Now commit the requested memory.  Windows keeps all virtual
	 reservations separate, so we can't commit the two regions in a single,
	 combined call or we suffer an ERROR_INVALID_ADDRESS.  The same error
	 is returned when trying to VirtualAlloc 0 bytes, which would occur if
	 the existing heap was already full. */
      if ((!commitbytes || VirtualAlloc (top, commitbytes, MEM_COMMIT,
					 PAGE_READWRITE))
	  && VirtualAlloc (max, reservebytes, MEM_COMMIT, PAGE_READWRITE))
	{
	  max = (char *) max + pround (newbrksize);
	  goto good;
	}
      /* If committing the memory failed, we must free the extendend reserved
         region, otherwise any other try to fetch memory (for instance by using
	 mmap) may fail just because we still reserve memory we don't even know
	 about. */
      VirtualFree (max, newbrksize, MEM_RELEASE);
    }

err:
  set_errno (ENOMEM);
  return (void *) -1;

good:
  void *oldbrk = ptr;
  ptr = newbrk;
  top = newtop;
  return oldbrk;
}
