/*
FUNCTION
	<<strncasecmp>>---case-insensitive character string compare
	
INDEX
	strncasecmp

SYNOPSIS
	#include <strings.h>
	int strncasecmp(const char *<[a]>, const char * <[b]>, size_t <[length]>);

DESCRIPTION
	<<strncasecmp>> compares up to <[length]> characters
	from the string at <[a]> to the string at <[b]> in a 
	case-insensitive manner.

RETURNS

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to lowercase), <<strncasecmp>> returns a
	number greater than zero.  If the two strings are equivalent,
	<<strncasecmp>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<strncasecmp>> returns a
	number less than zero.

PORTABILITY
<<strncasecmp>> is in the Berkeley Software Distribution.

<<strncasecmp>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	strncasecmp
*/

#include <strings.h>
#include <ctype.h>

int 
strncasecmp (const char *s1,
	const char *s2,
	size_t n)
{
  int d = 0;
  for ( ; n != 0; n--)
    {
      const int c1 = tolower(*s1++);
      const int c2 = tolower(*s2++);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
