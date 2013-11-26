/*
FUNCTION
	<<wcsncpy>>---copy part of a wide-character string 

ANSI_SYNOPSIS
	#include <wchar.h>
	wchar_t *wcsncpy(wchar_t *__restrict <[s1]>,
			const wchar_t *__restrict <[s2]>, size_t <[n]>);

TRAD_SYNOPSIS
	wchar_t *wcsncpy(<[s1]>, <[s2]>, <[n]>
	wchar_t *__restrict <[s1]>;
	const wchar_t *__restrict <[s2]>;
	size_t <[n]>;

DESCRIPTION
	The <<wcsncpy>> function copies not more than <[n]> wide-character codes
	(wide-character codes that follow a null wide-character code are not
	copied) from the array pointed to by <[s2]> to the array pointed to
	by <[s1]>. If copying takes place between objects that overlap, the
	behaviour is undefined.  Note that if <[s1]> contains more than <[n]>
	wide characters before its terminating null, the result is not
	null-terminated.

	If the array pointed to by <[s2]> is a wide-character string that is
	shorter than <[n]> wide-character codes, null wide-character codes are
	appended to the copy in the array pointed to by <[s1]>, until <[n]>
	wide-character codes in all are written. 

RETURNS
	The <<wcsncpy>> function returns <[s1]>; no return value is reserved to
	indicate an error. 

PORTABILITY
ISO/IEC 9899; POSIX.1.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <wchar.h>

wchar_t *
_DEFUN (wcsncpy, (s1, s2, n),
	wchar_t *__restrict s1 _AND
	_CONST wchar_t *__restrict s2 _AND
	size_t n)
{
  wchar_t *dscan=s1;

  while(n > 0)
    {
      --n;
      if((*dscan++ = *s2++) == L'\0')  break;
    }
  while(n-- > 0)  *dscan++ = L'\0';

  return s1;
}
