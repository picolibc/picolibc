/*
 * Copyright (c) 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include "locale_private.h"

static const int _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define SET_MDAY        (1 << 0)
#define SET_MON         (1 << 1)
#define SET_YEAR        (1 << 2)
#define SET_WDAY        (1 << 3)
#define SET_YDAY        (1 << 4)
#define SET_WEEK_SUN    (1 << 5)
#define SET_WEEK_MON    (1 << 6)
#define SET_WEEK_MON4   (1 << 7)
#define SET_AMPM        (1 << 8)
#define SET_YMD         (SET_YEAR | SET_MON | SET_MDAY)
#define SET_WEEK        (SET_WEEK_SUN | SET_WEEK_MON | SET_WEEK_MON4)

/*
 * tm_year is relative this year 
 */

#define TM_YEAR_BASE    1900

/*
 * Return TRUE iff `year' was a leap year.
 * Needed for strptime.
 */
static int
is_leap_year (int year)
{
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

/* Needed for strptime. */
static int
match_string (const char *__restrict *buf, const char * const*strs,
	      locale_t locale)
{
    int i = 0;

    for (i = 0; strs[i] != NULL; ++i) {
	int len = strlen (strs[i]);

	if (strncasecmp_l (*buf, strs[i], len, locale) == 0) {
	    *buf += len;
	    return i;
	}
    }
    return -1;
}

/* Needed for strptime. */
static int
first_day (int year)
{
    int ret = 4;

    while (--year >= 1970)
	ret = (ret + 365 + is_leap_year (year)) % 7;
    return ret;
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */

static void
set_week_number_sun (struct tm *timeptr, int wnum)
{
    int fday = first_day (timeptr->tm_year + TM_YEAR_BASE);

    timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = fday;
	timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */

static void
set_week_number_mon (struct tm *timeptr, int wnum)
{
    int fday = (first_day (timeptr->tm_year + TM_YEAR_BASE) + 6) % 7;

    timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = (fday + 1) % 7;
	timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */
static void
set_week_number_mon4 (struct tm *timeptr, int wnum)
{
    int fday = first_day (timeptr->tm_year + TM_YEAR_BASE);

    if (fday >= 4)
	wnum++;

    timeptr->tm_yday = (wnum - 1) * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
	timeptr->tm_wday = fday;
	timeptr->tm_yday = 0;
    }
}

char *
strptime_l (const char *buf, const char *format, struct tm *timeptr,
	    locale_t locale)
{
    char c;
    int ymd = 0;
    int wnum = 0;
    int ampm = 0;

    for (; (c = *format) != '\0'; ++format) {
	char *s;
	int ret;

	if (isspace_l ((unsigned char) c, locale)) {
	    while (isspace_l ((unsigned char) *buf, locale))
		++buf;
	} else if (c == '%' && format[1] != '\0') {
	    c = *++format;
	    if (c == 'E' || c == 'O')
		c = *++format;
	    switch (c) {
	    case 'A' :
		ret = match_string (&buf, TIME_WEEKDAY, locale);
		if (ret < 0)
		    return NULL;
		timeptr->tm_wday = ret;
		ymd |= SET_WDAY;
		break;
	    case 'a' :
		ret = match_string (&buf, TIME_WDAY, locale);
		if (ret < 0)
		    return NULL;
		timeptr->tm_wday = ret;
		ymd |= SET_WDAY;
		break;
	    case 'B' :
		ret = match_string (&buf, TIME_MONTH, locale);
		if (ret < 0)
		    return NULL;
		timeptr->tm_mon = ret;
		ymd |= SET_MON;
		break;
	    case 'b' :
	    case 'h' :
		ret = match_string (&buf, TIME_MON, locale);
		if (ret < 0)
		    return NULL;
		timeptr->tm_mon = ret;
		ymd |= SET_MON;
		break;
	    case 'C' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
		timeptr->tm_year = (ret * 100) - TM_YEAR_BASE;
		buf = s;
		ymd |= SET_YEAR;
		break;
	    case 'c' :		/* %a %b %e %H:%M:%S %Y */
		s = strptime_l (buf, TIME_C_FMT, timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		ymd |= SET_WDAY | SET_YMD;
		break;
	    case 'D' :		/* %m/%d/%y */
		s = strptime_l (buf, "%m/%d/%y", timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		ymd |= SET_YMD;
		break;
	    case 'd' :
	    case 'e' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
		timeptr->tm_mday = ret;
		buf = s;
		ymd |= SET_MDAY;
		break;
	    case 'F' :		/* %Y-%m-%d - GNU extension */
		s = strptime_l (buf, "%Y-%m-%d", timeptr, locale);
		if (s == NULL || s == buf)
		    return NULL;
		buf = s;
		ymd |= SET_YMD;
		break;
	    case 'H' :
	    case 'k' :		/* hour with leading space - GNU extension */
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 0 || 23 < ret)
                    return NULL;
		timeptr->tm_hour = ret;
		buf = s;
		break;
	    case 'I' :
	    case 'l' :		/* hour with leading space - GNU extension */
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 1 || 12 < ret)
                    return NULL;
		if (ret == 12)
                    ret = 0;
                timeptr->tm_hour = ret;
		buf = s;
		break;
	    case 'j' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 1 || 366 < ret)
                    return NULL;
		timeptr->tm_yday = ret - 1;
		buf = s;
		ymd |= SET_YDAY;
		break;
	    case 'm' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 1 || 12 < ret)
                    return NULL;
		timeptr->tm_mon = ret - 1;
		buf = s;
		ymd |= SET_MON;
		break;
	    case 'M' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 0 || 59 < ret)
                    return NULL;
		timeptr->tm_min = ret;
		buf = s;
		break;
	    case 'n' :
		if (*buf == '\n')
		    ++buf;
		else
		    return NULL;
		break;
	    case 'p' :
		ampm = match_string (&buf, TIME_AM_PM, locale);
		if (ampm < 0)
		    return NULL;
                ymd |= SET_AMPM;
		break;
	    case 'r' :		/* %I:%M:%S %p */
		s = strptime_l (buf, TIME_AMPM_FMT, timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'R' :		/* %H:%M */
		s = strptime_l (buf, "%H:%M", timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 's' :		/* seconds since Unix epoch - GNU extension */
		{
		    long long sec;
		    time_t t;
		    int save_errno;

		    save_errno = errno;
		    errno = 0;
		    sec = strtoll_l (buf, &s, 10, locale);
		    t = sec;
		    if (s == buf
			|| errno != 0
			|| t != sec
			|| localtime_r (&t, timeptr) != timeptr)
			return NULL;
		    errno = save_errno;
		    buf = s;
		    ymd |= SET_YDAY | SET_WDAY | SET_YMD;
		    break;
		}
	    case 'S' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 0 || 61 < ret)
                    return NULL;
		timeptr->tm_sec = ret;
		buf = s;
		break;
	    case 't' :
		if (*buf == '\t')
		    ++buf;
		else
		    return NULL;
		break;
	    case 'T' :		/* %H:%M:%S */
		s = strptime_l (buf, "%H:%M:%S", timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		break;
	    case 'u' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 1 || 7 < ret)
                    return NULL;
                if (ret == 7)
                    ret = 0;
		timeptr->tm_wday = ret;
		buf = s;
		ymd |= SET_WDAY;
		break;
	    case 'w' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 0 || 6 < ret)
                    return NULL;
		timeptr->tm_wday = ret;
		buf = s;
		ymd |= SET_WDAY;
		break;
	    case 'U' :
		wnum = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (wnum < 0 || 53 < wnum)
                    return NULL;
		buf = s;
		ymd |= SET_WEEK_SUN;
		break;
	    case 'V' :
		wnum = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (wnum < 1 || 53 < wnum)
                    return NULL;
		buf = s;
		ymd |= SET_WEEK_MON4;
		break;
	    case 'W' :
		wnum = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (wnum < 0 || 53 < wnum)
                    return NULL;
		buf = s;
		ymd |= SET_WEEK_MON;
		break;
	    case 'x' :
		s = strptime_l (buf, TIME_X_FMT, timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
		ymd |= SET_YMD;
		break;
	    case 'X' :
		s = strptime_l (buf, TIME_UX_FMT, timeptr, locale);
		if (s == NULL)
		    return NULL;
		buf = s;
	    	break;
	    case 'y' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
                if (ret < 0 || 99 < ret)
                    return NULL;
		if (ret < 70)
		    timeptr->tm_year = 100 + ret;
		else
		    timeptr->tm_year = ret;
		buf = s;
		ymd |= SET_YEAR;
		break;
	    case 'Y' :
		ret = strtol_l (buf, &s, 10, locale);
		if (s == buf)
		    return NULL;
		timeptr->tm_year = ret - TM_YEAR_BASE;
		buf = s;
		ymd |= SET_YEAR;
		break;
	    case 'Z' :
		/* Unsupported. Just ignore.  */
		break;
	    case '\0' :
		--format;
		__fallthrough;
	    case '%' :
		if (*buf == '%')
		    ++buf;
		else
		    return NULL;
		break;
	    default :
		if (*buf == '%' || *++buf == c)
		    ++buf;
		else
		    return NULL;
		break;
	    }
	} else {
	    if (*buf == c)
		++buf;
	    else
		return NULL;
	}
    }

    if ((ymd & SET_WEEK) != 0 && (ymd & (SET_YEAR|SET_WDAY)) == (SET_YEAR|SET_WDAY)) {
        /* tm_year and week number, compute tm_yday */

        if (ymd & SET_WEEK_SUN)
            set_week_number_sun (timeptr, wnum);
        else if (ymd & SET_WEEK_MON4)
            set_week_number_mon4 (timeptr, wnum);
        else if (ymd & SET_WEEK_MON)
            set_week_number_mon (timeptr, wnum);
        ymd |= SET_YDAY;
    }

    if (ymd & SET_AMPM) {
        if (timeptr->tm_hour > 12)
            return NULL;
        else if (timeptr->tm_hour == 12)
            timeptr->tm_hour = ampm * 12;
        else
            timeptr->tm_hour += ampm * 12;
    }

    if ((ymd & SET_YMD) == SET_YMD) {
	/* all of tm_year, tm_mon and tm_mday, but... */

	if (!(ymd & SET_YDAY)) {
	    /* ...not tm_yday, so fill it in */
	    timeptr->tm_yday = _DAYS_BEFORE_MONTH[timeptr->tm_mon]
		+ timeptr->tm_mday;
	    if (!is_leap_year (timeptr->tm_year + TM_YEAR_BASE)
		|| timeptr->tm_mon < 2)
	    {
		timeptr->tm_yday--;
	    }
	    ymd |= SET_YDAY;
	}
    }
    else if ((ymd & (SET_YEAR | SET_YDAY)) == (SET_YEAR | SET_YDAY)) {
	/* both of tm_year and tm_yday, but... */

	if (!(ymd & SET_MON)) {
	    /* ...not tm_mon, so fill it in, and/or... */
	    if (timeptr->tm_yday < _DAYS_BEFORE_MONTH[1])
		timeptr->tm_mon = 0;
	    else {
		int leap = is_leap_year (timeptr->tm_year + TM_YEAR_BASE);
		int i;
		for (i = 2; i < 12; ++i) {
		    if (timeptr->tm_yday < _DAYS_BEFORE_MONTH[i] + leap)
			break;
		}
		timeptr->tm_mon = i - 1;
	    }
	}

	if (!(ymd & SET_MDAY)) {
	    /* ...not tm_mday, so fill it in */
	    timeptr->tm_mday = timeptr->tm_yday
		- _DAYS_BEFORE_MONTH[timeptr->tm_mon];
	    if (!is_leap_year (timeptr->tm_year + TM_YEAR_BASE)
		|| timeptr->tm_mon < 2)
	    {
		timeptr->tm_mday++;
	    }
	}
    }

    if ((ymd & (SET_YEAR | SET_YDAY | SET_WDAY)) == (SET_YEAR | SET_YDAY)) {
	/* fill in tm_wday */
	int fday = first_day (timeptr->tm_year + TM_YEAR_BASE);
	timeptr->tm_wday = (fday + timeptr->tm_yday) % 7;
    }

    timeptr->tm_isdst = -1;

    return (char *)buf;
}

char *
strptime (const char *buf, const char *format, struct tm *timeptr)
{
  return strptime_l (buf, format, timeptr, __get_current_locale ());
}
