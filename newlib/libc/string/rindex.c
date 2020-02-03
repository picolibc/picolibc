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
	<<rindex>>---reverse search for character in string

INDEX
	rindex

SYNOPSIS
	#include <string.h>
	char * rindex(const char *<[string]>, int <[c]>);

DESCRIPTION
	This function finds the last occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

	This function is identical to <<strrchr>>.

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<rindex>> requires no supporting OS subroutines.

QUICKREF
	rindex - pure
*/

#include <string.h>
#include <strings.h>

char *
rindex (const char *s,
	int c)
{
  return strrchr (s, c);
}
