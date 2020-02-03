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
<<bzero>>---initialize memory to zero

INDEX
	bzero

SYNOPSIS
	#include <strings.h>
	void bzero(void *<[b]>, size_t <[length]>);

DESCRIPTION
<<bzero>> initializes <[length]> bytes of memory, starting at address
<[b]>, to zero.

RETURNS
<<bzero>> does not return a result.

PORTABILITY
<<bzero>> is in the Berkeley Software Distribution.
Neither ANSI C nor the System V Interface Definition (Issue 2) require
<<bzero>>.

<<bzero>> requires no supporting OS subroutines.
*/

#include <string.h>

void
bzero(void *b, size_t length)
{

	memset(b, 0, length);
}
