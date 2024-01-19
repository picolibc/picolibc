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
	<<strcspn>>---count characters not in string

INDEX
	strcspn

SYNOPSIS
	size_t strcspn(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	This function computes the length of the initial part of
	the string pointed to by <[s1]> which consists entirely of
	characters <[NOT]> from the string pointed to by <[s2]>
	(excluding the terminating null character).

RETURNS
	<<strcspn>> returns the length of the substring found.

PORTABILITY
<<strcspn>> is ANSI C.

<<strcspn>> requires no supporting OS subroutines.
 */

#include <string.h>

size_t
strcspn (const char *s1,
	const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    goto end;
	}
      s1++;
    }
end:
  return s1 - s;
}
