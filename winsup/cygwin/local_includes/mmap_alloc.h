#include "memory_layout.h"

class mmap_allocator
{
  caddr_t mmap_current_low;

public:
  mmap_allocator () : mmap_current_low ((caddr_t) MMAP_STORAGE_HIGH) {}

  PVOID alloc (PVOID in_addr, SIZE_T in_size, bool fixed);
};

extern mmap_allocator mmap_alloc;
