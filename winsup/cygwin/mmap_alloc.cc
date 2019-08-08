#ifdef __x86_64__

#include "winsup.h"
#include "mmap_alloc.h"
#include <sys/param.h>

/* FIXME?  Unfortunately the OS doesn't support a top down allocation with
	   a ceiling value.  The ZeroBits mechanism only works for
	   NtMapViewOfSection and it only evaluates the high bit of ZeroBits
	   on 64 bit, so it's pretty much useless for our purposes.

	   If the below simple mechanism to perform top-down allocations
	   turns out to be too dumb, we need something else.  One idea is to
	   dived the space in (3835) 4 Gig chunks and simply store the
	   available free space per slot.  Then we can go top down from slot
	   to slot and only try slots which are supposed to have enough space.
	   Bookkeeping would be very simple and fast. */
PVOID
mmap_allocator::alloc (PVOID in_addr, SIZE_T in_size, bool fixed)
{
  MEMORY_BASIC_INFORMATION mbi;

  SIZE_T size = roundup2 (in_size, wincap.allocation_granularity ());
  /* First check for the given address. */
  if (in_addr)
    {
      /* If it points to a free area, big enough to fulfill the request,
	 return the address. */
      if (VirtualQuery (in_addr, &mbi, sizeof mbi)
	  && mbi.State == MEM_FREE
	  && mbi.RegionSize >= size)
	return in_addr;
      /* Otherwise, if MAP_FIXED was given, give up. */
      if (fixed)
	return NULL;
      /* Otherwise, fall through to the usual free space search mechanism. */
    }
  /* Start with the last allocation start address - requested size. */
  caddr_t addr = mmap_current_low - size;
  bool merry_go_round = false;
  do
    {
      /* Did we hit the lower ceiling?  If so, restart from the upper
	 ceiling, but note that we did it. */
      if (addr < (caddr_t) MMAP_STORAGE_LOW)
	{
	  addr = (caddr_t) MMAP_STORAGE_HIGH - size;
	  merry_go_round = true;
	}
      /* Shouldn't fail, but better test. */
      if (!VirtualQuery ((PVOID) addr, &mbi, sizeof mbi))
	return NULL;
      /* If the region is free... */
      if (mbi.State == MEM_FREE)
	{
	  /* ...and the region is big enough to fulfill the request... */
	  if (mbi.RegionSize >= size)
	    {
	      /* ...note the address as next start address for our simple
		 merry-go-round and return the address. */
	      mmap_current_low = addr;
	      return (PVOID) addr;
	    }
	  /* Otherwise, subtract what's missing in size and try again. */
	  addr -= size - mbi.RegionSize;
	}
      /* If the region isn't free, skip to address below AllocationBase
	 and try again. */
      else
	addr = (caddr_t) mbi.AllocationBase - size;
    }
  /* Repeat until we had a full ride on the merry_go_round. */
  while (!merry_go_round || addr >= mmap_current_low);
  return NULL;
}

mmap_allocator mmap_alloc;	/* Inherited by forked child. */

#endif
