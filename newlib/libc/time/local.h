/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
/* local header used by libc/time routines */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <_ansi.h>
#include <time.h>
#include <sys/_tz_structs.h>
#include <sys/lock.h>

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define YEAR_BASE	1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4
#define EPOCH_YEARS_SINCE_LEAP 2
#define EPOCH_YEARS_SINCE_CENTURY 70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

static inline int isleap (int y)
{
  // This routine must return exactly 0 or 1, because the result is used to index on __month_lengths[].
  // The order of checks below is the fastest for a random year.
  return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

int         __tzcalc_limits (int __year);

extern const uint8_t __month_lengths[2][MONSPERYEAR];

void _tzset_unlocked (void);

/* locks for multi-threading */
#define TZ_LOCK		__LIBC_LOCK()
#define TZ_UNLOCK	__LIBC_UNLOCK()

