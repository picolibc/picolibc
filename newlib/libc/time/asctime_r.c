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

char *
asctime_r (const struct tm *__restrict tim_p,
	char *__restrict result)
{
  static const char day_name[7][3] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char mon_name[12][3] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  sprintf (result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
	    day_name[tim_p->tm_wday], 
	    mon_name[tim_p->tm_mon],
	    tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
	    tim_p->tm_sec, 1900 + tim_p->tm_year);
  return result;
}
