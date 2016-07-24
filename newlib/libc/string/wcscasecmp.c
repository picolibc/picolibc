/*
FUNCTION
	<<wcscasecmp>>---case-insensitive wide character string compare
	
INDEX
	wcscasecmp

ANSI_SYNOPSIS
	#include <wchar.h>
	int wcscasecmp(const wchar_t *<[a]>, const wchar_t *<[b]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	int wcscasecmp(<[a]>, <[b]>)
	wchar_t *<[a]>;
	wchar_t *<[b]>;

DESCRIPTION
	<<wcscasecmp>> compares the wide character string at <[a]> to
	the wide character string at <[b]> in a case-insensitive manner.

RETURNS 

	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>> (after
	both are converted to uppercase), <<wcscasecmp>> returns a
	number greater than zero.  If the two strings match,
	<<wcscasecmp>> returns zero.  If <<*<[a]>>> sorts
	lexicographically before <<*<[b]>>>, <<wcscasecmp>> returns a
	number less than zero.

PORTABILITY
POSIX-1.2008

<<wcscasecmp>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	wcscasecmp 
*/

#include <wchar.h>
#include <wctype.h>

int
_DEFUN (wcscasecmp, (s1, s2),
	_CONST wchar_t *s1 _AND
	_CONST wchar_t *s2)
{
  int d = 0;
  for ( ; ; )
    {
      const int c1 = towlower (*s1++);
      const int c2 = towlower (*s2++);
      if (((d = c1 - c2) != 0) || (c2 == '\0'))
        break;
    }
  return d;
}
