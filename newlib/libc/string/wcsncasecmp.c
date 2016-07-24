/*
FUNCTION
	<<wcsncasecmp>>---case-insensitive wide character string compare
	
INDEX
	wcsncasecmp

ANSI_SYNOPSIS
	#include <wchar.h>
	int wcsncasecmp(const wchar_t *<[a]>, const wchar_t * <[b]>, size_t <[length]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	int wcsncasecmp(<[a]>, <[b]>, <[length]>)
	wchar_t *<[a]>;
	wchar_t *<[b]>;
	size_t <[length]>

DESCRIPTION
	<<wcsncasecmp>> compares up to <[length]> wide characters
	from the string at <[a]> to the string at <[b]> in a 
	case-insensitive manner.

RETURNS

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to uppercase), <<wcsncasecmp>> returns a
	number greater than zero.  If the two strings are equivalent,
	<<wcsncasecmp>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<wcsncasecmp>> returns a
	number less than zero.

PORTABILITY
POSIX-1.2008

<<wcsncasecmp>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	wcsncasecmp
*/

#include <wchar.h>
#include <wctype.h>

int 
_DEFUN (wcsncasecmp, (s1, s2, n),
	_CONST wchar_t *s1 _AND
	_CONST wchar_t *s2 _AND
	size_t n)
{
  int d = 0;
  for ( ; n != 0; n--)
    {
      const int c1 = towlower (*s1++);
      const int c2 = towlower (*s2++);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
