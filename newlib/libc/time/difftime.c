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
 * difftime.c
 * Original Author:	G. Haley
 */

/*
FUNCTION
<<difftime>>---subtract two times

INDEX
	difftime

SYNOPSIS
	#include <time.h>
	double difftime(time_t <[tim1]>, time_t <[tim2]>);

DESCRIPTION
Subtracts the two times in the arguments: `<<<[tim1]> - <[tim2]>>>'.

RETURNS
The difference (in seconds) between <[tim2]> and <[tim1]>, as a <<double>>.

PORTABILITY
ANSI C requires <<difftime>>, and defines its result to be in seconds
in all implementations.

<<difftime>> requires no supporting OS subroutines.
*/

#include <time.h>

double
difftime (time_t tim1,
	time_t tim2)
{
  return (double)(tim1 - tim2);
}
