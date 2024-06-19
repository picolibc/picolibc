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
 * asctime_r.c
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>

#define oob(x,a) ((unsigned)(x) >= sizeof(a)/sizeof(a[0]))
#define valid(x,a)   (oob(x,a) ? "???" : a[x])

char *
asctime_r (const struct tm *__restrict tim_p,
           char result[__restrict static __ASCTIME_SIZE])
{
  static const char day_name[7][3] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char mon_name[12][3] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  int n;

  n = snprintf (result, __ASCTIME_SIZE, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
                valid(tim_p->tm_wday, day_name),
                valid(tim_p->tm_mon, mon_name),
                tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
                tim_p->tm_sec, 1900 + tim_p->tm_year);

  if (n < 0)
      return NULL;

  if (n >= __ASCTIME_SIZE)
      goto eoverflow;

  return result;

eoverflow:
  errno = EOVERFLOW;
  return NULL;
}
