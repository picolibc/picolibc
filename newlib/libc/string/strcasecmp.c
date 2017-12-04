/*
FUNCTION
	<<strcasecmp>>---case-insensitive character string compare
	
INDEX
	strcasecmp

SYNOPSIS
	#include <strings.h>
	int strcasecmp(const char *<[a]>, const char *<[b]>);

DESCRIPTION
	<<strcasecmp>> compares the string at <[a]> to
	the string at <[b]> in a case-insensitive manner.

RETURNS 

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to lowercase), <<strcasecmp>> returns a
	number greater than zero.  If the two strings match,
	<<strcasecmp>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<strcasecmp>> returns a
	number less than zero.

PORTABILITY
<<strcasecmp>> is in the Berkeley Software Distribution.

<<strcasecmp>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	strcasecmp
*/

#include <strings.h>
#include <ctype.h>

int
strcasecmp (const char *s1,
	const char *s2)
{
  int d = 0;
  for ( ; ; )
    {
      const int c1 = tolower(*s1++);
      const int c2 = tolower(*s2++);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
