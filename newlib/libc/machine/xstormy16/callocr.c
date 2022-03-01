#include <malloc.h>

void *
_calloc_r (struct _reent *r, size_t a, size_t b)
{
  return calloc (a, b);
}
