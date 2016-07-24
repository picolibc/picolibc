/*
FUNCTION
	<<strcasecmp_l>>---case-insensitive character string compare
	
INDEX
	strcasecmp_l

ANSI_SYNOPSIS
	#include <strings.h>
	int strcasecmp_l(const char *<[a]>, const char *<[b]>,
			 locale_t <[locale]>);

DESCRIPTION
	<<strcasecmp_l>> compares the string at <[a]> to
	the string at <[b]> in a case-insensitive manner.

	if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS 

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to lowercase), <<strcasecmp_l>> returns a
	number greater than zero.  If the two strings match,
	<<strcasecmp_l>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<strcasecmp_l>> returns a
	number less than zero.

PORTABILITY
<<strcasecmp_l>> is POSIX-1.2008.

<<strcasecmp_l>> requires no supporting OS subroutines. It uses
tolower_l() from elsewhere in this library.

QUICKREF
	strcasecmp_l
*/

#include <strings.h>
#include <ctype.h>

int
strcasecmp_l (const char *s1, const char *s2, struct __locale_t *locale)
{
  int d = 0;
  for ( ; ; )
    {
      const int c1 = tolower_l (*s1++, locale);
      const int c2 = tolower_l (*s2++, locale);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
