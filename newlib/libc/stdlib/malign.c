#ifndef MALLOC_PROVIDED
/* malign.c -- a wrapper for memalign_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

void *
memalign (size_t align,
	size_t nbytes)
{
  return _memalign_r (_REENT, align, nbytes);
}

#endif
#endif
