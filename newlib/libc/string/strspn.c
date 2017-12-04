/*
FUNCTION
	<<strspn>>---find initial match

INDEX
	strspn

SYNOPSIS
	#include <string.h>
	size_t strspn(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	This function computes the length of the initial segment of
	the string pointed to by <[s1]> which consists entirely of
	characters from the string pointed to by <[s2]> (excluding the
	terminating null character).

RETURNS
	<<strspn>> returns the length of the segment found.

PORTABILITY
<<strspn>> is ANSI C.

<<strspn>> requires no supporting OS subroutines.

QUICKREF
	strspn ansi pure
*/

#include <string.h>

size_t
strspn (const char *s1,
	const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    break;
	}
      if (*c == '\0')
	break;
      s1++;
    }

  return s1 - s;
}
