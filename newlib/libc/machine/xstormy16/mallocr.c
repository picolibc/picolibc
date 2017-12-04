#include <malloc.h>

#ifdef DEFINE_MALLOC
void *
_malloc_r (struct _reent *r, size_t sz)
{
  return malloc (sz);
}
#endif

#ifdef DEFINE_CALLOC
void *
_calloc_r (struct _reent *r, size_t a, size_t b)
{
  return calloc (a, b);
}
#endif

#ifdef DEFINE_FREE
void
_free_r (struct _reent *r, void *x)
{
  free (x);
}
#endif

#ifdef DEFINE_REALLOC
void *
_realloc_r (struct _reent *r, void *x, size_t sz)
{
  return realloc (x, sz);
}
#endif
