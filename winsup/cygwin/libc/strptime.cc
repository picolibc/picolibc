/*	$NetBSD: strptime.c,v 1.28 2008/04/28 20:23:01 martin Exp $	*/

/*-
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 * Heavily optimised by David Laight
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __CYGWIN__
#include "winsup.h"
#endif

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: strptime.c,v 1.28 2008/04/28 20:23:01 martin Exp $");
#endif

#ifdef __CYGWIN__
#include "../locale/setlocale.h"
#else
#include "namespace.h"
#include <sys/localedef.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <tzfile.h>

#ifdef __TM_GMTOFF
# define TM_GMTOFF __TM_GMTOFF
#endif
#ifdef __TM_ZONE
# define TM_ZONE __TM_ZONE
#endif

#ifdef __weak_alias
__weak_alias(strptime,_strptime)
#endif

#define	_ctloc(x)		(_CurrentTimeLocale->x)

#define ALT_E			0x01
#define ALT_O			0x02
#define	LEGAL_ALT(x)		{ if (alt_format & ~(x)) return NULL; }

static const int _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define SET_MDAY 1
#define SET_MON  2
#define SET_YEAR 4
#define SET_WDAY 8
#define SET_YDAY 16
#define SET_YMD  (SET_YEAR | SET_MON | SET_MDAY)

static const char gmt[4] = { "GMT" };

typedef struct _era_info_t {
  size_t num;		/* Only in first entry: Number of entries,
			   1 otherwise. */
  int dir;		/* Direction */
  long offset;		/* Number of year closest to start_date in the era. */
  struct tm start;	/* Start date of era */
  struct tm end;	/* End date of era */
  CHAR *era_C;		/* Era string */
  CHAR *era_Y;		/* Replacement for %EY */
} era_info_t;

static void
free_era_info (era_info_t *era_info)
{
  size_t num = era_info->num;

  for (size_t i = 0; i < num; ++i)
    {
      free (era_info[i].era_C);
      free (era_info[i].era_Y);
    }
  free (era_info);
}

static era_info_t *
get_era_info (const char *era, locale_t locale)
{
  char *c;
  era_info_t *ei = NULL;
  size_t num = 0, cur = 0, len;

  while (*era)
    {
      ++num;
      era_info_t *tmp = (era_info_t *) realloc (ei, num * sizeof (era_info_t));
      if (!tmp)
	{
	  ei->num = cur;
	  free_era_info (ei);
	  return NULL;
	}
      ei = tmp;
      ei[cur].num = 1;
      ei[cur].dir = (*era == '+') ? 1 : -1;
      era += 2;
      ei[cur].offset = strtol_l (era, &c, 10, locale);
      era = c + 1;
      ei[cur].start.tm_year = strtol_l (era, &c, 10, locale);
      /* Adjust offset for negative gregorian dates. */
      if (ei[cur].start.tm_year < 0)
	++ei[cur].start.tm_year;
      ei[cur].start.tm_mon = strtol_l (c + 1, &c, 10, locale);
      ei[cur].start.tm_mday = strtol_l (c + 1, &c, 10, locale);
      ei[cur].start.tm_hour = ei[cur].start.tm_min = ei[cur].start.tm_sec = 0;
      era = c + 1;
      if (era[0] == '-' && era[1] == '*')
	{
	  ei[cur].end = ei[cur].start;
	  ei[cur].start.tm_year = INT_MIN;
	  ei[cur].start.tm_mon = ei[cur].start.tm_mday = ei[cur].start.tm_hour
	  = ei[cur].start.tm_min = ei[cur].start.tm_sec = 0;
	  era += 3;
	}
      else if (era[0] == '+' && era[1] == '*')
	{
	  ei[cur].end.tm_year = INT_MAX;
	  ei[cur].end.tm_mon = 12;
	  ei[cur].end.tm_mday = 31;
	  ei[cur].end.tm_hour = 23;
	  ei[cur].end.tm_min = ei[cur].end.tm_sec = 59;
	  era += 3;
	}
      else
	{
	  ei[cur].end.tm_year = strtol_l (era, &c, 10, locale);
	  /* Adjust offset for negative gregorian dates. */
	  if (ei[cur].end.tm_year < 0)
	    ++ei[cur].end.tm_year;
	  ei[cur].end.tm_mon = strtol_l (c + 1, &c, 10, locale);
	  ei[cur].end.tm_mday = strtol_l (c + 1, &c, 10, locale);
	  ei[cur].end.tm_mday = 31;
	  ei[cur].end.tm_hour = 23;
	  ei[cur].end.tm_min = ei[cur].end.tm_sec = 59;
	  era = c + 1;
	}
      /* era_C */
      c = strchr (era, ':');
      len = c - era;
      ei[cur].era_C = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
      if (!ei[cur].era_C)
	{
	  ei->num = cur;
	  free_era_info (ei);
	  return NULL;
	}
      strncpy (ei[cur].era_C, era, len);
      era += len;
      ei[cur].era_C[len] = '\0';
      /* era_Y */
      ++era;
      c = strchr (era, ';');
      if (!c)
	c = strchr (era, '\0');
      len = c - era;
      ei[cur].era_Y = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
      if (!ei[cur].era_Y)
	{
	  free (ei[cur].era_C);
	  ei->num = cur;
	  free_era_info (ei);
	  return NULL;
	}
      strncpy (ei[cur].era_Y, era, len);
      era += len;
      ei[cur].era_Y[len] = '\0';
      ++cur;
      if (*c)
	era = c + 1;
    }
  ei->num = num;
  return ei;
}

