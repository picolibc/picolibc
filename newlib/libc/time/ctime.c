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
 * ctime.c
 * Original Author:	G. Haley
 */

/*
FUNCTION
<<ctime>>---convert time to local and format as string

INDEX
	ctime
INDEX
	ctime_r

SYNOPSIS
	#include <time.h>
	char *ctime(const time_t *<[clock]>);
	char *ctime_r(const time_t *<[clock]>, char *<[buf]>);

DESCRIPTION
Convert the time value at <[clock]> to local time (like <<localtime>>)
and format it into a string of the form
. Wed Jun 15 11:38:07 1988\n\0
(like <<asctime>>).

RETURNS
A pointer to the string containing a formatted timestamp.

PORTABILITY
ANSI C requires <<ctime>>.

<<ctime>> requires no supporting OS subroutines.
*/

#include <time.h>

#ifndef _REENT_ONLY

char *
ctime (const time_t * tim_p)
{
  return asctime (localtime (tim_p));
}

#endif
