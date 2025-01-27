/*
FUNCTION
	<<strncat>>---concatenate strings

INDEX
	strncat

SYNOPSIS
	#include <string.h>
	char *strncat(char *restrict <[dst]>, const char *restrict <[src]>,
                      size_t <[length]>);

DESCRIPTION
	<<strncat>> appends not more than <[length]> characters from
	the string pointed to by <[src]> (including the	terminating
	null character) to the end of the string pointed to by
	<[dst]>.  The initial character of <[src]> overwrites the null
	character at the end of <[dst]>.  A terminating null character
	is always appended to the result

WARNINGS
	Note that a null is always appended, so that if the copy is
	limited by the <[length]> argument, the number of characters
	appended to <[dst]> is <<n + 1>>.

RETURNS
	This function returns the initial value of <[dst]>

PORTABILITY
<<strncat>> is ANSI C.

<<strncat>> requires no supporting OS subroutines.

QUICKREF
	strncat ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

char *
strncat (char *__restrict s1,
	const char *__restrict s2,
	size_t n)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *s = s1;

  while (*s1)
    s1++;
  while (n-- != 0 && (*s1++ = *s2++))
    {
      if (n == 0)
	*s1 = '\0';
    }

  return s;
#else
  char *s = s1;

  /* Skip unaligned memory in s1.  */
  while (UNALIGNED_X(s1) && *s1)
    s1++;

  /* Skip over the aligned data in s1 as quickly as possible.  */
  unsigned long *aligned_s1 = (unsigned long *)s1;
  while (!DETECT_NULL(*aligned_s1))
    aligned_s1++;
  s1 = (char *)aligned_s1;

  /* Find string terminator.  */
  while (*s1)
    s1++;

  /* s1 now points to the its trailing null character, now copy
     up to N bytes from S2 into S1 stopping if a NULL is encountered
     in S2.

     It is not safe to use strncpy here since it copies EXACTLY N
     characters, NULL padding if necessary.  */
  while (n-- != 0 && (*s1++ = *s2++))
    {
      if (n == 0)
	*s1 = '\0';
    }
	
  return s;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
