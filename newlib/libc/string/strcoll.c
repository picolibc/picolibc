/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
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
