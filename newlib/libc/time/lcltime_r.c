/*
 * localtime_r.c
 * Original Author: Adapted from tzcode maintained by Arthur David Olson.
 * Modifications:
 * - Changed to mktm_r and added __tzcalc_limits - 04/10/02, Jeff Johnston
 * - Fixed bug in mday computations - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Fixed bug in __tzcalc_limits - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Implement localtime_r() with gmtime_r() and the conditional code moved
 *   from _mktm_r() - 05/09/14, Freddie Chopin <freddie_chopin@op.pl>
 *
 * Converts the calendar time pointed to by tim_p into a broken-down time
 * expressed as local time. Returns a pointer to a structure containing the
 * broken-down time.
 */

#include "local.h"

struct tm *
_DEFUN (localtime_r, (tim_p, res),
	_CONST time_t *__restrict tim_p _AND
	struct tm *__restrict res)
{
  long offset;
  int hours, mins, secs;
  int year;
  __tzinfo_type *_CONST tz = __gettzinfo ();
  _CONST int *ip;

  res = gmtime_r (tim_p, res);

  year = res->tm_year + YEAR_BASE;
  ip = __month_lengths[isleap(year)];

  TZ_LOCK;
  _tzset_unlocked ();
  if (_daylight)
    {
      if (year == tz->__tzyear || __tzcalc_limits (year))
	res->tm_isdst = (tz->__tznorth
	  ? (*tim_p >= tz->__tzrule[0].change
	  && *tim_p < tz->__tzrule[1].change)
	  : (*tim_p >= tz->__tzrule[0].change
	  || *tim_p < tz->__tzrule[1].change));
      else
	res->tm_isdst = -1;
    }
  else
    res->tm_isdst = 0;

  offset = (res->tm_isdst == 1
    ? tz->__tzrule[1].offset
    : tz->__tzrule[0].offset);

  hours = (int) (offset / SECSPERHOUR);
  offset = offset % SECSPERHOUR;

  mins = (int) (offset / SECSPERMIN);
  secs = (int) (offset % SECSPERMIN);

  res->tm_sec -= secs;
  res->tm_min -= mins;
  res->tm_hour -= hours;

  if (res->tm_sec >= SECSPERMIN)
    {
      res->tm_min += 1;
      res->tm_sec -= SECSPERMIN;
    }
  else if (res->tm_sec < 0)
    {
      res->tm_min -= 1;
      res->tm_sec += SECSPERMIN;
    }
  if (res->tm_min >= MINSPERHOUR)
    {
      res->tm_hour += 1;
      res->tm_min -= MINSPERHOUR;
    }
  else if (res->tm_min < 0)
    {
      res->tm_hour -= 1;
      res->tm_min += MINSPERHOUR;
    }
  if (res->tm_hour >= HOURSPERDAY)
    {
      ++res->tm_yday;
      ++res->tm_wday;
      if (res->tm_wday > 6)
	res->tm_wday = 0;
      ++res->tm_mday;
      res->tm_hour -= HOURSPERDAY;
      if (res->tm_mday > ip[res->tm_mon])
	{
	  res->tm_mday -= ip[res->tm_mon];
	  res->tm_mon += 1;
	  if (res->tm_mon == 12)
	    {
	      res->tm_mon = 0;
	      res->tm_year += 1;
	      res->tm_yday = 0;
	    }
	}
    }
  else if (res->tm_hour < 0)
    {
      res->tm_yday -= 1;
      res->tm_wday -= 1;
      if (res->tm_wday < 0)
	res->tm_wday = 6;
      res->tm_mday -= 1;
      res->tm_hour += 24;
      if (res->tm_mday == 0)
	{
	  res->tm_mon -= 1;
	  if (res->tm_mon < 0)
	    {
	      res->tm_mon = 11;
	      res->tm_year -= 1;
	      res->tm_yday = 364 + isleap(res->tm_year + YEAR_BASE);
	    }
	  res->tm_mday = ip[res->tm_mon];
	}
    }
  TZ_UNLOCK;

  return (res);
}
