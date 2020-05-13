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
	<<strupr>>---force string to uppercase
	
INDEX
	strupr

SYNOPSIS
	#include <string.h>
	char *strupr(char *<[a]>);

DESCRIPTION
	<<strupr>> converts each character in the string at <[a]> to
	uppercase.

RETURNS
	<<strupr>> returns its argument, <[a]>.

PORTABILITY
<<strupr>> is not widely portable.

<<strupr>> requires no supporting OS subroutines.

QUICKREF
	strupr
*/

#include <string.h>
#include <ctype.h>

char *
strupr (char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++)
    {
      *ucs = toupper(*ucs);
    }
  return s;
}
