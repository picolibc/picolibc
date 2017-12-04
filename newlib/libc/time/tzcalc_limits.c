/*
 * tzcalc_limits.c
 * Original Author: Adapted from tzcode maintained by Arthur David Olson.
 * Modifications:
 * - Changed to mktm_r and added __tzcalc_limits - 04/10/02, Jeff Johnston
 * - Fixed bug in mday computations - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Fixed bug in __tzcalc_limits - 08/12/04, Alex Mogilnikov <alx@intellectronika.ru>
 * - Moved __tzcalc_limits() to separate file - 05/09/14, Freddie Chopin <freddie_chopin@op.pl>
 */

#include "local.h"

int
__tzcalc_limits (int year)
{
  int days, year_days, years;
  int i, j;
  __tzinfo_type *const tz = __gettzinfo ();

  if (year < EPOCH_YEAR)
    return 0;

  tz->__tzyear = year;

  years = (year - EPOCH_YEAR);

  year_days = years * 365 +
    (years - 1 + EPOCH_YEARS_SINCE_LEAP) / 4 -
    (years - 1 + EPOCH_YEARS_SINCE_CENTURY) / 100 +
    (years - 1 + EPOCH_YEARS_SINCE_LEAP_CENTURY) / 400;

  for (i = 0; i < 2; ++i)
    {
      if (tz->__tzrule[i].ch == 'J')
	{
	  /* The Julian day n (1 <= n <= 365). */
	  days = year_days + tz->__tzrule[i].d +
	    (isleap(year) && tz->__tzrule[i].d >= 60);
	  /* Convert to yday */
	  --days;
	}
      else if (tz->__tzrule[i].ch == 'D')
	days = year_days + tz->__tzrule[i].d;
      else
	{
	  const int yleap = isleap(year);
	  int m_day, m_wday, wday_diff;
	  const int *const ip = __month_lengths[yleap];

	  days = year_days;

	  for (j = 1; j < tz->__tzrule[i].m; ++j)
	    days += ip[j-1];

	  m_wday = (EPOCH_WDAY + days) % DAYSPERWEEK;

	  wday_diff = tz->__tzrule[i].d - m_wday;
	  if (wday_diff < 0)
	    wday_diff += DAYSPERWEEK;
	  m_day = (tz->__tzrule[i].n - 1) * DAYSPERWEEK + wday_diff;

	  while (m_day >= ip[j-1])
	    m_day -= DAYSPERWEEK;

	  days += m_day;
	}

      /* store the change-over time in GMT form by adding offset */
      tz->__tzrule[i].change = days * SECSPERDAY +
      tz->__tzrule[i].s + tz->__tzrule[i].offset;
    }

  tz->__tznorth = (tz->__tzrule[0].change < tz->__tzrule[1].change);

  return 1;
}
