/*
FUNCTION
	<<strncasecmp_l>>---case-insensitive character string compare
	
INDEX
	strncasecmp_l

ANSI_SYNOPSIS
	#include <strings.h>
	int strncasecmp_l(const char *<[a]>, const char * <[b]>,
			  size_t <[length]>, locale_t <[locale]>);

DESCRIPTION
	<<strncasecmp_l>> compares up to <[length]> characters
	from the string at <[a]> to the string at <[b]> in a 
	case-insensitive manner.

	if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to lowercase), <<strncasecmp_l>> returns a
	number greater than zero.  If the two strings are equivalent,
	<<strncasecmp_l>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<strncasecmp_l>> returns a
	number less than zero.

PORTABILITY
<<strncasecmp_l>> is POSIX-1.2008.

<<strncasecmp_l>> requires no supporting OS subroutines. It uses
tolower_l() from elsewhere in this library.

QUICKREF
	strncasecmp_l
*/

#include <strings.h>
#include <ctype.h>

int 
strncasecmp_l (const char *s1, const char *s2, size_t n,
	       struct __locale_t *locale)
{
  int d = 0;
  for ( ; n != 0; n--)
    {
      const int c1 = tolower_l (*s1++, locale);
      const int c2 = tolower_l (*s2++, locale);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
