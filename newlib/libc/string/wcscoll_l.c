/*
FUNCTION
	<<wcscoll_l>>---locale-specific wide-character string compare
	
INDEX
	wcscoll_l

SYNOPSIS
	#include <wchar.h>
	int wcscoll_l(const wchar_t *<[stra]>, const wchar_t * <[strb]>,
		      locale_t <[locale]>);

DESCRIPTION
	<<wcscoll_l>> compares the wide-character string pointed to by
	<[stra]> to the wide-character string pointed to by <[strb]>,
	using an interpretation appropriate to the current <<LC_COLLATE>>
	state.

	(NOT Cygwin:) The current implementation of <<wcscoll_l>> simply
	uses <<wcscmp>> and does not support any language-specific sorting.

	If <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS
	If the first string is greater than the second string,
	<<wcscoll_l>> returns a number greater than zero.  If the two
	strings are equivalent, <<wcscoll_l>> returns zero.  If the first
	string is less than the second string, <<wcscoll_l>> returns a
	number less than zero.

PORTABILITY
<<wcscoll_l>> is POSIX-1.2008.
*/

#include <_ansi.h>
#include <wchar.h>

int
wcscoll_l (const wchar_t *a, const wchar_t *b, struct __locale_t *locale)
{
  return wcscmp (a, b);
}
