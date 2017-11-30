/*
FUNCTION
	<<strcoll_l>>---locale-specific character string compare
	
INDEX
	strcoll_l

SYNOPSIS
	#include <string.h>
	int strcoll_l(const char *<[stra]>, const char * <[strb]>,
		      locale_t <[locale]>);

DESCRIPTION
	<<strcoll_l>> compares the string pointed to by <[stra]> to
	the string pointed to by <[strb]>, using an interpretation
	appropriate to the current <<LC_COLLATE>> state.

	(NOT Cygwin:) The current implementation of <<strcoll_l>> simply
	uses <<strcmp>> and does not support any language-specific sorting.

	If <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the
	behaviour is undefined.

RETURNS
	If the first string is greater than the second string,
	<<strcoll_l>> returns a number greater than zero.  If the two
	strings are equivalent, <<strcoll_l>> returns zero.  If the first
	string is less than the second string, <<strcoll_l>> returns a
	number less than zero.

PORTABILITY
<<strcoll_l>> is POSIX-1.2008.

<<strcoll_l>> requires no supporting OS subroutines.

QUICKREF
	strcoll_l ansi pure
*/

#include <string.h>

int
strcoll_l (const char *a, const char *b, struct __locale_t *locale)
{
  return strcmp (a, b);
}
