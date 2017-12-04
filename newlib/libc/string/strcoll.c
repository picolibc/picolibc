/*
FUNCTION
	<<strcoll>>---locale-specific character string compare
	
INDEX
	strcoll

SYNOPSIS
	#include <string.h>
	int strcoll(const char *<[stra]>, const char * <[strb]>);

DESCRIPTION
	<<strcoll>> compares the string pointed to by <[stra]> to
	the string pointed to by <[strb]>, using an interpretation
	appropriate to the current <<LC_COLLATE>> state.

	(NOT Cygwin:) The current implementation of <<strcoll>> simply
	uses <<strcmp>> and does not support any language-specific sorting.

RETURNS
	If the first string is greater than the second string,
	<<strcoll>> returns a number greater than zero.  If the two
	strings are equivalent, <<strcoll>> returns zero.  If the first
	string is less than the second string, <<strcoll>> returns a
	number less than zero.

PORTABILITY
<<strcoll>> is ANSI C.

<<strcoll>> requires no supporting OS subroutines.

QUICKREF
	strcoll ansi pure
*/

#include <string.h>

int
strcoll (const char *a,
	const char *b)

{
  return strcmp (a, b);
}