typedef struct _alt_digits_t {
  size_t num;
  char **digit;
  char *buffer;
} alt_digits_t;

static alt_digits_t *
get_alt_digits (const char *alt_digits)
{
  alt_digits_t *adi;
  const char *a, *e;
  char *aa, *ae;
  size_t len;

  adi = (alt_digits_t *) calloc (1, sizeof (alt_digits_t));
  if (!adi)
    return NULL;

  /* Compute number of alt_digits. */
  adi->num = 1;
  for (a = alt_digits; (e = strchr (a, ';')) != NULL; a = e + 1)
      ++adi->num;
  /* Allocate the `digit' array, which is an array of `num' pointers into
     `buffer'. */
  adi->digit = (CHAR **) calloc (adi->num, sizeof (CHAR **));
  if (!adi->digit)
    {
      free (adi);
      return NULL;
    }
  /* Compute memory required for `buffer'. */
  len = strlen (alt_digits);
  /* Allocate it. */
  adi->buffer = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
  if (!adi->buffer)
    {
      free (adi->digit);
      free (adi);
      return NULL;
    }
  /* Store digits in it. */
  strcpy (adi->buffer, alt_digits);
  /* Store the pointers into `buffer' into the appropriate `digit' slot. */
  for (len = 0, aa = adi->buffer; (ae = strchr (aa, ';')) != NULL;
       ++len, aa = ae + 1)
    {
      *ae = '\0';
      adi->digit[len] = aa;
    }
  adi->digit[len] = aa;
  return adi;
}

static void
free_alt_digits (alt_digits_t *adi)
{
  free (adi->digit);
  free (adi->buffer);
  free (adi);
}

static const unsigned char *
find_alt_digits (const unsigned char *bp, alt_digits_t *adi, uint *pval)
{
  /* This is rather error-prone, but the entire idea of alt_digits
     isn't thought out well.  If you start to look for matches at the
     start, there's a high probability that you find short matches but
     the entire translation is wrong.  So we scan the alt_digits array
     from the highest to the lowest digits instead, hoping that it's
     more likely to catch digits consisting of multiple characters. */
  for (int i = (int) adi->num - 1; i >= 0; --i)
    {
      size_t len = strlen (adi->digit[i]);
      if (!strncmp ((const char *) bp, adi->digit[i], len))
	{
	  *pval = i;
	  return bp + len;
	}
    }
  return NULL;
}

