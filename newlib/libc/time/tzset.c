/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
/*
FUNCTION
<<tzset>>---set timezone characteristics from TZ environment variable

INDEX
	tzset
INDEX
	_tzset_r

SYNOPSIS
	#include <time.h>
	void tzset(void);
	void _tzset_r (struct _reent *<[reent_ptr]>);

DESCRIPTION
<<tzset>> examines the TZ environment variable and sets up the three
external variables: <<_timezone>>, <<_daylight>>, and <<tzname>>.  The
value of <<_timezone>> shall be the offset from the current time zone
to GMT.  The value of <<_daylight>> shall be 0 if there is no daylight
savings time for the current time zone, otherwise it will be non-zero.
The <<tzname>> array has two entries: the first is the name of the
standard time zone, the second is the name of the daylight-savings time
zone.

The TZ environment variable is expected to be in the following POSIX
format:

  stdoffset1[dst[offset2][,start[/time1],end[/time2]]]

where: std is the name of the standard time-zone (minimum 3 chars)
       offset1 is the value to add to local time to arrive at Universal time
         it has the form:  hh[:mm[:ss]]
       dst is the name of the alternate (daylight-savings) time-zone (min 3 chars)
       offset2 is the value to add to local time to arrive at Universal time
         it has the same format as the std offset
       start is the day that the alternate time-zone starts
       time1 is the optional time that the alternate time-zone starts
         (this is in local time and defaults to 02:00:00 if not specified)
       end is the day that the alternate time-zone ends
       time2 is the time that the alternate time-zone ends
         (it is in local time and defaults to 02:00:00 if not specified)

Note that there is no white-space padding between fields.  Also note that
if TZ is null, the default is Universal GMT which has no daylight-savings
time.  If TZ is empty, the default EST5EDT is used.

The function <<_tzset_r>> is identical to <<tzset>> only it is reentrant
and is used for applications that use multiple threads.

RETURNS
There is no return value.

PORTABILITY
<<tzset>> is part of the POSIX standard.

Supporting OS subroutine required: None
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"

extern char *getenv(const char *);

static char __tzname_std[11];
static char __tzname_dst[11];
static char *prev_tzenv = NULL;

void
_tzset_unlocked (void)
{
  char *tzenv;
  unsigned short hh, mm, ss, m, w, d;
  int sign, n;
  int i, ch;
  __tzinfo_type *tz = __gettzinfo ();

  if ((tzenv = getenv ("TZ")) == NULL)
      {
	_timezone = 0;
	_daylight = 0;
	_tzname[0] = "GMT";
	_tzname[1] = "GMT";
	free(prev_tzenv);
	prev_tzenv = NULL;
	return;
      }

  if (prev_tzenv != NULL && strcmp(tzenv, prev_tzenv) == 0)
    return;

  free(prev_tzenv);
  prev_tzenv = malloc (strlen(tzenv) + 1);
  if (prev_tzenv != NULL)
    strcpy (prev_tzenv, tzenv);

  /* ignore implementation-specific format specifier */
  if (*tzenv == ':')
    ++tzenv;  

  if (sscanf (tzenv, "%10[^0-9,+-]%n", __tzname_std, &n) <= 0)
    return;
 
  tzenv += n;

  sign = 1;
  if (*tzenv == '-')
    {
      sign = -1;
      ++tzenv;
    }
  else if (*tzenv == '+')
    ++tzenv;

  mm = 0;
  ss = 0;
 
  if (sscanf (tzenv, "%hu%n:%hu%n:%hu%n", &hh, &n, &mm, &n, &ss, &n) < 1)
    return;
  
  tz->__tzrule[0].offset = sign * (ss + SECSPERMIN * mm + SECSPERHOUR * hh);
  _tzname[0] = __tzname_std;
  tzenv += n;
  
  if (sscanf (tzenv, "%10[^0-9,+-]%n", __tzname_dst, &n) <= 0)
    { /* No dst */
      _tzname[1] = _tzname[0];
      _timezone = tz->__tzrule[0].offset;
      _daylight = 0;
      return;
    }
  else
    _tzname[1] = __tzname_dst;

  tzenv += n;

  /* otherwise we have a dst name, look for the offset */
  sign = 1;
  if (*tzenv == '-')
    {
      sign = -1;
      ++tzenv;
    }
  else if (*tzenv == '+')
    ++tzenv;

  hh = 0;  
  mm = 0;
  ss = 0;
  
  n  = 0;
  if (sscanf (tzenv, "%hu%n:%hu%n:%hu%n", &hh, &n, &mm, &n, &ss, &n) <= 0)
    tz->__tzrule[1].offset = tz->__tzrule[0].offset - 3600;
  else
    tz->__tzrule[1].offset = sign * (ss + SECSPERMIN * mm + SECSPERHOUR * hh);

  tzenv += n;

  for (i = 0; i < 2; ++i)
    {
      if (*tzenv == ',')
        ++tzenv;

      if (*tzenv == 'M')
	{
	  if (sscanf (tzenv, "M%hu%n.%hu%n.%hu%n", &m, &n, &w, &n, &d, &n) != 3 ||
	      m < 1 || m > 12 || w < 1 || w > 5 || d > 6)
	    return;
	  
	  tz->__tzrule[i].ch = 'M';
	  tz->__tzrule[i].m = m;
	  tz->__tzrule[i].n = w;
	  tz->__tzrule[i].d = d;
	  
	  tzenv += n;
	}
      else 
	{
	  char *end;
	  if (*tzenv == 'J')
	    {
	      ch = 'J';
	      ++tzenv;
	    }
	  else
	    ch = 'D';
	  
	  d = strtoul (tzenv, &end, 10);
	  
	  /* if unspecified, default to US settings */
	  /* From 1987-2006, US was M4.1.0,M10.5.0, but starting in 2007 is
	   * M3.2.0,M11.1.0 (2nd Sunday March through 1st Sunday November)  */
	  if (end == tzenv)
	    {
	      if (i == 0)
		{
		  tz->__tzrule[0].ch = 'M';
		  tz->__tzrule[0].m = 3;
		  tz->__tzrule[0].n = 2;
		  tz->__tzrule[0].d = 0;
		}
	      else
		{
		  tz->__tzrule[1].ch = 'M';
		  tz->__tzrule[1].m = 11;
		  tz->__tzrule[1].n = 1;
		  tz->__tzrule[1].d = 0;
		}
	    }
	  else
	    {
	      tz->__tzrule[i].ch = ch;
	      tz->__tzrule[i].d = d;
	    }
	  
	  tzenv = end;
	}
      
      /* default time is 02:00:00 am */
      hh = 2;
      mm = 0;
      ss = 0;
      n = 0;
      
      if (*tzenv == '/')
	sscanf (tzenv, "/%hu%n:%hu%n:%hu%n", &hh, &n, &mm, &n, &ss, &n);

      tz->__tzrule[i].s = ss + SECSPERMIN * mm + SECSPERHOUR  * hh;
      
      tzenv += n;
    }

  __tzcalc_limits (tz->__tzyear);
  _timezone = tz->__tzrule[0].offset;  
  _daylight = tz->__tzrule[0].offset != tz->__tzrule[1].offset;
}

void
tzset (void)
{
  TZ_LOCK;
  _tzset_unlocked ();
  TZ_UNLOCK;
}
