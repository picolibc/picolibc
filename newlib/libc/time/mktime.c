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
 * mktime.c
 * Original Author:	G. Haley
 *
 * Converts the broken-down time, expressed as local time, in the structure
 * pointed to by tim_p into a calendar time value. The original values of the
 * tm_wday and tm_yday fields of the structure are ignored, and the original
 * values of the other fields have no restrictions. On successful completion
 * the fields of the structure are set to represent the specified calendar
 * time. Returns the specified calendar time. If the calendar time can not be
 * represented, returns the value (time_t) -1.
 *
 * Modifications:	Fixed tm_isdst usage - 27 August 2008 Craig Howland.
 * 			Added timegm - 15 May 2021 R. Diez.
 */

/*
FUNCTION
<<mktime>>, <<timegm>>---convert time to arithmetic representation

INDEX
	mktime
INDEX
	timegm

SYNOPSIS
	#include <time.h>
	time_t mktime(struct tm *<[timp]>);
	time_t timegm(struct tm *<[timp]>);

DESCRIPTION
<<mktime>> assumes the time at <[timp]> is a local time, and converts
its representation from the traditional representation defined by
<<struct tm>> into a representation suitable for arithmetic.

<<localtime>> is the inverse of <<mktime>>.

<<timegm>> is similar to <<mktime>>, but assumes that the time at
<[timp]> is Coordinated Universal Time (UTC).

<<timegm>> could be emulated by setting the TZ environment variable to UTC,
calling <<mktime>> and restoring the value of TZ. However, other concurrent
threads could be affected by the temporary change to TZ.

<<timegm>> is the inverse of <<gmtime>>.

<<timegm>> is available if _BSD_SOURCE || _SVID_SOURCE || _DEFAULT_SOURCE.

RETURNS
If the contents of the structure at <[timp]> do not form a valid
calendar time representation, the result is <<-1>>.  Otherwise, the
result is the time, converted to a <<time_t>> value.

PORTABILITY
ANSI C requires <<mktime>>.

<<timegm>> is a nonstandard GNU extension that is also present on the BSDs.

<<mktime>> and <<timegm>> require no supporting OS subroutines.
*/

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <time.h>
#include "local.h"


#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : __month_lengths[0][x])

static const int16_t _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define _DAYS_IN_YEAR(year) (isleap(year+YEAR_BASE) ? 366 : 365)

static void
set_tm_wday (long days, struct tm *tim_p)
{
  if ((tim_p->tm_wday = (days + 4) % 7) < 0)
    tim_p->tm_wday += 7;
}

static void 
validate_structure (struct tm *tim_p)
{
  div_t res;
  int days_in_feb = 28;

  /* calculate time & date to account for out of range values */
  if (tim_p->tm_sec < 0 || tim_p->tm_sec > 59)
    {
      res = div (tim_p->tm_sec, 60);
      tim_p->tm_min += res.quot;
      if ((tim_p->tm_sec = res.rem) < 0)
	{
	  tim_p->tm_sec += 60;
	  --tim_p->tm_min;
	}
    }

  if (tim_p->tm_min < 0 || tim_p->tm_min > 59)
    {
      res = div (tim_p->tm_min, 60);
      tim_p->tm_hour += res.quot;
      if ((tim_p->tm_min = res.rem) < 0)
	{
	  tim_p->tm_min += 60;
	  --tim_p->tm_hour;
        }
    }

  if (tim_p->tm_hour < 0 || tim_p->tm_hour > 23)
    {
      res = div (tim_p->tm_hour, 24);
      tim_p->tm_mday += res.quot;
      if ((tim_p->tm_hour = res.rem) < 0)
	{
	  tim_p->tm_hour += 24;
	  --tim_p->tm_mday;
        }
    }

  if (tim_p->tm_mon < 0 || tim_p->tm_mon > 11)
    {
      res = div (tim_p->tm_mon, 12);
      tim_p->tm_year += res.quot;
      if ((tim_p->tm_mon = res.rem) < 0)
        {
	  tim_p->tm_mon += 12;
	  --tim_p->tm_year;
        }
    }

  if (isleap (tim_p->tm_year+YEAR_BASE))
    days_in_feb = 29;

  if (tim_p->tm_mday <= 0)
    {
      while (tim_p->tm_mday <= 0)
	{
	  if (--tim_p->tm_mon == -1)
	    {
	      tim_p->tm_year--;
	      tim_p->tm_mon = 11;
	      days_in_feb =
		(isleap (tim_p->tm_year+YEAR_BASE) ?
		 29 : 28);
	    }
	  tim_p->tm_mday += _DAYS_IN_MONTH (tim_p->tm_mon);
	}
    }
  else
    {
      while (tim_p->tm_mday > _DAYS_IN_MONTH (tim_p->tm_mon))
	{
	  tim_p->tm_mday -= _DAYS_IN_MONTH (tim_p->tm_mon);
	  if (++tim_p->tm_mon == 12)
	    {
	      tim_p->tm_year++;
	      tim_p->tm_mon = 0;
	      days_in_feb =
		(isleap (tim_p->tm_year+YEAR_BASE) ?
		 29 : 28);
	    }
	}
    }
}