static int
is_leap_year (int year)
{
  return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

static int
first_day (int year)
{
  int ret = 4;

  while (--year >= 1970)
    ret = (ret + 365 + is_leap_year (year)) % 7;
  return ret;
}

/* This simplifies the calls to conv_num enormously. */
#define ALT_DIGITS	((alt_format & ALT_O) ? *alt_digits : NULL)

static const unsigned char *conv_num(const unsigned char *, int *, uint, uint,
				     alt_digits_t *);
static const unsigned char *find_string(const unsigned char *, int *,
					const char * const *,
					const char * const *, int,
					locale_t);

static char *
__strptime(const char *buf, const char *fmt, struct tm *tm,
	   era_info_t **era_info, alt_digits_t **alt_digits,
	   locale_t locale)
{
	unsigned char c;
	const unsigned char *bp;
	int alt_format, i, split_year = 0;
	era_info_t *era = NULL;
	int era_offset, got_eoff = 0;
	int saw_padding;
	unsigned long width;
	const char *new_fmt;
	uint ulim;
	int ymd = 0;

	bp = (const unsigned char *)buf;
	const struct lc_time_T *_CurrentTimeLocale = __get_time_locale (locale);

	while (bp != NULL && (c = *fmt++) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		saw_padding = 0;
		width = 0;
		alt_format = 0;
		i = 0;

		/* Eat up white-space. */
		if (isspace_l(c, locale)) {
			while (isspace_l(*bp, locale))
				bp++;
			continue;
		}

		if (c != '%')
			goto literal;


again:		switch (c = *fmt++) {
		case '%':	/* "%%" is converted to "%". */
literal:
			if (c != *bp++)
				return NULL;
			LEGAL_ALT(0);
			continue;

		/*
		 * "Alternative" modifiers. Just set the appropriate flag
		 * and start over again.
		 */
		case 'E':	/* "%E?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_E;
			if (!*era_info && *_CurrentTimeLocale->era)
			  *era_info = get_era_info (_CurrentTimeLocale->era,
						    locale);
			goto again;

		case 'O':	/* "%O?" alternative conversion modifier. */
			LEGAL_ALT(0);
			alt_format |= ALT_O;
			if (!*alt_digits && *_CurrentTimeLocale->alt_digits)
			  *alt_digits =
			      get_alt_digits (_CurrentTimeLocale->alt_digits);
			goto again;
		case '0':
		case '+':
			LEGAL_ALT(0);
			if (saw_padding)
			  return NULL;
			saw_padding = 1;
			goto again;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			/* POSIX-1.2008 maximum field width.  Per POSIX,
			   the width is only defined for the 'C', 'F', and 'Y'
			   conversion specifiers. */
			LEGAL_ALT(0);
			{
			  char *end;
			  width = strtoul_l (fmt - 1, &end, 10, locale);
			  fmt = (const char *) end;
			  goto again;
			}
		/*
		 * "Complex" conversion rules, implemented through recursion.
		 */
		case 'c':	/* Date and time, using the locale's format. */
			new_fmt = (alt_format & ALT_E)
				  ? _ctloc (era_d_t_fmt) : _ctloc(c_fmt);
			LEGAL_ALT(ALT_E);
			ymd |= SET_WDAY | SET_YMD;
			goto recurse;

		case 'D':	/* The date as "%m/%d/%y". */
			new_fmt = "%m/%d/%y";
			LEGAL_ALT(0);
			ymd |= SET_YMD;
			goto recurse;

		case 'F':	/* The date as "%Y-%m-%d". */
			{
			  LEGAL_ALT(0);
			  char *tmp = __strptime ((const char *) bp, "%Y-%m-%d",
						  tm, era_info, alt_digits,
						  locale);
			  /* width max chars converted, default 10, < 6 => 6 */
			  if (tmp && (char *) bp +
				(!width ? 10 : width < 6 ? 6 : width) < tmp)
			    return NULL;
			  bp = (const unsigned char *) tmp;
			  ymd |= SET_YMD;
			  continue;
			}

		case 'R':	/* The time as "%H:%M". */
			new_fmt = "%H:%M";
			LEGAL_ALT(0);
			goto recurse;

		case 'r':	/* The time in 12-hour clock representation. */
			new_fmt =_ctloc(ampm_fmt);
			LEGAL_ALT(0);
			goto recurse;

		case 'T':	/* The time as "%H:%M:%S". */
			new_fmt = "%H:%M:%S";
			LEGAL_ALT(0);
			goto recurse;

		case 'X':	/* The time, using the locale's format. */
			new_fmt = (alt_format & ALT_E)
				  ? _ctloc (era_t_fmt) : _ctloc(X_fmt);
			LEGAL_ALT(ALT_E);
			goto recurse;

		case 'x':	/* The date, using the locale's format. */
			new_fmt = (alt_format & ALT_E)
				  ? _ctloc (era_d_fmt) : _ctloc(x_fmt);
			LEGAL_ALT(ALT_E);
			ymd |= SET_YMD;
		    recurse:
			bp = (const unsigned char *)
			     __strptime((const char *)bp, new_fmt, tm,
					era_info, alt_digits, locale);
			continue;

		/*
		 * "Elementary" conversion rules.
		 */
		case 'A':	/* The day of week, using the locale's form. */
		case 'a':
			bp = find_string(bp, &tm->tm_wday, _ctloc(weekday),
					_ctloc(wday), 7, locale);
			LEGAL_ALT(0);
			ymd |= SET_WDAY;
			continue;

		case 'B':	/* The month, using the locale's form. */
		case 'b':
		case 'h':
			bp = find_string(bp, &tm->tm_mon, _ctloc(month),
					_ctloc(mon), 12, locale);
			LEGAL_ALT(0);
			ymd |= SET_WDAY;
			continue;

		case 'C':	/* The century number. */
			LEGAL_ALT(ALT_E);
			ymd |= SET_YEAR;
			if ((alt_format & ALT_E) && *era_info)
			  {
			    /* With E modifier, an era.  We potentially
			       don't know the era offset yet, so we have to
			       store the value in a local variable.
			       The final computation of tm_year is only done
			       right before this function returns. */
			    size_t num = (*era_info)->num;
			    for (size_t i = 0; i < num; ++i)
			      if (!strncmp ((const char *) bp,
					    (*era_info)[i].era_C,
					    strlen ((*era_info)[i].era_C)))
				{
				  era = (*era_info) + i;
				  bp += strlen (era->era_C);
				  break;
				}
			    if (!era)
			      return NULL;
			    continue;
			  }
			i = 20;
			for (ulim = 99; width && width < 2; ++width)
			  ulim /= 10;
			bp = conv_num(bp, &i, 0, ulim, NULL);

			i = i * 100 - TM_YEAR_BASE;
			if (split_year)
				i += tm->tm_year % 100;
			split_year = 1;
			tm->tm_year = i;
			era = NULL;
			got_eoff = 0;
			continue;

		case 'd':	/* The day of month. */
		case 'e':
			LEGAL_ALT(ALT_O);
			ymd |= SET_MDAY;
			bp = conv_num(bp, &tm->tm_mday, 1, 31, ALT_DIGITS);
			continue;

		case 'k':	/* The hour (24-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'H':
			LEGAL_ALT(ALT_O);
			bp = conv_num(bp, &tm->tm_hour, 0, 23, ALT_DIGITS);
			continue;

		case 'l':	/* The hour (12-hour clock representation). */
			LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'I':
			LEGAL_ALT(ALT_O);
			bp = conv_num(bp, &tm->tm_hour, 1, 12, ALT_DIGITS);
			if (tm->tm_hour == 12)
				tm->tm_hour = 0;
			continue;

		case 'j':	/* The day of year. */
			i = 1;
			bp = conv_num(bp, &i, 1, 366, NULL);
			tm->tm_yday = i - 1;
			LEGAL_ALT(0);
			ymd |= SET_YDAY;
			continue;

		case 'M':	/* The minute. */
			LEGAL_ALT(ALT_O);
			bp = conv_num(bp, &tm->tm_min, 0, 59, ALT_DIGITS);
			continue;

		case 'm':	/* The month. */
			LEGAL_ALT(ALT_O);
			ymd |= SET_MON;
			i = 1;
			bp = conv_num(bp, &i, 1, 12, ALT_DIGITS);
			tm->tm_mon = i - 1;
			continue;

		case 'p':	/* The locale's equivalent of AM/PM. */
			bp = find_string(bp, &i, _ctloc(am_pm), NULL, 2,
					 locale);
			if (tm->tm_hour > 11)
				return NULL;
			tm->tm_hour += i * 12;
			LEGAL_ALT(0);
			continue;

		case 'S':	/* The seconds. */
			LEGAL_ALT(ALT_O);
			bp = conv_num(bp, &tm->tm_sec, 0, 61, ALT_DIGITS);
			continue;

		case 's' :	/* The seconds since Unix epoch - GNU extension */
		    {
			long long sec;
			time_t t;
			char *end;
			save_errno save;

			LEGAL_ALT(0);
			sec = strtoll_l ((char *)bp, &end, 10, locale);
			t = sec;
			if (end == (char *)bp
			    || errno != 0
			    || t != sec
			    || localtime_r (&t, tm) != tm)
			    return NULL;
			bp = (const unsigned char *)end;
			ymd |= SET_YDAY | SET_WDAY | SET_YMD;
			break;
		    }

		case 'U':	/* The week of year, beginning on sunday. */
		case 'W':	/* The week of year, beginning on monday. */
			/*
			 * XXX This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so just check the
			 * range for now.
			 */
			 LEGAL_ALT(ALT_O);
			 bp = conv_num(bp, &i, 0, 53, ALT_DIGITS);
			 continue;

		case 'u':	/* The day of week, beginning on monday. */
			LEGAL_ALT(ALT_O);
			ymd |= SET_WDAY;
			bp = conv_num(bp, &i, 1, 7, ALT_DIGITS);
			tm->tm_wday = i % 7;
			continue;
		case 'w':	/* The day of week, beginning on sunday. */
			LEGAL_ALT(ALT_O);
			ymd |= SET_WDAY;
			bp = conv_num(bp, &tm->tm_wday, 0, 6, ALT_DIGITS);
			continue;

		case 'Y':	/* The year. */
			LEGAL_ALT(ALT_E);
			ymd |= SET_YEAR;
			if ((alt_format & ALT_E) && *era_info)
			  {
			    bool gotit = false;
			    size_t num = (*era_info)->num;
			    (*era_info)->num = 1;
			    for (size_t i = 0; i < num; ++i)
			      {
				era_info_t *tmp_ei = (*era_info) + i;
				char *tmp = __strptime ((const char *) bp,
							tmp_ei->era_Y,
							tm, &tmp_ei,
							alt_digits, locale);
				if (tmp)
				  {
				    bp = (const unsigned char *) tmp;
				    gotit = true;
				    break;
				  }
			      }
			    (*era_info)->num = num;
			    if (gotit)
			      continue;
			    return NULL;
			  }
			i = TM_YEAR_BASE;	/* just for data sanity... */
			for (ulim = 9999; width && width < 4; ++width)
			  ulim /= 10;
			bp = conv_num(bp, &i, 0, ulim, NULL);
			tm->tm_year = i - TM_YEAR_BASE;
			era = NULL;
			got_eoff = 0;
			continue;

		case 'y':	/* The year within 100 years of the epoch. */
			/* LEGAL_ALT(ALT_E | ALT_O); */
			ymd |= SET_YEAR;
			if ((alt_format & ALT_E) && *era_info)
			  {
			    /* With E modifier, the offset to the start date
			       of the era specified with %EC.  We potentially
			       don't know the era yet, so we have to store the
			       value in a local variable, just like era itself.
			       The final computation of tm_year is only done
			       right before this function returns. */
			    bp = conv_num(bp, &era_offset, 0, UINT_MAX, NULL);
			    got_eoff = 1;
			    continue;
			  }
			bp = conv_num(bp, &i, 0, 99, ALT_DIGITS);

			if (split_year) /* preserve century */
				i += (tm->tm_year / 100) * 100;
			else {
				split_year = 1;
				if (i <= 68)
					i = i + 2000 - TM_YEAR_BASE;
				else
					i = i + 1900 - TM_YEAR_BASE;
			}
			tm->tm_year = i;
			era = NULL;
			got_eoff = 0;
			continue;

		case 'Z':
			tzset();
			if (strncmp((const char *)bp, gmt, 3) == 0) {
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
				  tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
				  tm->TM_ZONE = gmt;
#endif
				bp += 3;
			} else {
				const unsigned char *ep;

				ep = find_string(bp, &i,
					       	 (const char * const *)tzname,
					       	  NULL, 2, locale);
				if (ep != NULL) {
					tm->tm_isdst = i;
#ifdef TM_GMTOFF
					if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
					  tm->TM_GMTOFF = -(timezone);
#endif
#ifdef TM_ZONE
					if (CYGWIN_VERSION_CHECK_FOR_EXTRA_TM_MEMBERS)
					  tm->TM_ZONE = tzname[i];
#endif
				}
				bp = ep;
			}
			continue;

		/*
		 * Miscellaneous conversions.
		 */
		case 'n':	/* Any kind of white-space. */
		case 't':
			while (isspace_l(*bp, locale))
				bp++;
			LEGAL_ALT(0);
			continue;


		default:	/* Unknown/unsupported conversion. */
			return NULL;
		}
	}

	if (bp && (era || got_eoff))
	  {
	    /* Default to current era. */
	    if (!era)
	      era = *era_info;
	    /* Default to first year of era if offset is missing */
	    if (!got_eoff)
	      era_offset = era->offset;
	    tm->tm_year = (era->start.tm_year != INT_MIN
			   ? era->start.tm_year : era->end.tm_year)
			   + (era_offset - era->offset) * era->dir;
	    /* Check if year falls into the era.  If not, it's an
	       invalid combination of era and offset. */
	    if (era->start.tm_year > tm->tm_year
	    	|| era->end.tm_year < tm->tm_year)
	      return NULL;
	    tm->tm_year -= TM_YEAR_BASE;
	  }

	if ((ymd & SET_YMD) == SET_YMD)
	  {
	    /* all of tm_year, tm_mon and tm_mday, but... */
	    if (!(ymd & SET_YDAY))
	      {
		/* ...not tm_yday, so fill it in */
		tm->tm_yday = _DAYS_BEFORE_MONTH[tm->tm_mon] + tm->tm_mday;
		if (!is_leap_year (tm->tm_year + TM_YEAR_BASE)
		    || tm->tm_mon < 2)
		  tm->tm_yday--;
		ymd |= SET_YDAY;
	      }
	  }
	else if ((ymd & (SET_YEAR | SET_YDAY)) == (SET_YEAR | SET_YDAY))
	  {
	    /* both of tm_year and tm_yday, but... */
	    if (!(ymd & SET_MON))
	      {
		/* ...not tm_mon, so fill it in, and/or... */
		if (tm->tm_yday < _DAYS_BEFORE_MONTH[1])
		  tm->tm_mon = 0;
		else
		  {
		    int leap = is_leap_year (tm->tm_year + TM_YEAR_BASE);
		    for (i = 2; i < 12; ++i)
		      if (tm->tm_yday < _DAYS_BEFORE_MONTH[i] + leap)
			break;
		    tm->tm_mon = i - 1;
		  }
	      }
	    if (!(ymd & SET_MDAY))
	      {
		/* ...not tm_mday, so fill it in */
		tm->tm_mday = tm->tm_yday - _DAYS_BEFORE_MONTH[tm->tm_mon];
		if (!is_leap_year (tm->tm_year + TM_YEAR_BASE)
		    || tm->tm_mon < 2)
		  tm->tm_mday++;
	      }
	  }

	if ((ymd & (SET_YEAR | SET_YDAY | SET_WDAY)) == (SET_YEAR | SET_YDAY))
	  {
	    /* fill in tm_wday */
	    int fday = first_day (tm->tm_year + TM_YEAR_BASE);
	    tm->tm_wday = (fday + tm->tm_yday) % 7;
	  }
	return (char *) bp;
}

char *
strptime_l (const char *__restrict buf, const char *__restrict fmt,
	    struct tm *__restrict tm, locale_t locale)
{
  era_info_t *era_info = NULL;
  alt_digits_t *alt_digits = NULL;
  char *ret = __strptime (buf, fmt, tm, &era_info, &alt_digits, locale);
  if (era_info)
    free_era_info (era_info);
  if (alt_digits)
    free_alt_digits (alt_digits);
  return ret;
}

char *
strptime (const char *__restrict buf, const char *__restrict fmt,
	  struct tm *__restrict tm)
{
  era_info_t *era_info = NULL;
  alt_digits_t *alt_digits = NULL;
  char *ret = __strptime (buf, fmt, tm, &era_info, &alt_digits,
			  __get_current_locale ());
  if (era_info)
    free_era_info (era_info);
  if (alt_digits)
    free_alt_digits (alt_digits);
  return ret;
}

static const unsigned char *
conv_num(const unsigned char *buf, int *dest, uint llim, uint ulim,
	 alt_digits_t *alt_digits)
{
	uint result = 0;
	unsigned char ch;

	if (alt_digits)
	  buf = find_alt_digits (buf, alt_digits, &result);
	else
	  {
	    /* The limit also determines the number of valid digits. */
	    uint rulim = ulim;

	    ch = *buf;
	    if (ch < '0' || ch > '9')
		    return NULL;

	    do {
		    result *= 10;
		    result += ch - '0';
		    rulim /= 10;
		    ch = *++buf;
	    } while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');
	  }

	if (result < llim || result > ulim)
		return NULL;

	*dest = result;
	return buf;
}

static const unsigned char *
find_string(const unsigned char *bp, int *tgt, const char * const *n1,
	    const char * const *n2, int c, locale_t locale)
{
	int i;
	unsigned int len;

	/* check full name - then abbreviated ones */
	for (; n1 != NULL; n1 = n2, n2 = NULL) {
		for (i = 0; i < c; i++, n1++) {
			len = strlen(*n1);
			if (strncasecmp_l(*n1, (const char *)bp, len,
					  locale) == 0) {
				*tgt = i;
				return bp + len;
			}
		}
	}

	/* Nothing matched */
	return NULL;
}
