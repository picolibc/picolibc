#ifdef __x86_64__

/* The memory region used for memory maps */
#define MMAP_STORAGE_LOW	0x001000000000L	/* Leave 32 Gigs for heap. */
/* Up to Win 8 only supporting 44 bit address space, starting with Win 8.1
   48 bit address space. */
#define MMAP_STORAGE_HIGH	wincap.mmap_storage_high ()

class mmap_allocator
{
  caddr_t mmap_current_low;

public:
  mmap_allocator () : mmap_current_low ((caddr_t) MMAP_STORAGE_HIGH) {}

  PVOID alloc (PVOID in_addr, SIZE_T in_size, bool fixed);
};

extern mmap_allocator mmap_alloc;

#endif
