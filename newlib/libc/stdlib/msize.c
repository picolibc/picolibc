#ifndef MALLOC_PROVIDED
/* msize.c -- a wrapper for malloc_usable_size.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _REENT_ONLY

size_t
malloc_usable_size (void *ptr)
{
  return _malloc_usable_size_r (_REENT, ptr);
}

#endif
#endif
