/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
/* local header used by libc/time routines */
#define _GNU_SOURCE
#include <time.h>
#include <sys/lock.h>
#include <stdint.h>
#include <sys/syslimits.h>

#define SECSPERMIN                     60L
#define MINSPERHOUR                    60L
#define HOURSPERDAY                    24L
#define SECSPERHOUR                    (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY                     (SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK                    7
#define MONSPERYEAR                    12

#define YEAR_BASE                      1900
#define EPOCH_YEAR                     1970
#define EPOCH_WDAY                     4
#define EPOCH_YEARS_SINCE_LEAP         2
#define EPOCH_YEARS_SINCE_CENTURY      70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

typedef struct {
    char   ch;
    int    m; /* Month of year if ch=M */
    int    n; /* Week of month if ch=M */
    int    d; /* Day of week if ch=M, day of year if ch=J or ch=D */
    int    s; /* Time of day in seconds */
    time_t change;
    long   offset; /* Match type of _timezone. */
} tzrule_t;

typedef struct {
    int      north;
    int      year;
    tzrule_t rule[2];
} tzinfo_t;

static inline int
isleap(int y)
{
    // This routine must return exactly 0 or 1, because the result is used to index on
    // __month_lengths[]. The order of checks below is the fastest for a random year.
    return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

static inline int
isleap2(int y, int y_off)
{
    // range reduce to avoid integer overflow
    if (y_off)
        y = y % 400 + y_off;
    return isleap(y);
}

static inline clockid_t
timebase_to_clockid(int base)
{
    switch (base) {
    case TIME_UTC:
        return CLOCK_REALTIME;
    case TIME_MONOTONIC:
        return CLOCK_MONOTONIC;
    case TIME_ACTIVE:
        return CLOCK_PROCESS_CPUTIME_ID;
    case TIME_THREAD_ACTIVE:
        return CLOCK_THREAD_CPUTIME_ID;
    default:
        return (clockid_t)-1;
    }
}

int                  __tzcalc_limits(int __year);

extern const uint8_t __month_lengths[2][MONSPERYEAR];
extern char          __tzname_std[TZNAME_MAX + 2];
extern char          __tzname_dst[TZNAME_MAX + 2];
extern tzinfo_t      __tzinfo;
