/*
FUNCTION
	<<wcsxfrm_l>>---locale-specific wide-character string transformation
	
INDEX
	wcsxfrm_l

SYNOPSIS
	#include <wchar.h>
	int wcsxfrm_l(wchar_t *__restrict <[stra]>,
		      const wchar_t *__restrict <[strb]>, size_t <[n]>,
		      locale_t <[locale]>);

DESCRIPTION
	<<wcsxfrm_l>> transforms the wide-character string pointed to by
	<[strb]> to the wide-character string pointed to by <[stra]>,
	Comparing two transformed wide strings with <<wcscmp>> should return
	the same result as comparing the original strings with <<wcscoll>>.
	No more than <[n]> wide characters are transformed, including the
	trailing null character.

	If <[n]> is 0, <[stra]> may be a NULL pointer.

	If <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

	(NOT Cygwin:) The current implementation of <<wcsxfrm_l>> simply uses
	<<wcslcpy>> and does not support any language-specific transformations.

RETURNS
	<<wcsxfrm_l>> returns the length of the transformed wide character
	string.  if the return value is greater or equal to <[n]>, the
	content of <[stra]> is undefined.

PORTABILITY
<<wcsxfrm_l>> is POSIX-1.2008.
*/

#include <_ansi.h>
#include <wchar.h>

size_t
wcsxfrm_l (wchar_t *__restrict a, const wchar_t *__restrict b, size_t n,
	   struct __locale_t *locale)
{
  return wcslcpy (a, b, n);
}
