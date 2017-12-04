/*
FUNCTION
	<<wcsxfrm>>---locale-specific wide-character string transformation
	
INDEX
	wcsxfrm

SYNOPSIS
	#include <wchar.h>
	int wcsxfrm(wchar_t *__restrict <[stra]>,
		    const wchar_t *__restrict <[strb]>, size_t <[n]>);

DESCRIPTION
	<<wcsxfrm>> transforms the wide-character string pointed to by
	<[strb]> to the wide-character string pointed to by <[stra]>,
	Comparing two transformed wide strings with <<wcscmp>> should return
	the same result as comparing the original strings with <<wcscoll>>.
	No more than <[n]> wide characters are transformed, including the
	trailing null character.

	If <[n]> is 0, <[stra]> may be a NULL pointer.

	(NOT Cygwin:) The current implementation of <<wcsxfrm>> simply uses
	<<wcslcpy>> and does not support any language-specific transformations.

RETURNS
	<<wcsxfrm>> returns the length of the transformed wide character
	string.  if the return value is greater or equal to <[n]>, the
	content of <[stra]> is undefined.

PORTABILITY
<<wcsxfrm>> is ISO/IEC 9899/AMD1:1995 (ISO C).
*/

#include <_ansi.h>
#include <wchar.h>

size_t
wcsxfrm (wchar_t *__restrict a,
	const wchar_t *__restrict b,
	size_t n)

{
  return wcslcpy (a, b, n);
}
