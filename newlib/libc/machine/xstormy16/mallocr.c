#include <malloc.h>

void *
_malloc_r (struct _reent *r, size_t sz)
{
  return malloc (sz);
}
