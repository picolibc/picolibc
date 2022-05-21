#include <malloc.h>

void *
_realloc_r (struct _reent *r, void *x, size_t sz)
{
  return realloc (x, sz);
}
