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
 * ctime_r.c
 */

#include <time.h>

char *
ctime_r (const time_t * tim_p,
        char * result)

{
  struct tm tm;
  return asctime_r (localtime_r (tim_p, &tm), result);
}
