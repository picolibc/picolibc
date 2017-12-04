/* vec_realloc.c -- a wrapper for _vec_realloc_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>

#ifndef _REENT_ONLY

void *
vec_realloc (void *ap,
	size_t nbytes)
{
  return _vec_realloc_r (_REENT, ap, nbytes);
}

#endif
