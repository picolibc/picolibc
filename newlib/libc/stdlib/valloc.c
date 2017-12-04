#ifndef MALLOC_PROVIDED
/* valloc.c -- a wrapper for valloc_r and pvalloc_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

void *
valloc (size_t nbytes)
{
  return _valloc_r (_REENT, nbytes);
}

void *
pvalloc (size_t nbytes)
{
  return _pvalloc_r (_REENT, nbytes);
}

#endif
#endif
