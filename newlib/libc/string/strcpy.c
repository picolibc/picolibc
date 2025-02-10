/*
FUNCTION
	<<strcpy>>---copy string

INDEX
	strcpy

SYNOPSIS
	#include <string.h>
	char *strcpy(char *<[dst]>, const char *<[src]>);

DESCRIPTION
	<<strcpy>> copies the string pointed to by <[src]>
	(including the terminating null character) to the array
	pointed to by <[dst]>.

RETURNS
	This function returns the initial value of <[dst]>.

PORTABILITY
<<strcpy>> is ANSI C.

<<strcpy>> requires no supporting OS subroutines.

QUICKREF
	strcpy ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

char*
strcpy (char *dst0,
	const char *src0)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *s = dst0;

  while (*dst0++ = *src0++)
    ;

  return s;
#else
  char *dst = dst0;
  const char *src = src0;
  long *aligned_dst;
  const long *aligned_src;

  /* If SRC or DEST is unaligned, then copy bytes.  */
  if (!UNALIGNED_X_Y(src, dst))
    {
      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* SRC and DEST are both "long int" aligned, try to do "long int"
         sized copies.  */
      while (!DETECT_NULL(*aligned_src))
        {
          *aligned_dst++ = *aligned_src++;
        }

      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while ((*dst++ = *src++))
    ;
  return dst0;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
