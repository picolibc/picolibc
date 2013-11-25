/*
 * asctime_r.c
 */

#include <stdio.h>
#include <time.h>

char *
_DEFUN (asctime_r, (tim_p, result),
	_CONST struct tm *__restrict tim_p _AND
	char *__restrict result)
{
  static _CONST char day_name[7][3] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static _CONST char mon_name[12][3] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  siprintf (result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
	    day_name[tim_p->tm_wday], 
	    mon_name[tim_p->tm_mon],
	    tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
	    tim_p->tm_sec, 1900 + tim_p->tm_year);
  return result;
}
