/*
FUNCTION
<<tzset>>---set timezone characteristics from <[TZ]> environment variable

INDEX
	tzset
INDEX
	_tzset_r

SYNOPSIS
	#include <time.h>
	void tzset(void);
	void _tzset_r (struct _reent *<[reent_ptr]>);

DESCRIPTION
<<tzset>> examines the <[TZ]> environment variable and sets up the three
external variables: <<_timezone>>, <<_daylight>>, and <<tzname>>.
The value of <<_timezone>> shall be the offset from the current time
to Universal Time.
The value of <<_daylight>> shall be 0 if there is no daylight savings
time for the current time zone, otherwise it will be non-zero.
The <<tzname>> array has two entries: the first is the designation of the
standard time period, the second is the designation of the alternate time
period.

The <[TZ]> environment variable is expected to be in the following POSIX
format (spaces inserted for clarity):

    <[std]> <[offset1]> [<[dst]> [<[offset2]>] [,<[start]> [/<[time1]>], <[end]> [/<[time2]>]]]

where:

<[std]> is the designation for the standard time period (minimum 3,
maximum <<TZNAME_MAX>> bytes) in one of two forms:

*- quoted within angle bracket '<' '>' characters: portable numeric
sign or alphanumeric characters in the current locale; the
quoting characters are not included in the designation

*- unquoted: portable alphabetic characters in the current locale

<[offset1]> is the value to add to local standard time to get Universal Time;
it has the format:

    [<[S]>]<[hh]>[:<[mm]>[:<[ss]>]]

    where:

    <[S]> is an optional numeric sign character; if negative '-', the
    time zone is East of the International Reference
    Meridian; else it is positive and West, and '+' may be used

    <[hh]> is the required numeric hour between 0 and 24

    <[mm]> is the optional numeric minute between 0 and 59

    <[ss]> is the optional numeric second between 0 and 59

<[dst]> is the designation of the alternate (daylight saving or
summer) time period, with the same limits and forms as
the standard time period designation

<[offset2]> is the value to add to local alternate time to get
Universal Time; it has the same format as <[offset1]>

<[start]> is the date in the year that alternate time starts;
the form may be one of:
(quotes "'" around characters below are used only to distinguish literals)

    <[n]>	zero based Julian day (0-365), counting February 29 Leap days

    'J'<[n]>	one based Julian day (1-365), not counting February 29 Leap
    days; in all years day 59 is February 28 and day 60 is March 1

    'M'<[m]>'.'<[w]>'.'<[d]>
    month <[m]> (1-12) week <[w]> (1-5) day <[d]> (0-6) where week 1 is
    the first week in month <[m]> with day <[d]>; week 5 is the last
    week in the month; day 0 is Sunday

<[time1]> is the optional local time that alternate time starts, in
the same format as <[offset1]> without any sign, and defaults
to 02:00:00

<[end]> is the date in the year that alternate time ends, in the same
forms as <[start]>

<[time2]> is the optional local time that alternate time ends, in
the same format as <[offset1]> without any sign, and
defaults to 02:00:00

Note that there is no white-space padding between fields. Also note that
if <[TZ]> is null, the default is Universal Time which has no daylight saving
time. If <[TZ]> is empty, the default EST5EDT is used.

The function <<_tzset_r>> is identical to <<tzset>> only it is reentrant
and is used for applications that use multiple threads.

RETURNS
There is no return value.

PORTABILITY
<<tzset>> is part of the POSIX standard.

Supporting OS subroutine required: None
*/

#include <_ansi.h>
#include <reent.h>
#include <time.h>
#include "local.h"

void
_tzset_unlocked (void)
{
  _tzset_unlocked_r (_REENT);
}

void
tzset (void)
{
  TZ_LOCK;
  _tzset_unlocked_r (_REENT);
  TZ_UNLOCK;
}
