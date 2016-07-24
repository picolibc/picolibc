/*
FUNCTION
	<<wcscasecmp_l>>---case-insensitive wide character string compare
	
INDEX
	wcscasecmp_l

ANSI_SYNOPSIS
	#include <wchar.h>
	int wcscasecmp_l(const wchar_t *<[a]>, const wchar_t *<[b]>,
			 locale_t <[locale]>);

DESCRIPTION
	<<wcscasecmp_l>> compares the wide character string at <[a]> to
	the wide character string at <[b]> in a case-insensitive manner.

	if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object,
	the behaviour is undefined.

RETURNS 

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to uppercase), <<wcscasecmp_l>> returns a
	number greater than zero.  If the two strings match,
	<<wcscasecmp_l>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<wcscasecmp_l>> returns a
	number less than zero.

PORTABILITY
<<wcscasecmp_l>> is POSIX-1.2008

<<wcscasecmp_l>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	wcscasecmp_l 
*/

#include <wchar.h>
#include <wctype.h>

int
wcscasecmp_l (const wchar_t *s1, const wchar_t *s2, struct __locale_t *locale)
{
  int d = 0;
  for ( ; ; )
    {
      const int c1 = towlower_l (*s1++, locale);
      const int c2 = towlower_l (*s2++, locale);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
