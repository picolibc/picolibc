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
	<<strpbrk>>---find characters in string

INDEX
	strpbrk

SYNOPSIS
	#include <string.h>
	char *strpbrk(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	This function locates the first occurence in the string
	pointed to by <[s1]> of any character in string pointed to by
	<[s2]> (excluding the terminating null character).

RETURNS
	<<strpbrk>> returns a pointer to the character found in <[s1]>, or a
	null pointer if no character from <[s2]> occurs in <[s1]>.

PORTABILITY
<<strpbrk>> requires no supporting OS subroutines.
*/

#include <string.h>

char *
strpbrk (const char *s1,
	const char *s2)
{
  const char *c = s2;

  while (*s1)
    {
      for (c = s2; *c; c++)
	{
	  if (*s1 == *c)
	    return (char *) s1;
	}
      s1++;
    }

  return (char *) NULL;
}
