/* vec_free.c - a wrapper for _free_r */
#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>

#ifndef _REENT_ONLY

void
vec_free (void *aptr)
{
  _free_r (_REENT, aptr);
}

#endif /* !_REENT_ONLY */
