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
	<<bcopy>>---copy memory regions

SYNOPSIS
	#include <strings.h>
	void bcopy(const void *<[in]>, void *<[out]>, size_t <[n]>);

DESCRIPTION
	This function copies <[n]> bytes from the memory region
	pointed to by <[in]> to the memory region pointed to by
	<[out]>.

	This function is implemented in term of <<memmove>>.

PORTABILITY
<<bcopy>> requires no supporting OS subroutines.

QUICKREF
	bcopy - pure
*/

#include <string.h>
#include <strings.h>

void
bcopy (const void *b1,
	void *b2,
	size_t length)
{
  memmove (b2, b1, length);
}
