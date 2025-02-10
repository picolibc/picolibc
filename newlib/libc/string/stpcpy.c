/*
FUNCTION
	<<stpcpy>>---copy string returning a pointer to its end

INDEX
	stpcpy

SYNOPSIS
	#include <string.h>
	char *stpcpy(char *restrict <[dst]>, const char *restrict <[src]>);

DESCRIPTION
	<<stpcpy>> copies the string pointed to by <[src]>
	(including the terminating null character) to the array
	pointed to by <[dst]>.

RETURNS
	This function returns a pointer to the end of the destination string,
	thus pointing to the trailing '\0'.

PORTABILITY
<<stpcpy>> is a GNU extension, candidate for inclusion into POSIX/SUSv4.

<<stpcpy>> requires no supporting OS subroutines.

QUICKREF
	stpcpy gnu
*/

#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

char*
stpcpy (char *__restrict dst,
	const char *__restrict src)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
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
#endif /* not PREFER_SIZE_OVER_SPEED */

  while ((*dst++ = *src++))
    ;
  return --dst;
}