static time_t
mktime_utc (struct tm *tim_p, long *days_p)
{
  time_t tim = 0;
  long days = 0;
  int year;

  /* validate structure */
  validate_structure (tim_p);

  /* compute hours, minutes, seconds */
  tim += tim_p->tm_sec + (tim_p->tm_min * SECSPERMIN) +
    (tim_p->tm_hour * SECSPERHOUR);

  /* compute days in year */
  days += tim_p->tm_mday - 1;
  days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
  if (tim_p->tm_mon > 1 && isleap (tim_p->tm_year+YEAR_BASE))
    days++;

  /* compute day of the year */
  tim_p->tm_yday = days;

  if (tim_p->tm_year > 10000 || tim_p->tm_year < -10000)
      return (time_t) -1;

  /* compute days in other years */
  if ((year = tim_p->tm_year) > 70)
    {
      for (year = 70; year < tim_p->tm_year; year++)
	days += _DAYS_IN_YEAR (year);
    }
  else if (year < 70)
    {
      for (year = 69; year > tim_p->tm_year; year--)
	days -= _DAYS_IN_YEAR (year);
      days -= _DAYS_IN_YEAR (year);
    }

  /* compute total seconds */
  tim += (time_t)days * SECSPERDAY;

  *days_p = days;
  return tim;
}

time_t
mktime (struct tm *tim_p)
{
  long days;
  time_t tim;
  int year;
  int isdst=0;
  __tzinfo_type *tz;

  tim = mktime_utc (tim_p, &days);

  if (tim == (time_t) -1)
    return tim;

  year = tim_p->tm_year;

  tz = __gettzinfo ();

  TZ_LOCK;

  _tzset_unlocked ();

  if (_daylight)
    {
      int tm_isdst;
      int y = tim_p->tm_year + YEAR_BASE;
      /* Convert user positive into 1 */
      tm_isdst = tim_p->tm_isdst > 0  ?  1 : tim_p->tm_isdst;
      isdst = tm_isdst;

      if (y == tz->__tzyear || __tzcalc_limits (y))
	{
	  /* calculate start of dst in dst local time and 
	     start of std in both std local time and dst local time */
          time_t startdst_dst = tz->__tzrule[0].change
	    - (time_t) tz->__tzrule[1].offset;
	  time_t startstd_dst = tz->__tzrule[1].change
	    - (time_t) tz->__tzrule[1].offset;
	  time_t startstd_std = tz->__tzrule[1].change
	    - (time_t) tz->__tzrule[0].offset;
	  /* if the time is in the overlap between dst and std local times */
	  if (tim >= startstd_std && tim < startstd_dst)
	    ; /* we let user decide or leave as -1 */
          else
	    {
	      isdst = (tz->__tznorth
		       ? (tim >= startdst_dst && tim < startstd_std)
		       : (tim >= startdst_dst || tim < startstd_std));
 	      /* if user committed and was wrong, perform correction, but not
 	       * if the user has given a negative value (which
 	       * asks mktime() to determine if DST is in effect or not) */
 	      if (tm_isdst >= 0  &&  (isdst ^ tm_isdst) == 1)
		{
		  /* we either subtract or add the difference between
		     time zone offsets, depending on which way the user got it
		     wrong. The diff is typically one hour, or 3600 seconds,
		     and should fit in a 16-bit int, even though offset
		     is a long to accomodate 12 hours. */
		  int diff = (int) (tz->__tzrule[0].offset
				    - tz->__tzrule[1].offset);
		  if (!isdst)
		    diff = -diff;
		  tim_p->tm_sec += diff;
		  tim += diff;  /* we also need to correct our current time calculation */
		  int mday = tim_p->tm_mday;
		  validate_structure (tim_p);
		  mday = tim_p->tm_mday - mday;
		  /* roll over occurred */
		  if (mday) {
		    /* compensate for month roll overs */
		    if (mday > 1)
			  mday = -1;
		    else if (mday < -1)
			  mday = 1;
		    /* update days for wday calculation */
		    days += mday;
		    /* handle yday */
		    if ((tim_p->tm_yday += mday) < 0) {
			  --year;
			  tim_p->tm_yday = _DAYS_IN_YEAR(year) - 1;
		    } else {
			  mday = _DAYS_IN_YEAR(year);
			  if (tim_p->tm_yday > (mday - 1))
				tim_p->tm_yday -= mday;
		    }
		  }
		}
	    }
	}
    }

  /* add appropriate offset to put time in gmt format */
  if (isdst == 1)
    tim += (time_t) tz->__tzrule[1].offset;
  else /* otherwise assume std time */
    tim += (time_t) tz->__tzrule[0].offset;

  TZ_UNLOCK;

  /* reset isdst flag to what we have calculated */
  tim_p->tm_isdst = isdst;

  set_tm_wday(days, tim_p);
	
  return tim;
}

time_t
timegm (struct tm *tim_p)
{
  long days;
  time_t tim = mktime_utc (tim_p, &days);

  if (tim == (time_t) -1)
    return tim;

  /* The time is always UTC, so there is no daylight saving time. */
  tim_p->tm_isdst = 0;

  set_tm_wday(days, tim_p);

  return tim;
}
