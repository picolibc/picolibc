/*
 * gmtime_r.c
 * Original Author: Adapted from tzcode maintained by Arthur David Olson.
 * Modifications:
 * - Changed to mktm_r and added __tzcalc_limits - 04/10/02, Jeff Johnston
 * - Fixed bug in mday computations - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Fixed bug in __tzcalc_limits - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Move code from _mktm_r() to gmtime_r() - 05/09/14, Freddie Chopin <freddie_chopin@op.pl>
 *
 * Converts the calendar time pointed to by tim_p into a broken-down time
 * expressed as local time. Returns a pointer to a structure containing the
 * broken-down time.
 */

#include "local.h"

/* there are 97 leap years in 400-year periods */
#define DAYS_PER_400_YEARS	((400 - 97) * 365 + 97 * 366)
/* there are 24 leap years in 100-year periods */
#define DAYS_PER_100_YEARS	((100 - 24) * 365 + 24 * 366)
/* there is one leap year every 4 years */
#define DAYS_PER_4_YEARS	(3 * 365 + 366)

static _CONST int days_per_year[4] = {
  365,  /* 1970 or 1966 */
  365,  /* 1971 or 1967 */
  366,  /* 1972 or 1968 */
  365   /* 1973 or 1969 */
} ;

struct tm *
_DEFUN (gmtime_r, (tim_p, res),
	_CONST time_t *__restrict tim_p _AND
	struct tm *__restrict res)
{
  long days, rem;
  _CONST time_t lcltime = *tim_p;
  int year;
  int years400, years100, years4;
  int yearleap;
  _CONST int *ip;

  days = ((long)lcltime) / SECSPERDAY;
  rem = ((long)lcltime) % SECSPERDAY;
  while (rem < 0)
    {
      rem += SECSPERDAY;
      --days;
    }
  while (rem >= SECSPERDAY)
    {
      rem -= SECSPERDAY;
      ++days;
    }

  /* compute hour, min, and sec */
  res->tm_hour = (int) (rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  res->tm_min = (int) (rem / SECSPERMIN);
  res->tm_sec = (int) (rem % SECSPERMIN);

  /* compute day of week */
  if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
    res->tm_wday += DAYSPERWEEK;

  /* compute year & day of year */
  years400 = days / DAYS_PER_400_YEARS;
  days -= years400 * DAYS_PER_400_YEARS;
  years100 = days / DAYS_PER_100_YEARS;
  days -= years100 * DAYS_PER_100_YEARS;
  years4 = days / DAYS_PER_4_YEARS;
  days -= years4 * DAYS_PER_4_YEARS;

  year = EPOCH_YEAR + years400 * 400 + years100 * 100 + years4 * 4;

  /* the value left in days is based in 1970 */
  if (days < 0)
    {
      ip = &days_per_year[3];
      while (days < 0)
        {
          days += *ip--;
          --year;
        }
    }
  else
    {
      ip = &days_per_year[0];
      while (days >= *ip)
        {
          days -= *ip++;
          ++year;
        }
    }

  res->tm_year = year - YEAR_BASE;
  res->tm_yday = days;
  yearleap = isleap(year);
  ip = __month_lengths[yearleap];
  for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
    days -= ip[res->tm_mon];
  res->tm_mday = days + 1;

  res->tm_isdst = 0;

  return (res);
}
