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
<<time>>---get current calendar time (as single number)

INDEX
	time

SYNOPSIS
	#include <time.h>
	time_t time(time_t *<[t]>);

DESCRIPTION
<<time>> looks up the best available representation of the current
time and returns it, encoded as a <<time_t>>.  It stores the same
value at <[t]> unless the argument is <<NULL>>.

RETURNS
A <<-1>> result means the current time is not available; otherwise the
result represents the current time.

PORTABILITY
ANSI C requires <<time>>.

Supporting OS subroutine required: Some implementations require
<<gettimeofday>>.
*/

/* Most times we have a system call in newlib/libc/sys/.. to do this job */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/time.h>

time_t
time (time_t * t)
{
  struct timeval now;

  if (gettimeofday (&now, NULL) < 0)
    now.tv_sec = (time_t) -1;

  if (t)
    *t = now.tv_sec;
  return now.tv_sec;
}
