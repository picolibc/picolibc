/*
FUNCTION
	<<wcsncasecmp_l>>---case-insensitive wide character string compare
	
INDEX
	wcsncasecmp_l

SYNOPSIS
	#include <wchar.h>
	int wcsncasecmp_l(const wchar_t *<[a]>, const wchar_t * <[b]>,
			  size_t <[length]>, locale_t <[locale]>);

DESCRIPTION
	<<wcsncasecmp_l>> compares up to <[length]> wide characters
	from the string at <[a]> to the string at <[b]> in a 
	case-insensitive manner.

	if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to uppercase), <<wcsncasecmp_l>> returns a
	number greater than zero.  If the two strings are equivalent,
	<<wcsncasecmp_l>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<wcsncasecmp_l>> returns a
	number less than zero.

PORTABILITY
POSIX-1.2008

<<wcsncasecmp_l>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	wcsncasecmp_l
*/

#include <wchar.h>
#include <wctype.h>

int 
wcsncasecmp_l (const wchar_t *s1, const wchar_t *s2, size_t n,
	       struct __locale_t *locale)
{
  int d = 0;
  for ( ; n != 0; n--)
    {
      const int c1 = towlower_l (*s1++, locale);
      const int c2 = towlower_l (*s2++, locale);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
