#include <malloc.h>

void
_free_r (struct _reent *r, void *x)
{
  free (x);
}
