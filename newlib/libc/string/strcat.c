/*
FUNCTION
	<<strcat>>---concatenate strings

INDEX
	strcat

SYNOPSIS
	#include <string.h>
	char *strcat(char *restrict <[dst]>, const char *restrict <[src]>);

DESCRIPTION
	<<strcat>> appends a copy of the string pointed to by <[src]>
	(including the terminating null character) to the end of the
	string pointed to by <[dst]>.  The initial character of
	<[src]> overwrites the null character at the end of <[dst]>.

RETURNS
	This function returns the initial value of <[dst]>

PORTABILITY
<<strcat>> is ANSI C.

<<strcat>> requires no supporting OS subroutines.

QUICKREF
	strcat ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

char *
strcat (char *__restrict s1,
	const char *__restrict s2)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *s = s1;

  while (*s1)
    s1++;

  while (*s1++ = *s2++)
    ;
  return s;
#else
  char *s = s1;

  /* Skip unaligned memory in s1.  */
  while (UNALIGNED_X(s1) && *s1)
    s1++;

  if (*s1)
    {
      /* Skip over the aligned data in s1 as quickly as possible.  */
      unsigned long *aligned_s1 = (unsigned long *)s1;
      while (!DETECT_NULL(*aligned_s1))
        aligned_s1++;
      s1 = (char *)aligned_s1;

      /* Find string terminator.  */
      while (*s1)
        s1++;
    }

  /* s1 now points to the its trailing null character, we can
     just use strcpy to do the work for us now.

     ?!? We might want to just include strcpy here.
     Also, this will cause many more unaligned string copies because
     s1 is much less likely to be aligned.  I don't know if its worth
     tweaking strcpy to handle this better.  */
  strcpy (s1, s2);
	
  return s;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
