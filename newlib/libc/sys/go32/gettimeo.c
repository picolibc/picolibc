/*
  (c) Copyright 1992 Eric Backus

  This software may be used freely so long as this copyright notice is
  left intact.  There is no warrantee on this software.
*/

#include <time.h>
#include <sys/time.h>
#include "dos.h"

static int daylight, gmtoffset;

int
gettimeofday (struct timeval *tp, struct timezone *tzp)
{

  if (tp)
  {
    struct time t;
    struct date d;
    struct tm tmrec;

    gettime (&t);
    getdate (&d);
    tmrec.tm_year = d.da_year - 1900;
    tmrec.tm_mon = d.da_mon - 1;
    tmrec.tm_mday = d.da_day;
    tmrec.tm_hour = t.ti_hour;
    tmrec.tm_min = t.ti_min;
    tmrec.tm_sec = t.ti_sec;
/*    tmrec.tm_gmtoff = gmtoffset;*/
    tmrec.tm_isdst = daylight;
    tp->tv_sec = mktime (&tmrec);
    tp->tv_usec = t.ti_hund * (1000000 / 100);
  }
  if (tzp)
  {
    tzp->tz_minuteswest = gmtoffset;
    tzp->tz_dsttime = daylight;
  }

  return 0;
}

void
__gettimeofday_init ()
{
  time_t ltm, gtm;
  struct tm *lstm;

  daylight = 0;
  gmtoffset = 0;
  ltm = gtm = time (NULL);
  ltm = mktime (lstm = localtime (&ltm));
  gtm = mktime (gmtime (&gtm));
  daylight = lstm->tm_isdst;
  gmtoffset = (int)(gtm - ltm) / 60;

}

