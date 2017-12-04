#ifdef MALLOC_PROVIDED
int _dummy_realloc = 1;
#else
/* realloc.c -- a wrapper for realloc_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

void *
realloc (void *ap,
	size_t nbytes)
{
  return _realloc_r (_REENT, ap, nbytes);
}

#endif
#endif /* MALLOC_PROVIDED */
