/* NOTE:  This file defines both strftime() and wcsftime().  Take care when
 * making changes.  See also wcsftime.c, and note the (small) overlap in the
 * manual description, taking care to edit both as needed.  */
/*
 * strftime.c
 * Original Author:	G. Haley
 * Additions from:	Eric Blake, Corinna Vinschen
 * Changes to allow dual use as wcstime, also:	Craig Howland
 *
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate.
 */

/*
FUNCTION
<<strftime>>, <<strftime_l>>---convert date and time to a formatted string

INDEX
	strftime

INDEX
	strftime_l

SYNOPSIS
	#include <time.h>
	size_t strftime(char *restrict <[s]>, size_t <[maxsize]>,
			const char *restrict <[format]>,
                        const struct tm *restrict <[timp]>);
	size_t strftime_l(char *restrict <[s]>, size_t <[maxsize]>,
			  const char *restrict <[format]>,
			  const struct tm *restrict <[timp]>,
			  locale_t <[locale]>);

DESCRIPTION
<<strftime>> converts a <<struct tm>> representation of the time (at
<[timp]>) into a null-terminated string, starting at <[s]> and occupying
no more than <[maxsize]> characters.

<<strftime_l>> is like <<strftime>> but creates a string in a format
as expected in locale <[locale]>.  If <[locale]> is LC_GLOBAL_LOCALE or
not a valid locale object, the behaviour is undefined.

You control the format of the output using the string at <[format]>.
<<*<[format]>>> can contain two kinds of specifications: text to be
copied literally into the formatted string, and time conversion
specifications.  Time conversion specifications are two- and
three-character sequences beginning with `<<%>>' (use `<<%%>>' to
include a percent sign in the output).  Each defined conversion
specification selects only the specified field(s) of calendar time
data from <<*<[timp]>>>, and converts it to a string in one of the
following ways:

o+
o %a
The abbreviated weekday name according to the current locale. [tm_wday]

o %A
The full weekday name according to the current locale.
In the default "C" locale, one of `<<Sunday>>', `<<Monday>>', `<<Tuesday>>',
`<<Wednesday>>', `<<Thursday>>', `<<Friday>>', `<<Saturday>>'. [tm_wday]

o %b
The abbreviated month name according to the current locale. [tm_mon]

o %B
The full month name according to the current locale.
In the default "C" locale, one of `<<January>>', `<<February>>',
`<<March>>', `<<April>>', `<<May>>', `<<June>>', `<<July>>',
`<<August>>', `<<September>>', `<<October>>', `<<November>>',
`<<December>>'. [tm_mon]

o %c
The preferred date and time representation for the current locale.
[tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday]

o %C
The century, that is, the year divided by 100 then truncated.  For
4-digit years, the result is zero-padded and exactly two characters;
but for other years, there may a negative sign or more digits.  In
this way, `<<%C%y>>' is equivalent to `<<%Y>>'. [tm_year]

o %d
The day of the month, formatted with two digits (from `<<01>>' to
`<<31>>'). [tm_mday]

o %D
A string representing the date, in the form `<<"%m/%d/%y">>'.
[tm_mday, tm_mon, tm_year]

o %e
The day of the month, formatted with leading space if single digit
(from `<<1>>' to `<<31>>'). [tm_mday]

o %E<<x>>
In some locales, the E modifier selects alternative representations of
certain modifiers <<x>>.  In newlib, it is ignored, and treated as %<<x>>.

o %F
A string representing the ISO 8601:2000 date format, in the form
`<<"%Y-%m-%d">>'. [tm_mday, tm_mon, tm_year]

o %g
The last two digits of the week-based year, see specifier %G (from
`<<00>>' to `<<99>>'). [tm_year, tm_wday, tm_yday]

o %G
The week-based year. In the ISO 8601:2000 calendar, week 1 of the year
includes January 4th, and begin on Mondays. Therefore, if January 1st,
2nd, or 3rd falls on a Sunday, that day and earlier belong to the last
week of the previous year; and if December 29th, 30th, or 31st falls
on Monday, that day and later belong to week 1 of the next year.  For
consistency with %Y, it always has at least four characters.
Example: "%G" for Saturday 2nd January 1999 gives "1998", and for
Tuesday 30th December 1997 gives "1998". [tm_year, tm_wday, tm_yday]

o %h
Synonym for "%b". [tm_mon]

o %H
The hour (on a 24-hour clock), formatted with two digits (from
`<<00>>' to `<<23>>'). [tm_hour]

o %I
The hour (on a 12-hour clock), formatted with two digits (from
`<<01>>' to `<<12>>'). [tm_hour]

o %j
The count of days in the year, formatted with three digits
(from `<<001>>' to `<<366>>'). [tm_yday]

o %k
The hour (on a 24-hour clock), formatted with leading space if single
digit (from `<<0>>' to `<<23>>'). Non-POSIX extension (c.p. %I). [tm_hour]

o %l
The hour (on a 12-hour clock), formatted with leading space if single
digit (from `<<1>>' to `<<12>>'). Non-POSIX extension (c.p. %H). [tm_hour]

o %m
The month number, formatted with two digits (from `<<01>>' to `<<12>>').
[tm_mon]

o %M
The minute, formatted with two digits (from `<<00>>' to `<<59>>'). [tm_min]

o %n
A newline character (`<<\n>>').

o %O<<x>>
In some locales, the O modifier selects alternative digit characters
for certain modifiers <<x>>.  In newlib, it is ignored, and treated as %<<x>>.

o %p
Either `<<AM>>' or `<<PM>>' as appropriate, or the corresponding strings for
the current locale. [tm_hour]

o %P
Same as '<<%p>>', but in lowercase.  This is a GNU extension. [tm_hour]

o %q
Quarter of the year (from `<<1>>' to `<<4>>'), with January starting
the first quarter. This is a GNU extension. [tm_mon]

o %r
Replaced by the time in a.m. and p.m. notation.  In the "C" locale this
is equivalent to "%I:%M:%S %p".  In locales which don't define a.m./p.m.
notations, the result is an empty string. [tm_sec, tm_min, tm_hour]

o %R
The 24-hour time, to the minute.  Equivalent to "%H:%M". [tm_min, tm_hour]

o %s
The time elapsed, in seconds, since the start of the Unix epoch at
1970-01-01 00:00:00 UTC.

o %S
The second, formatted with two digits (from `<<00>>' to `<<60>>').  The
value 60 accounts for the occasional leap second. [tm_sec]

o %t
A tab character (`<<\t>>').

o %T
The 24-hour time, to the second.  Equivalent to "%H:%M:%S". [tm_sec,
tm_min, tm_hour]

o %u
The weekday as a number, 1-based from Monday (from `<<1>>' to
`<<7>>'). [tm_wday]

o %U
The week number, where weeks start on Sunday, week 1 contains the first
Sunday in a year, and earlier days are in week 0.  Formatted with two
digits (from `<<00>>' to `<<53>>').  See also <<%W>>. [tm_wday, tm_yday]

o %V
The week number, where weeks start on Monday, week 1 contains January 4th,
and earlier days are in the previous year.  Formatted with two digits
(from `<<01>>' to `<<53>>').  See also <<%G>>. [tm_year, tm_wday, tm_yday]

o %v
A string representing the BSD/OSX/Ruby VMS/Oracle date format, in the form
"%e-%b-%Y". Non-POSIX extension. [tm_mday, tm_mon, tm_year]

o %w
The weekday as a number, 0-based from Sunday (from `<<0>>' to `<<6>>').
[tm_wday]

o %W
The week number, where weeks start on Monday, week 1 contains the first
Monday in a year, and earlier days are in week 0.  Formatted with two
digits (from `<<00>>' to `<<53>>'). [tm_wday, tm_yday]

o %x
Replaced by the preferred date representation in the current locale.
In the "C" locale this is equivalent to "%m/%d/%y".
[tm_mon, tm_mday, tm_year]

o %X
Replaced by the preferred time representation in the current locale.
In the "C" locale this is equivalent to "%H:%M:%S". [tm_sec, tm_min, tm_hour]

o %y
The last two digits of the year (from `<<00>>' to `<<99>>'). [tm_year]
(Implementation interpretation:  always positive, even for negative years.)

o %Y
The full year, equivalent to <<%C%y>>.  It will always have at least four
characters, but may have more.  The year is accurate even when tm_year
added to the offset of 1900 overflows an int. [tm_year]

o %z
The offset from UTC.  The format consists of a sign (negative is west of
Greewich), two characters for hour, then two characters for minutes
(-hhmm or +hhmm).  If tm_isdst is negative, the offset is unknown and no
output is generated; if it is zero, the offset is the standard offset for
the current time zone; and if it is positive, the offset is the daylight
savings offset for the current timezone. The offset is determined from
the TZ environment variable, as if by calling tzset(). [tm_isdst]

o %Z
The current time zone abbreviation. If tm_isdst is negative, no output
is generated. Otherwise, the time zone abbreviation is based on the TZ
environment variable, as if by calling tzset(). [tm_isdst]

o %%
A single character, `<<%>>'.
o-

RETURNS
When the formatted time takes up no more than <[maxsize]> characters,
the result is the length of the formatted string.  Otherwise, if the
formatting operation was abandoned due to lack of room, the result is
<<0>>, and the string starting at <[s]> corresponds to just those
parts of <<*<[format]>>> that could be completely filled in within the
<[maxsize]> limit.

PORTABILITY
ANSI C requires <<strftime>>, but does not specify the contents of
<<*<[s]>>> when the formatted string would require more than
<[maxsize]> characters.  Unrecognized specifiers and fields of
<<timp>> that are out of range cause undefined results.  Since some
formats expand to 0 bytes, it is wise to set <<*<[s]>>> to a nonzero
value beforehand to distinguish between failure and an empty string.
This implementation does not support <<s>> being NULL, nor overlapping
<<s>> and <<format>>.

<<strftime_l>> is POSIX-1.2008.

<<strftime>> and <<strftime_l>> require no supporting OS subroutines.

BUGS
(NOT Cygwin:) <<strftime>> ignores the LC_TIME category of the current
locale, hard-coding the "C" locale settings.
*/

#include <newlib.h>
#include <sys/config.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "../locale/setlocale.h"

/* Defines to make the file dual use for either strftime() or wcsftime().
 * To get wcsftime, define MAKE_WCSFTIME.
 * To get strftime, do not define MAKE_WCSFTIME.
 * Names are kept friendly to strftime() usage.  The biggest ugliness is the
 * use of the CQ() macro to make either regular character constants and
 * string literals or wide-character constants and wide-character-string
 * literals, as appropriate.  */
#if !defined(MAKE_WCSFTIME)
#  define CHAR		char		/* string type basis */
#  define CQ(a)		a		/* character constant qualifier */
#  define SFLG				/* %s flag (null for normal char) */
#  define _ctloc(x)     (ctloclen = strlen (ctloc = _CurrentTimeLocale->x))
#  define snprintf	sniprintf	/* avoid to pull in FP functions. */
#  define TOLOWER(c)	tolower((int)(unsigned char)(c))
#  define STRTOUL(c,p,b) strtoul((c),(p),(b))
#  define STRCPY(a,b)	strcpy((a),(b))
#  define STRCHR(a,b)	strchr((a),(b))
#  define STRLEN(a)	strlen(a)
# else
#  define strftime	wcsftime	/* Alternate function name */
#  define strftime_l	wcsftime_l	/* Alternate function name */
#  define CHAR		wchar_t		/* string type basis */
#  define CQ(a)		L##a		/* character constant qualifier */
#  define snprintf	swprintf	/* wide-char equivalent function name */
#  define strncmp	wcsncmp		/* wide-char equivalent function name */
#  define TOLOWER(c)	towlower((wint_t)(c))
#  define STRTOUL(c,p,b) wcstoul((c),(p),(b))
#  define STRCPY(a,b)	wcscpy((a),(b))
#  define STRCHR(a,b)	wcschr((a),(b))
#  define STRLEN(a)	wcslen(a)
#  define SFLG		"l"		/* %s flag (l for wide char) */
#  ifdef __HAVE_LOCALE_INFO_EXTENDED__
#   define _ctloc(x)    (ctloclen = wcslen (ctloc = _CurrentTimeLocale->w##x))
#  else
#   define CTLOCBUFLEN   256		/* Arbitrary big buffer size */
    const wchar_t *
    __ctloc (wchar_t *buf, const char *elem, size_t *len_ret)
    {
      buf[CTLOCBUFLEN - 1] = L'\0';
      *len_ret = mbstowcs (buf, elem, CTLOCBUFLEN - 1);
      if (*len_ret == (size_t) -1 )
	*len_ret = 0;
      return buf;
    }
#   define _ctloc(x)    (ctloc = __ctloc (ctlocbuf, _CurrentTimeLocale->x, \
					  &ctloclen))
#  endif
#endif  /* MAKE_WCSFTIME */

#define CHECK_LENGTH()	if (len < 0 || (count += len) >= maxsize) \
			  return 0

/* Enforce the coding assumptions that YEAR_BASE is positive.  (%C, %Y, etc.) */
#if YEAR_BASE < 0
#  error "YEAR_BASE < 0"
#endif

/* Using the tm_year, tm_wday, and tm_yday components of TIM_P, return
   -1, 0, or 1 as the adjustment to add to the year for the ISO week
   numbering used in "%g%G%V", avoiding overflow.  */
static int
iso_year_adjust (const struct tm *tim_p)
{
  /* Account for fact that tm_year==0 is year 1900.  */
  int leap = isleap (tim_p->tm_year + (YEAR_BASE
				       - (tim_p->tm_year < 0 ? 0 : 2000)));

  /* Pack the yday, wday, and leap year into a single int since there are so
     many disparate cases.  */
#define PACK(yd, wd, lp) (((yd) << 4) + (wd << 1) + (lp))
  switch (PACK (tim_p->tm_yday, tim_p->tm_wday, leap))
    {
    case PACK (0, 5, 0): /* Jan 1 is Fri, not leap.  */
    case PACK (0, 6, 0): /* Jan 1 is Sat, not leap.  */
    case PACK (0, 0, 0): /* Jan 1 is Sun, not leap.  */
    case PACK (0, 5, 1): /* Jan 1 is Fri, leap year.  */
    case PACK (0, 6, 1): /* Jan 1 is Sat, leap year.  */
    case PACK (0, 0, 1): /* Jan 1 is Sun, leap year.  */
    case PACK (1, 6, 0): /* Jan 2 is Sat, not leap.  */
    case PACK (1, 0, 0): /* Jan 2 is Sun, not leap.  */
    case PACK (1, 6, 1): /* Jan 2 is Sat, leap year.  */
    case PACK (1, 0, 1): /* Jan 2 is Sun, leap year.  */
    case PACK (2, 0, 0): /* Jan 3 is Sun, not leap.  */
    case PACK (2, 0, 1): /* Jan 3 is Sun, leap year.  */
      return -1; /* Belongs to last week of previous year.  */
    case PACK (362, 1, 0): /* Dec 29 is Mon, not leap.  */
    case PACK (363, 1, 1): /* Dec 29 is Mon, leap year.  */
    case PACK (363, 1, 0): /* Dec 30 is Mon, not leap.  */
    case PACK (363, 2, 0): /* Dec 30 is Tue, not leap.  */
    case PACK (364, 1, 1): /* Dec 30 is Mon, leap year.  */
    case PACK (364, 2, 1): /* Dec 30 is Tue, leap year.  */
    case PACK (364, 1, 0): /* Dec 31 is Mon, not leap.  */
    case PACK (364, 2, 0): /* Dec 31 is Tue, not leap.  */
    case PACK (364, 3, 0): /* Dec 31 is Wed, not leap.  */
    case PACK (365, 1, 1): /* Dec 31 is Mon, leap year.  */
    case PACK (365, 2, 1): /* Dec 31 is Tue, leap year.  */
    case PACK (365, 3, 1): /* Dec 31 is Wed, leap year.  */
      return 1; /* Belongs to first week of next year.  */
    }
  return 0; /* Belongs to specified year.  */
#undef PACK
}

#ifdef _WANT_C99_TIME_FORMATS
typedef struct {
  int   year;
  CHAR *era_C;
  CHAR *era_Y;
} era_info_t;

static era_info_t *
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
get_era_info (const struct tm *tim_p, const wchar_t *era)
#else
get_era_info (const struct tm *tim_p, const char *era)
#endif
{
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
  wchar_t *c;
  const wchar_t *dir;
# define ERA_STRCHR(a,b)	wcschr((a),(b))
# define ERA_STRNCPY(a,b,c)	wcsncpy((a),(b),(c))
# define ERA_STRTOL(a,b,c)	wcstol((a),(b),(c))
#else
  char *c;
  const char *dir;
# define ERA_STRCHR(a,b)	strchr((a),(b))
# define ERA_STRNCPY(a,b,c)	strncpy((a),(b),(c))
# define ERA_STRTOL(a,b,c)	strtol((a),(b),(c))
#endif
  long offset;
  struct tm stm, etm;
  era_info_t *ei;

  ei = (era_info_t *) calloc (1, sizeof (era_info_t));
  if (!ei)
    return NULL;

  stm.tm_isdst = etm.tm_isdst = 0;
  while (era)
    {
      dir = era;
      era += 2;
      offset = ERA_STRTOL (era, &c, 10);
      era = c + 1;
      stm.tm_year = ERA_STRTOL (era, &c, 10) - YEAR_BASE;
      /* Adjust offset for negative gregorian dates. */
      if (stm.tm_year <= -YEAR_BASE)
      	++stm.tm_year;
      stm.tm_mon = ERA_STRTOL (c + 1, &c, 10) - 1;
      stm.tm_mday = ERA_STRTOL (c + 1, &c, 10);
      stm.tm_hour = stm.tm_min = stm.tm_sec = 0;
      era = c + 1;
      if (era[0] == '-' && era[1] == '*')
      	{
	  etm = stm;
	  stm.tm_year = INT_MIN;
	  stm.tm_mon = stm.tm_mday = stm.tm_hour = stm.tm_min = stm.tm_sec = 0;
	  era += 3;
	}
      else if (era[0] == '+' && era[1] == '*')
	{
	  etm.tm_year = INT_MAX;
	  etm.tm_mon = 11;
	  etm.tm_mday = 31;
	  etm.tm_hour = 23;
	  etm.tm_min = etm.tm_sec = 59;
	  era += 3;
	}
      else
      	{
	  etm.tm_year = ERA_STRTOL (era, &c, 10) - YEAR_BASE;
	  /* Adjust offset for negative gregorian dates. */
	  if (etm.tm_year <= -YEAR_BASE)
	    ++etm.tm_year;
	  etm.tm_mon = ERA_STRTOL (c + 1, &c, 10) - 1;
	  etm.tm_mday = ERA_STRTOL (c + 1, &c, 10);
	  etm.tm_mday = 31;
	  etm.tm_hour = 23;
	  etm.tm_min = etm.tm_sec = 59;
	  era = c + 1;
	}
      if ((tim_p->tm_year > stm.tm_year
	   || (tim_p->tm_year == stm.tm_year
	       && (tim_p->tm_mon > stm.tm_mon
		   || (tim_p->tm_mon == stm.tm_mon
		       && tim_p->tm_mday >= stm.tm_mday))))
	  && (tim_p->tm_year < etm.tm_year
	      || (tim_p->tm_year == etm.tm_year
		  && (tim_p->tm_mon < etm.tm_mon
		      || (tim_p->tm_mon == etm.tm_mon
			  && tim_p->tm_mday <= etm.tm_mday)))))
	{
	  /* Gotcha */
	  size_t len;

	  /* year */
	  if (*dir == '+' && stm.tm_year != INT_MIN)
	    ei->year = tim_p->tm_year - stm.tm_year + offset;
	  else
	    ei->year = etm.tm_year - tim_p->tm_year + offset;
	  /* era_C */
	  c = ERA_STRCHR (era, ':');
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  len = mbsnrtowcs (NULL, &era, c - era, 0, NULL);
	  if (len == (size_t) -1)
	    {
	      free (ei);
	      return NULL;
	    }
#else
	  len = c - era;
#endif
	  ei->era_C = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
	  if (!ei->era_C)
	    {
	      free (ei);
	      return NULL;
	    }
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  len = mbsnrtowcs (ei->era_C, &era, c - era, len + 1, NULL);
#else
	  ERA_STRNCPY (ei->era_C, era, len);
	  era += len;
#endif
	  ei->era_C[len] = CQ('\0');
	  /* era_Y */
	  ++era;
	  c = ERA_STRCHR (era, ';');
	  if (!c)
	    c = ERA_STRCHR (era, '\0');
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  len = mbsnrtowcs (NULL, &era, c - era, 0, NULL);
	  if (len == (size_t) -1)
	    {
	      free (ei->era_C);
	      free (ei);
	      return NULL;
	    }
#else
	  len = c - era;
#endif
	  ei->era_Y = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
	  if (!ei->era_Y)
	    {
	      free (ei->era_C);
	      free (ei);
	      return NULL;
	    }
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  len = mbsnrtowcs (ei->era_Y, &era, c - era, len + 1, NULL);
#else
	  ERA_STRNCPY (ei->era_Y, era, len);
	  era += len;
#endif
	  ei->era_Y[len] = CQ('\0');
	  return ei;
	}
      else
	era = ERA_STRCHR (era, ';');
      if (era)
	++era;
    }
  return NULL;
}

static void
free_era_info (era_info_t *ei)
{
  free (ei->era_C);
  free (ei->era_Y);
  free (ei);
}

typedef struct {
  size_t num;
  CHAR **digit;
  CHAR *buffer;
} alt_digits_t;

static alt_digits_t *
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
get_alt_digits (const wchar_t *alt_digits)
#else
get_alt_digits (const char *alt_digits)
#endif
{
  alt_digits_t *adi;
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
  const wchar_t *a, *e;
# define ALT_STRCHR(a,b)	wcschr((a),(b))
# define ALT_STRCPY(a,b)	wcscpy((a),(b))
# define ALT_STRLEN(a)		wcslen(a)
#else
  const char *a, *e;
# define ALT_STRCHR(a,b)	strchr((a),(b))
# define ALT_STRCPY(a,b)	strcpy((a),(b))
# define ALT_STRLEN(a)		strlen(a)
#endif
  CHAR *aa, *ae;
  size_t len;

  adi = (alt_digits_t *) calloc (1, sizeof (alt_digits_t));
  if (!adi)
    return NULL;

  /* Compute number of alt_digits. */
  adi->num = 1;
  for (a = alt_digits; (e = ALT_STRCHR (a, ';')) != NULL; a = e + 1)
      ++adi->num;
  /* Allocate the `digit' array, which is an array of `num' pointers into
     `buffer'. */
  adi->digit = (CHAR **) calloc (adi->num, sizeof (CHAR *));
  if (!adi->digit)
    {
      free (adi);
      return NULL;
    }
  /* Compute memory required for `buffer'. */
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
  len = mbstowcs (NULL, alt_digits, 0);
  if (len == (size_t) -1)
    {
      free (adi->digit);
      free (adi);
      return NULL;
    }
#else
  len = ALT_STRLEN (alt_digits);
#endif
  /* Allocate it. */
  adi->buffer = (CHAR *) malloc ((len + 1) * sizeof (CHAR));
  if (!adi->buffer)
    {
      free (adi->digit);
      free (adi);
      return NULL;
    }
  /* Store digits in it. */
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
  mbstowcs (adi->buffer, alt_digits, len + 1);
#else
  ALT_STRCPY (adi->buffer, alt_digits);
#endif
  /* Store the pointers into `buffer' into the appropriate `digit' slot. */
  for (len = 0, aa = adi->buffer; (ae = STRCHR (aa, CQ(';'))) != NULL;
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

/* Return 0 if no alt_digit is available for a number.
   Return -1 if buffer size isn't sufficient to hold alternative digit.
   Return length of new digit otherwise. */
static int
conv_to_alt_digits (CHAR *buf, size_t bufsiz, unsigned num, alt_digits_t *adi)
{
  if (num < adi->num)
    {
      size_t len = STRLEN (adi->digit[num]);
      if (bufsiz < len)
      	return -1;
      STRCPY (buf, adi->digit[num]);
      return (int) len;
    }
  return 0;
}

static size_t
__strftime (CHAR *s, size_t maxsize, const CHAR *format,
	    const struct tm *tim_p, struct __locale_t *locale,
	    era_info_t **era_info, alt_digits_t **alt_digits)
#else /* !_WANT_C99_TIME_FORMATS */
static size_t
__strftime (CHAR *s, size_t maxsize, const CHAR *format,
	    const struct tm *tim_p, struct __locale_t *locale)

#define __strftime(s,m,f,t,l,e,a)	__strftime((s),(m),(f),(t),(l))
#endif /* !_WANT_C99_TIME_FORMATS */
{
  size_t count = 0;
  int len = 0;
  const CHAR *ctloc;
#if defined (MAKE_WCSFTIME) && !defined (__HAVE_LOCALE_INFO_EXTENDED__)
  CHAR ctlocbuf[CTLOCBUFLEN];
#endif
  size_t i, ctloclen;
  CHAR alt;
  CHAR pad;
  unsigned long width;
  int tzset_called = 0;

  const struct lc_time_T *_CurrentTimeLocale = __get_time_locale (locale);
  for (;;)
    {
      while (*format && *format != CQ('%'))
	{
	  if (count < maxsize - 1)
	    s[count++] = *format++;
	  else
	    return 0;
	}
      if (*format == CQ('\0'))
	break;
      format++;
      pad = '\0';
      width = 0;

      /* POSIX-1.2008 feature: '0' and '+' modifiers require 0-padding with
         slightly different semantics. */
      if (*format == CQ('0') || *format == CQ('+'))
	pad = *format++;

      /* POSIX-1.2008 feature: A minimum field width can be specified. */
      if (*format >= CQ('1') && *format <= CQ('9'))
      	{
	  CHAR *fp;
	  width = STRTOUL (format, &fp, 10);
	  format = fp;
	}

      alt = CQ('\0');
      if (*format == CQ('E'))
	{
	  alt = *format++;
#ifdef _WANT_C99_TIME_FORMATS
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  if (!*era_info && *_CurrentTimeLocale->wera)
	    *era_info = get_era_info (tim_p, _CurrentTimeLocale->wera);
#else
	  if (!*era_info && *_CurrentTimeLocale->era)
	    *era_info = get_era_info (tim_p, _CurrentTimeLocale->era);
#endif
#endif /* _WANT_C99_TIME_FORMATS */
	}
      else if (*format == CQ('O'))
	{
	  alt = *format++;
#ifdef _WANT_C99_TIME_FORMATS
#if defined (MAKE_WCSFTIME) && defined (__HAVE_LOCALE_INFO_EXTENDED__)
	  if (!*alt_digits && *_CurrentTimeLocale->walt_digits)
	    *alt_digits = get_alt_digits (_CurrentTimeLocale->walt_digits);
#else
	  if (!*alt_digits && *_CurrentTimeLocale->alt_digits)
	    *alt_digits = get_alt_digits (_CurrentTimeLocale->alt_digits);
#endif
#endif /* _WANT_C99_TIME_FORMATS */
	}

      switch (*format)
	{
	case CQ('a'):
	  _ctloc (wday[tim_p->tm_wday]);
	  for (i = 0; i < ctloclen; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] = ctloc[i];
	      else
		return 0;
	    }
	  break;
	case CQ('A'):
	  _ctloc (weekday[tim_p->tm_wday]);
	  for (i = 0; i < ctloclen; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] = ctloc[i];
	      else
		return 0;
	    }
	  break;
	case CQ('b'):
	case CQ('h'):
	  _ctloc (mon[tim_p->tm_mon]);
	  for (i = 0; i < ctloclen; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] = ctloc[i];
	      else
		return 0;
	    }
	  break;
	case CQ('B'):
	  _ctloc (month[tim_p->tm_mon]);
	  for (i = 0; i < ctloclen; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] = ctloc[i];
	      else
		return 0;
	    }
	  break;
	case CQ('c'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == 'E' && *era_info && *_CurrentTimeLocale->era_d_t_fmt)
	    _ctloc (era_d_t_fmt);
	  else
#endif /* _WANT_C99_TIME_FORMATS */
	    _ctloc (c_fmt);
	  goto recurse;
	case CQ('r'):
	  _ctloc (ampm_fmt);
	  goto recurse;
	case CQ('x'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == 'E' && *era_info && *_CurrentTimeLocale->era_d_fmt)
	    _ctloc (era_d_fmt);
	  else
#endif /* _WANT_C99_TIME_FORMATS */
	    _ctloc (x_fmt);
	  goto recurse;
	case CQ('X'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == 'E' && *era_info && *_CurrentTimeLocale->era_t_fmt)
	    _ctloc (era_t_fmt);
	  else
#endif /* _WANT_C99_TIME_FORMATS */
	    _ctloc (X_fmt);
recurse:
	  if (*ctloc)
	    {
	      /* Recurse to avoid need to replicate %Y formation. */
	      len = __strftime (&s[count], maxsize - count, ctloc, tim_p,
				locale, era_info, alt_digits);
	      if (len > 0)
		count += len;
	      else
		return 0;
	    }
	  break;
	case CQ('C'):
	  {
	    /* Examples of (tm_year + YEAR_BASE) that show how %Y == %C%y
	       with 32-bit int.
	       %Y		%C		%y
	       2147485547	21474855	47
	       10000		100		00
	       9999		99		99
	       0999		09		99
	       0099		00		99
	       0001		00		01
	       0000		00		00
	       -001		-0		01
	       -099		-0		99
	       -999		-9		99
	       -1000		-10		00
	       -10000		-100		00
	       -2147481748	-21474817	48

	       Be careful of both overflow and sign adjustment due to the
	       asymmetric range of years.
	    */
#ifdef _WANT_C99_TIME_FORMATS
	    if (alt == 'E' && *era_info)
	      len = snprintf (&s[count], maxsize - count, CQ("%" SFLG "s"),
			      (*era_info)->era_C);
	    else
#endif /* _WANT_C99_TIME_FORMATS */
	      {
		CHAR *fmt = CQ("%s%.*d");
		char *pos = "";
		int neg = tim_p->tm_year < -YEAR_BASE;
		int century = tim_p->tm_year >= 0
		  ? tim_p->tm_year / 100 + YEAR_BASE / 100
		  : abs (tim_p->tm_year + YEAR_BASE) / 100;
		if (pad) /* '0' or '+' */
		  {
		    fmt = CQ("%s%0.*d");
		    if (century >= 100 && pad == CQ('+'))
		      pos = "+";
		  }
		if (width < 2)
		  width = 2;
		len = snprintf (&s[count], maxsize - count, fmt,
				neg ? "-" : pos, width - neg, century);
	      }
            CHECK_LENGTH ();
	  }
	  break;
	case CQ('d'):
	case CQ('e'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == CQ('O') && *alt_digits)
	    {
	      if (tim_p->tm_mday < 10)
	      	{
		  if (*format == CQ('d'))
		    {
		      if (maxsize - count < 2) return 0;
		      len = conv_to_alt_digits (&s[count], maxsize - count,
						0, *alt_digits);
		      CHECK_LENGTH ();
		    }
		  if (*format == CQ('e') || len == 0)
		    s[count++] = CQ(' ');
		}
	      len = conv_to_alt_digits (&s[count], maxsize - count,
					tim_p->tm_mday, *alt_digits);
	      CHECK_LENGTH ();
	      if (len > 0)
		break;
	    }
#endif /* _WANT_C99_TIME_FORMATS */
	  len = snprintf (&s[count], maxsize - count,
			  *format == CQ('d') ? CQ("%.2d") : CQ("%2d"),
			  tim_p->tm_mday);
	  CHECK_LENGTH ();
	  break;
	case CQ('D'):
	  /* %m/%d/%y */
	  len = snprintf (&s[count], maxsize - count,
			  CQ("%.2d/%.2d/%.2d"),
			  tim_p->tm_mon + 1, tim_p->tm_mday,
			  tim_p->tm_year >= 0 ? tim_p->tm_year % 100
			  : abs (tim_p->tm_year + YEAR_BASE) % 100);
          CHECK_LENGTH ();
	  break;
	case CQ('F'):
	  { /* %F is equivalent to "%+4Y-%m-%d", flags and width can change
	       that.  Recurse to avoid need to replicate %Y formation. */
	    CHAR fmtbuf[32], *fmt = fmtbuf;

	    *fmt++ = CQ('%');
	    if (pad) /* '0' or '+' */
	      *fmt++ = pad;
	    else
	      *fmt++ = '+';
	    if (!pad)
	      width = 10;
	    if (width < 6)
	      width = 6;
	    width -= 6;
	    if (width)
	      {
		len = snprintf (fmt, fmtbuf + 32 - fmt, CQ("%lu"), width);
		if (len > 0)
		  fmt += len;
	      }
	    STRCPY (fmt, CQ("Y-%m-%d"));
	    len = __strftime (&s[count], maxsize - count, fmtbuf, tim_p,
			      locale, era_info, alt_digits);
	    if (len > 0)
	      count += len;
	    else
	      return 0;
	  }
          break;
	case CQ('g'):
	  /* Be careful of both overflow and negative years, thanks to
		 the asymmetric range of years.  */
	  {
	    int adjust = iso_year_adjust (tim_p);
	    int year = tim_p->tm_year >= 0 ? tim_p->tm_year % 100
		: abs (tim_p->tm_year + YEAR_BASE) % 100;
	    if (adjust < 0 && tim_p->tm_year <= -YEAR_BASE)
		adjust = 1;
	    else if (adjust > 0 && tim_p->tm_year < -YEAR_BASE)
		adjust = -1;
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			    ((year + adjust) % 100 + 100) % 100);
            CHECK_LENGTH ();
	  }
          break;
	case CQ('G'):
	  {
	    /* See the comments for 'C' and 'Y'; this is a variable length
	       field.  Although there is no requirement for a minimum number
	       of digits, we use 4 for consistency with 'Y'.  */
	    int sign = tim_p->tm_year < -YEAR_BASE;
	    int adjust = iso_year_adjust (tim_p);
	    int century = tim_p->tm_year >= 0
	      ? tim_p->tm_year / 100 + YEAR_BASE / 100
	      : abs (tim_p->tm_year + YEAR_BASE) / 100;
	    int year = tim_p->tm_year >= 0 ? tim_p->tm_year % 100
	      : abs (tim_p->tm_year + YEAR_BASE) % 100;
	    if (adjust < 0 && tim_p->tm_year <= -YEAR_BASE)
	      sign = adjust = 1;
	    else if (adjust > 0 && sign)
	      adjust = -1;
	    year += adjust;
	    if (year == -1)
	      {
		year = 99;
		--century;
	      }
	    else if (year == 100)
	      {
		year = 0;
		++century;
	      }
	    CHAR fmtbuf[10], *fmt = fmtbuf;
	    /* int potentially overflows, so use unsigned instead.  */
	    unsigned p_year = century * 100 + year;
	    if (sign)
	      *fmt++ = CQ('-');
	    else if (pad == CQ('+') && p_year >= 10000)
	      {
		*fmt++ = CQ('+');
		sign = 1;
	      }
	    if (width && sign)
	      --width;
	    *fmt++ = CQ('%');
	    if (pad)
	      *fmt++ = CQ('0');
	    STRCPY (fmt, CQ(".*u"));
	    len = snprintf (&s[count], maxsize - count, fmtbuf, width, p_year);
            if (len < 0  ||  (count+=len) >= maxsize)
              return 0;
	  }
          break;
	case CQ('H'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == CQ('O') && *alt_digits)
	    {
	      len = conv_to_alt_digits (&s[count], maxsize - count,
					tim_p->tm_hour, *alt_digits);
	      CHECK_LENGTH ();
	      if (len > 0)
		break;
	    }
#endif /* _WANT_C99_TIME_FORMATS */
	  /*FALLTHRU*/
	case CQ('k'):	/* newlib extension */
	  len = snprintf (&s[count], maxsize - count,
			  *format == CQ('k') ? CQ("%2d") : CQ("%.2d"),
			  tim_p->tm_hour);
          CHECK_LENGTH ();
	  break;
	case CQ('l'):	/* newlib extension */
	  if (alt == CQ('O'))
	    alt = CQ('\0');
	  /*FALLTHRU*/
	case CQ('I'):
	  {
	    register int  h12;
	    h12 = (tim_p->tm_hour == 0 || tim_p->tm_hour == 12)  ?
						12  :  tim_p->tm_hour % 12;
#ifdef _WANT_C99_TIME_FORMATS
	    if (alt != CQ('O') || !*alt_digits
		|| !(len = conv_to_alt_digits (&s[count], maxsize - count,
					       h12, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	      len = snprintf (&s[count], maxsize - count,
			      *format == CQ('I') ? CQ("%.2d") : CQ("%2d"), h12);
	    CHECK_LENGTH ();
	  }
	  break;
	case CQ('j'):
	  len = snprintf (&s[count], maxsize - count, CQ("%.3d"),
			  tim_p->tm_yday + 1);
          CHECK_LENGTH ();
	  break;
	case CQ('m'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt != CQ('O') || !*alt_digits
	      || !(len = conv_to_alt_digits (&s[count], maxsize - count,
					     tim_p->tm_mon + 1, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			    tim_p->tm_mon + 1);
          CHECK_LENGTH ();
	  break;
	case CQ('M'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt != CQ('O') || !*alt_digits
	      || !(len = conv_to_alt_digits (&s[count], maxsize - count,
					     tim_p->tm_min, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			    tim_p->tm_min);
          CHECK_LENGTH ();
	  break;
	case CQ('n'):
	  if (count < maxsize - 1)
	    s[count++] = CQ('\n');
	  else
	    return 0;
	  break;
	case CQ('p'):
	case CQ('P'):
	  _ctloc (am_pm[tim_p->tm_hour < 12 ? 0 : 1]);
	  for (i = 0; i < ctloclen; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] = (*format == CQ('P') ? TOLOWER (ctloc[i])
						 : ctloc[i]);
	      else
		return 0;
	    }
	  break;
	case CQ('q'):	/* GNU quarter year */
	  len = snprintf (&s[count], maxsize - count, CQ("%.1d"),
			  tim_p->tm_mon / 3 + 1);
	  CHECK_LENGTH ();
	  break;
	case CQ('R'):
          len = snprintf (&s[count], maxsize - count, CQ("%.2d:%.2d"),
			  tim_p->tm_hour, tim_p->tm_min);
          CHECK_LENGTH ();
          break;
	case CQ('s'):
/*
 * From:
 * The Open Group Base Specifications Issue 7
 * IEEE Std 1003.1, 2013 Edition
 * Copyright (c) 2001-2013 The IEEE and The Open Group
 * XBD Base Definitions
 * 4. General Concepts
 * 4.15 Seconds Since the Epoch
 * A value that approximates the number of seconds that have elapsed since the
 * Epoch. A Coordinated Universal Time name (specified in terms of seconds
 * (tm_sec), minutes (tm_min), hours (tm_hour), days since January 1 of the year
 * (tm_yday), and calendar year minus 1900 (tm_year)) is related to a time
 * represented as seconds since the Epoch, according to the expression below.
 * If the year is <1970 or the value is negative, the relationship is undefined.
 * If the year is >=1970 and the value is non-negative, the value is related to a
 * Coordinated Universal Time name according to the C-language expression, where
 * tm_sec, tm_min, tm_hour, tm_yday, and tm_year are all integer types:
 * tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *     (tm_year-70)*31536000 + ((tm_year-69)/4)*86400 -
 *     ((tm_year-1)/100)*86400 + ((tm_year+299)/400)*86400
 * OR
 * ((((tm_year-69)/4 - (tm_year-1)/100 + (tm_year+299)/400 +
 *         (tm_year-70)*365 + tm_yday)*24 + tm_hour)*60 + tm_min)*60 + tm_sec
 */
/* modified from %z case by hoisting offset outside if block and initializing */
	  {
	    long offset = 0;	/* offset < 0 => W of GMT, > 0 => E of GMT:
				   subtract to get UTC */

	    if (tim_p->tm_isdst >= 0)
	      {
		TZ_LOCK;
		if (!tzset_called)
		  {
		    _tzset_unlocked ();
		    tzset_called = 1;
		  }

#if defined (__CYGWIN__)
		/* Cygwin must check if the application has been built with or
		   without the extra tm members for backward compatibility, and
		   then use either that or the old method fetching from tzinfo.
		   Rather than pulling in the version check infrastructure, we
		   just call a Cygwin function. */
		extern long __cygwin_gettzoffset (const struct tm *tmp);
		offset = __cygwin_gettzoffset (tim_p);
#elif defined (__TM_GMTOFF)
		offset = tim_p->__TM_GMTOFF;
#else
		__tzinfo_type *tz = __gettzinfo ();
		/* The sign of this is exactly opposite the envvar TZ.  We
		   could directly use the global _timezone for tm_isdst==0,
		   but have to use __tzrule for daylight savings.  */
		offset = -tz->__tzrule[tim_p->tm_isdst > 0].offset;
#endif
		TZ_UNLOCK;
	      }
	    len = snprintf (&s[count], maxsize - count, CQ("%lld"),
			    (((((long long)tim_p->tm_year - 69)/4
				- (tim_p->tm_year - 1)/100
				+ (tim_p->tm_year + 299)/400
				+ (tim_p->tm_year - 70)*365 + tim_p->tm_yday)*24
			      + tim_p->tm_hour)*60 + tim_p->tm_min)*60
			    + tim_p->tm_sec - offset);
	    CHECK_LENGTH ();
	  }
          break;
	case CQ('S'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt != CQ('O') || !*alt_digits
	      || !(len = conv_to_alt_digits (&s[count], maxsize - count,
					     tim_p->tm_sec, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			    tim_p->tm_sec);
          CHECK_LENGTH ();
	  break;
	case CQ('t'):
	  if (count < maxsize - 1)
	    s[count++] = CQ('\t');
	  else
	    return 0;
	  break;
	case CQ('T'):
          len = snprintf (&s[count], maxsize - count, CQ("%.2d:%.2d:%.2d"),
			  tim_p->tm_hour, tim_p->tm_min, tim_p->tm_sec);
          CHECK_LENGTH ();
          break;
	case CQ('u'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == CQ('O') && *alt_digits)
	    {
	      len = conv_to_alt_digits (&s[count], maxsize - count,
					tim_p->tm_wday == 0 ? 7
							    : tim_p->tm_wday,
					*alt_digits);
	      CHECK_LENGTH ();
	      if (len > 0)
		break;
	    }
#endif /* _WANT_C99_TIME_FORMATS */
          if (count < maxsize - 1)
            {
              if (tim_p->tm_wday == 0)
                s[count++] = CQ('7');
              else
                s[count++] = CQ('0') + tim_p->tm_wday;
            }
          else
            return 0;
          break;
	case CQ('U'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt != CQ('O') || !*alt_digits
	      || !(len = conv_to_alt_digits (&s[count], maxsize - count,
					     (tim_p->tm_yday + 7 -
					      tim_p->tm_wday) / 7,
					     *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			 (tim_p->tm_yday + 7 -
			  tim_p->tm_wday) / 7);
          CHECK_LENGTH ();
	  break;
	case CQ('V'):
	  {
	    int adjust = iso_year_adjust (tim_p);
	    int wday = (tim_p->tm_wday) ? tim_p->tm_wday - 1 : 6;
	    int week = (tim_p->tm_yday + 10 - wday) / 7;
	    if (adjust > 0)
		week = 1;
	    else if (adjust < 0)
		/* Previous year has 53 weeks if current year starts on
		   Fri, and also if current year starts on Sat and
		   previous year was leap year.  */
		week = 52 + (4 >= (wday - tim_p->tm_yday
				   - isleap (tim_p->tm_year
					     + (YEAR_BASE - 1
						- (tim_p->tm_year < 0
						   ? 0 : 2000)))));
#ifdef _WANT_C99_TIME_FORMATS
	    if (alt != CQ('O') || !*alt_digits
		|| !(len = conv_to_alt_digits (&s[count], maxsize - count,
					       week, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	      len = snprintf (&s[count], maxsize - count, CQ("%.2d"), week);
            CHECK_LENGTH ();
	  }
          break;
	case CQ('v'):	/* BSD/OSX/Ruby extension VMS/Oracle date format
			   from Arnold Robbins strftime version 3.0 */
	  { /* %v is equivalent to "%e-%b-%Y", flags and width can change year
	       format. Recurse to avoid need to replicate %b and %Y formation. */
	    CHAR fmtbuf[32], *fmt = fmtbuf;
	    STRCPY (fmt, CQ("%e-%b-%"));
	    fmt += STRLEN (fmt);
	    if (pad) /* '0' or '+' */
	      *fmt++ = pad;
	    else
	      *fmt++ = '+';
	    if (!pad)
	      width = 10;
	    if (width < 6)
	      width = 6;
	    width -= 6;
	    if (width)
	      {
		len = snprintf (fmt, fmtbuf + 32 - fmt, CQ("%lu"), width);
		if (len > 0)
		  fmt += len;
	      }
	    STRCPY (fmt, CQ("Y"));
	    len = __strftime (&s[count], maxsize - count, fmtbuf, tim_p,
			      locale, era_info, alt_digits);
	    if (len > 0)
	      count += len;
	    else
	      return 0;
	  }
	  break;
	case CQ('w'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == CQ('O') && *alt_digits)
	    {
	      len = conv_to_alt_digits (&s[count], maxsize - count,
					tim_p->tm_wday, *alt_digits);
	      CHECK_LENGTH ();
	      if (len > 0)
		break;
	    }
#endif /* _WANT_C99_TIME_FORMATS */
	  if (count < maxsize - 1)
            s[count++] = CQ('0') + tim_p->tm_wday;
	  else
	    return 0;
	  break;
	case CQ('W'):
	  {
	    int wday = (tim_p->tm_wday) ? tim_p->tm_wday - 1 : 6;
	    wday = (tim_p->tm_yday + 7 - wday) / 7;
#ifdef _WANT_C99_TIME_FORMATS
	    if (alt != CQ('O') || !*alt_digits
		|| !(len = conv_to_alt_digits (&s[count], maxsize - count,
					       wday, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
	      len = snprintf (&s[count], maxsize - count, CQ("%.2d"), wday);
            CHECK_LENGTH ();
	  }
	  break;
	case CQ('y'):
	    {
#ifdef _WANT_C99_TIME_FORMATS
	      if (alt == 'E' && *era_info)
		len = snprintf (&s[count], maxsize - count, CQ("%d"),
				(*era_info)->year);
	      else
#endif /* _WANT_C99_TIME_FORMATS */
		{
		  /* Be careful of both overflow and negative years, thanks to
		     the asymmetric range of years.  */
		  int year = tim_p->tm_year >= 0 ? tim_p->tm_year % 100
			     : abs (tim_p->tm_year + YEAR_BASE) % 100;
#ifdef _WANT_C99_TIME_FORMATS
		  if (alt != CQ('O') || !*alt_digits
		      || !(len = conv_to_alt_digits (&s[count], maxsize - count,
						     year, *alt_digits)))
#endif /* _WANT_C99_TIME_FORMATS */
		    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
				    year);
		}
              CHECK_LENGTH ();
	    }
	  break;
	case CQ('Y'):
#ifdef _WANT_C99_TIME_FORMATS
	  if (alt == 'E' && *era_info)
	    {
	      ctloc = (*era_info)->era_Y;
	      goto recurse;
	    }
	  else
#endif /* _WANT_C99_TIME_FORMATS */
	    {
	      CHAR fmtbuf[10], *fmt = fmtbuf;
	      int sign = tim_p->tm_year < -YEAR_BASE;
	      /* int potentially overflows, so use unsigned instead.  */
	      register unsigned year = (unsigned) tim_p->tm_year
				       + (unsigned) YEAR_BASE;
	      if (sign)
		{
		  *fmt++ = CQ('-');
		  year = UINT_MAX - year + 1;
		}
	      else if (pad == CQ('+') && year >= 10000)
		{
		  *fmt++ = CQ('+');
		  sign = 1;
		}
	      if (width && sign)
		--width;
	      *fmt++ = CQ('%');
	      if (pad)
		*fmt++ = CQ('0');
	      STRCPY (fmt, CQ(".*u"));
	      len = snprintf (&s[count], maxsize - count, fmtbuf, width,
			      year);
	      CHECK_LENGTH ();
	    }
	  break;
	case CQ('z'):
          if (tim_p->tm_isdst >= 0)
            {
	      long offset;

	      TZ_LOCK;
	      if (!tzset_called)
		{
		  _tzset_unlocked ();
		  tzset_called = 1;
		}

#if defined (__CYGWIN__)
	      /* Cygwin must check if the application has been built with or
		 without the extra tm members for backward compatibility, and
		 then use either that or the old method fetching from tzinfo.
		 Rather than pulling in the version check infrastructure, we
		 just call a Cygwin function. */
	      extern long __cygwin_gettzoffset (const struct tm *tmp);
	      offset = __cygwin_gettzoffset (tim_p);
#elif defined (__TM_GMTOFF)
	      offset = tim_p->__TM_GMTOFF;
#else
	      __tzinfo_type *tz = __gettzinfo ();
	      /* The sign of this is exactly opposite the envvar TZ.  We
		 could directly use the global _timezone for tm_isdst==0,
		 but have to use __tzrule for daylight savings.  */
	      offset = -tz->__tzrule[tim_p->tm_isdst > 0].offset;
#endif
	      TZ_UNLOCK;
	      len = snprintf (&s[count], maxsize - count, CQ("%+03ld%.2ld"),
			      offset / SECSPERHOUR,
			      labs (offset / SECSPERMIN) % 60L);
              CHECK_LENGTH ();
            }
          break;
	case CQ('Z'):
	  if (tim_p->tm_isdst >= 0)
	    {
	      size_t size;
	      const char *tznam = NULL;

	      TZ_LOCK;
	      if (!tzset_called)
		{
		  _tzset_unlocked ();
		  tzset_called = 1;
		}
#if defined (__CYGWIN__)
	      /* See above. */
	      extern const char *__cygwin_gettzname (const struct tm *tmp);
	      tznam = __cygwin_gettzname (tim_p);
#elif defined (__TM_ZONE)
	      tznam = tim_p->__TM_ZONE;
#endif
	      if (!tznam)
		tznam = _tzname[tim_p->tm_isdst > 0];
	      /* Note that in case of wcsftime this loop only works for
	         timezone abbreviations using the portable codeset (aka ASCII).
		 This seems to be the case, but if that ever changes, this
		 loop needs revisiting. */
	      size = strlen (tznam);
	      for (i = 0; i < size; i++)
		{
		  if (count < maxsize - 1)
		    s[count++] = tznam[i];
		  else
		    {
		      TZ_UNLOCK;
		      return 0;
		    }
		}
	      TZ_UNLOCK;
	    }
	  break;
	case CQ('%'):
	  if (count < maxsize - 1)
	    s[count++] = CQ('%');
	  else
	    return 0;
	  break;
	default:
	  return 0;
	}
      if (*format)
	format++;
      else
	break;
    }
  if (maxsize)
    s[count] = CQ('\0');

  return count;
}

size_t
strftime (CHAR *__restrict s,
	size_t maxsize,
	const CHAR *__restrict format,
	const struct tm *__restrict tim_p)
{
#ifdef _WANT_C99_TIME_FORMATS
  era_info_t *era_info = NULL;
  alt_digits_t *alt_digits = NULL;
  size_t ret = __strftime (s, maxsize, format, tim_p, __get_current_locale (),
			   &era_info, &alt_digits);
  if (era_info)
    free_era_info (era_info);
  if (alt_digits)
    free_alt_digits (alt_digits);
  return ret;
#else /* !_WANT_C99_TIME_FORMATS */
  return __strftime (s, maxsize, format, tim_p, __get_current_locale (),
		     NULL, NULL);
#endif /* !_WANT_C99_TIME_FORMATS */
}

size_t
strftime_l (CHAR *__restrict s, size_t maxsize, const CHAR *__restrict format,
	    const struct tm *__restrict tim_p, struct __locale_t *locale)
{
#ifdef _WANT_C99_TIME_FORMATS
  era_info_t *era_info = NULL;
  alt_digits_t *alt_digits = NULL;
  size_t ret = __strftime (s, maxsize, format, tim_p, locale,
			   &era_info, &alt_digits);
  if (era_info)
    free_era_info (era_info);
  if (alt_digits)
    free_alt_digits (alt_digits);
  return ret;
#else /* !_WANT_C99_TIME_FORMATS */
  return __strftime (s, maxsize, format, tim_p, locale, NULL, NULL);
#endif /* !_WANT_C99_TIME_FORMATS */
}

/* The remainder of this file can serve as a regression test.  Compile
 *  with -D_REGRESSION_TEST.  */
#if defined(_REGRESSION_TEST)	/* [Test code:  */

/* This test code relies on ANSI C features, in particular on the ability
 * of adjacent strings to be pasted together into one string.  */

/* Test output buffer size (should be larger than all expected results) */
#define OUTSIZE	256

struct test {
	CHAR  *fmt;	/* Testing format */
	size_t  max;	/* Testing maxsize */
	size_t	ret;	/* Expected return value */
	CHAR  *out;	/* Expected output string */
	};
struct list {
	const struct tm  *tms;	/* Time used for these vectors */
	const struct test *vec;	/* Test vectors */
	int  cnt;		/* Number of vectors */
	};

const char  TZ[]="TZ=EST5EDT";

/* Define list of test inputs and expected outputs, for the given time zone
 * and time.  */
const struct tm  tm0 = {
	/* Tue Dec 30 10:53:47 EST 2008 (time_t=1230648827) */
	.tm_sec 	= 47,
	.tm_min 	= 53,
	.tm_hour	= 9,
	.tm_mday	= 30,
	.tm_mon 	= 11,
	.tm_year	= 108,
	.tm_wday	= 2,
	.tm_yday	= 364,
	.tm_isdst	= 0
	};
const struct test  Vec0[] = {
	/* Testing fields one at a time, expecting to pass, using exact
	 * allowed length as what is needed.  */
	/* Using tm0 for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%a"), 3+1, EXP(CQ("Tue")) },
	{ CQ("%A"), 7+1, EXP(CQ("Tuesday")) },
	{ CQ("%b"), 3+1, EXP(CQ("Dec")) },
	{ CQ("%B"), 8+1, EXP(CQ("December")) },
	{ CQ("%c"), 24+1, EXP(CQ("Tue Dec 30 09:53:47 2008")) },
	{ CQ("%C"), 2+1, EXP(CQ("20")) },
	{ CQ("%d"), 2+1, EXP(CQ("30")) },
	{ CQ("%D"), 8+1, EXP(CQ("12/30/08")) },
	{ CQ("%e"), 2+1, EXP(CQ("30")) },
	{ CQ("%F"), 10+1, EXP(CQ("2008-12-30")) },
	{ CQ("%g"), 2+1, EXP(CQ("09")) },
	{ CQ("%G"), 4+1, EXP(CQ("2009")) },
	{ CQ("%h"), 3+1, EXP(CQ("Dec")) },
	{ CQ("%H"), 2+1, EXP(CQ("09")) },
	{ CQ("%I"), 2+1, EXP(CQ("09")) },
	{ CQ("%j"), 3+1, EXP(CQ("365")) },
	{ CQ("%k"), 2+1, EXP(CQ(" 9")) },
	{ CQ("%l"), 2+1, EXP(CQ(" 9")) },
	{ CQ("%m"), 2+1, EXP(CQ("12")) },
	{ CQ("%M"), 2+1, EXP(CQ("53")) },
	{ CQ("%n"), 1+1, EXP(CQ("\n")) },
	{ CQ("%p"), 2+1, EXP(CQ("AM")) },
	{ CQ("%q"), 1+1, EXP(CQ("4")) },
	{ CQ("%r"), 11+1, EXP(CQ("09:53:47 AM")) },
	{ CQ("%R"), 5+1, EXP(CQ("09:53")) },
	{ CQ("%s"), 2+1, EXP(CQ("1230648827")) },
	{ CQ("%S"), 2+1, EXP(CQ("47")) },
	{ CQ("%t"), 1+1, EXP(CQ("\t")) },
	{ CQ("%T"), 8+1, EXP(CQ("09:53:47")) },
	{ CQ("%u"), 1+1, EXP(CQ("2")) },
	{ CQ("%U"), 2+1, EXP(CQ("52")) },
	{ CQ("%V"), 2+1, EXP(CQ("01")) },
	{ CQ("%v"), 11+1, EXP(CQ("30-Dec-2008")) },
	{ CQ("%w"), 1+1, EXP(CQ("2")) },
	{ CQ("%W"), 2+1, EXP(CQ("52")) },
	{ CQ("%x"), 8+1, EXP(CQ("12/30/08")) },
	{ CQ("%X"), 8+1, EXP(CQ("09:53:47")) },
	{ CQ("%y"), 2+1, EXP(CQ("08")) },
	{ CQ("%Y"), 4+1, EXP(CQ("2008")) },
	{ CQ("%z"), 5+1, EXP(CQ("-0500")) },
	{ CQ("%Z"), 3+1, EXP(CQ("EST")) },
	{ CQ("%%"), 1+1, EXP(CQ("%")) },
	#undef EXP
	};
/* Define list of test inputs and expected outputs, for the given time zone
 * and time.  */
const struct tm  tm1 = {
	/* Wed Jul  2 23:01:13 EDT 2008 (time_t=1215054073) */
	.tm_sec 	= 13,
	.tm_min 	= 1,
	.tm_hour	= 23,
	.tm_mday	= 2,
	.tm_mon 	= 6,
	.tm_year	= 108,
	.tm_wday	= 3,
	.tm_yday	= 183,
	.tm_isdst	= 1
	};
const struct test  Vec1[] = {
	/* Testing fields one at a time, expecting to pass, using exact
	 * allowed length as what is needed.  */
	/* Using tm1 for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%a"), 3+1, EXP(CQ("Wed")) },
	{ CQ("%A"), 9+1, EXP(CQ("Wednesday")) },
	{ CQ("%b"), 3+1, EXP(CQ("Jul")) },
	{ CQ("%B"), 4+1, EXP(CQ("July")) },
	{ CQ("%c"), 24+1, EXP(CQ("Wed Jul  2 23:01:13 2008")) },
	{ CQ("%C"), 2+1, EXP(CQ("20")) },
	{ CQ("%d"), 2+1, EXP(CQ("02")) },
	{ CQ("%D"), 8+1, EXP(CQ("07/02/08")) },
	{ CQ("%e"), 2+1, EXP(CQ(" 2")) },
	{ CQ("%F"), 10+1, EXP(CQ("2008-07-02")) },
	{ CQ("%g"), 2+1, EXP(CQ("08")) },
	{ CQ("%G"), 4+1, EXP(CQ("2008")) },
	{ CQ("%h"), 3+1, EXP(CQ("Jul")) },
	{ CQ("%H"), 2+1, EXP(CQ("23")) },
	{ CQ("%I"), 2+1, EXP(CQ("11")) },
	{ CQ("%j"), 3+1, EXP(CQ("184")) },
	{ CQ("%k"), 2+1, EXP(CQ("23")) },
	{ CQ("%l"), 2+1, EXP(CQ("11")) },
	{ CQ("%m"), 2+1, EXP(CQ("07")) },
	{ CQ("%M"), 2+1, EXP(CQ("01")) },
	{ CQ("%n"), 1+1, EXP(CQ("\n")) },
	{ CQ("%p"), 2+1, EXP(CQ("PM")) },
	{ CQ("%q"), 1+1, EXP(CQ("3")) },
	{ CQ("%r"), 11+1, EXP(CQ("11:01:13 PM")) },
	{ CQ("%R"), 5+1, EXP(CQ("23:01")) },
	{ CQ("%s"), 2+1, EXP(CQ("1215054073")) },
	{ CQ("%S"), 2+1, EXP(CQ("13")) },
	{ CQ("%t"), 1+1, EXP(CQ("\t")) },
	{ CQ("%T"), 8+1, EXP(CQ("23:01:13")) },
	{ CQ("%u"), 1+1, EXP(CQ("3")) },
	{ CQ("%U"), 2+1, EXP(CQ("26")) },
	{ CQ("%V"), 2+1, EXP(CQ("27")) },
	{ CQ("%v"), 11+1, EXP(CQ(" 2-Jul-2008")) },
	{ CQ("%w"), 1+1, EXP(CQ("3")) },
	{ CQ("%W"), 2+1, EXP(CQ("26")) },
	{ CQ("%x"), 8+1, EXP(CQ("07/02/08")) },
	{ CQ("%X"), 8+1, EXP(CQ("23:01:13")) },
	{ CQ("%y"), 2+1, EXP(CQ("08")) },
	{ CQ("%Y"), 4+1, EXP(CQ("2008")) },
	{ CQ("%z"), 5+1, EXP(CQ("-0400")) },
	{ CQ("%Z"), 3+1, EXP(CQ("EDT")) },
	{ CQ("%%"), 1+1, EXP(CQ("%")) },
	#undef EXP
	#define VEC(s)	s, sizeof(s)/sizeof(CHAR), sizeof(s)/sizeof(CHAR)-1, s
	#define EXP(s)	sizeof(s)/sizeof(CHAR), sizeof(s)/sizeof(CHAR)-1, s
	{ VEC(CQ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")) },
	{ CQ("0123456789%%%h:`~"), EXP(CQ("0123456789%Jul:`~")) },
	{ CQ("%R%h:`~ %x %w"), EXP(CQ("23:01Jul:`~ 07/02/08 3")) },
	#undef VEC
	#undef EXP
	};

#if YEAR_BASE == 1900  /* ( */
/* Checks for very large years.  YEAR_BASE value relied upon so that the
 * answer strings can be predetermined.
 * Years more than 4 digits are not mentioned in the standard for %C, so the
 * test for those cases are based on the design intent (which is to print the
 * whole number, being the century).  */
const struct tm  tmyr0 = {
	/* Wed Jul  2 23:01:13 EDT [HUGE#] */
	.tm_sec 	= 13,
	.tm_min 	= 1,
	.tm_hour	= 23,
	.tm_mday	= 2,
	.tm_mon 	= 6,
	.tm_year	= INT_MAX - YEAR_BASE/2,
	.tm_wday	= 3,
	.tm_yday	= 183,
	.tm_isdst	= 1
	};
#if INT_MAX == 32767
#  define YEAR	CQ("33717")		/* INT_MAX + YEAR_BASE/2 */
#  define CENT	CQ("337")
#  define Year	   CQ("17")
# elif INT_MAX == 2147483647
#  define YEAR	CQ("2147484597")
#  define CENT	CQ("21474845")
#  define Year	        CQ("97")
# elif INT_MAX == 9223372036854775807
#  define YEAR	CQ("9223372036854776757")
#  define CENT	CQ("92233720368547777")
#  define Year	                 CQ("57")
# else
#  error "Unrecognized INT_MAX value:  enhance me to recognize what you have"
#endif
const struct test  Vecyr0[] = {
	/* Testing fields one at a time, expecting to pass, using a larger
	 * allowed length than what is needed.  */
	/* Using tmyr0 for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%C"), OUTSIZE, EXP(CENT) },
	{ CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:13 ")YEAR) },
	{ CQ("%D"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
	{ CQ("%x"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%y"), OUTSIZE, EXP(Year) },
	{ CQ("%Y"), OUTSIZE, EXP(YEAR) },
	#undef EXP
	};
#undef YEAR
#undef CENT
#undef Year
/* Checks for very large negative years.  YEAR_BASE value relied upon so that
 * the answer strings can be predetermined.  */
const struct tm  tmyr1 = {
	/* Wed Jul  2 23:01:13 EDT [HUGE#] */
	.tm_sec 	= 13,
	.tm_min 	= 1,
	.tm_hour	= 23,
	.tm_mday	= 2,
	.tm_mon 	= 6,
	.tm_year	= INT_MIN,
	.tm_wday	= 3,
	.tm_yday	= 183,
	.tm_isdst	= 1
	};
#if INT_MAX == 32767
#  define YEAR	CQ("-30868")		/* INT_MIN + YEAR_BASE */
#  define CENT	CQ("-308")
#  define Year	    CQ("68")
# elif INT_MAX == 2147483647
#  define YEAR	CQ("-2147481748")
#  define CENT	CQ("-21474817")
#  define Year	         CQ("48")
# elif INT_MAX == 9223372036854775807
#  define YEAR	CQ("-9223372036854773908")
#  define CENT	CQ("-92233720368547739")
#  define Year	                  CQ("08")
# else
#  error "Unrecognized INT_MAX value:  enhance me to recognize what you have"
#endif
const struct test  Vecyr1[] = {
	/* Testing fields one at a time, expecting to pass, using a larger
	 * allowed length than what is needed.  */
	/* Using tmyr1 for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%C"), OUTSIZE, EXP(CENT) },
	{ CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:13 ")YEAR) },
	{ CQ("%D"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
	{ CQ("%x"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%y"), OUTSIZE, EXP(Year) },
	{ CQ("%Y"), OUTSIZE, EXP(YEAR) },
	#undef EXP
	};
#undef YEAR
#undef CENT
#undef Year
#endif /* YEAR_BASE ) */

/* Checks for years just over zero (also test for s=60).
 * Years less than 4 digits are not mentioned for %Y in the standard, so the
 * test for that case is based on the design intent.  */
const struct tm  tmyrzp = {
	/* Wed Jul  2 23:01:60 EDT 0007 */
	.tm_sec 	= 60,
	.tm_min 	= 1,
	.tm_hour	= 23,
	.tm_mday	= 2,
	.tm_mon 	= 6,
	.tm_year	= 7-YEAR_BASE,
	.tm_wday	= 3,
	.tm_yday	= 183,
	.tm_isdst	= 1
	};
#define YEAR	CQ("0007")	/* Design intent:  %Y=%C%y */
#define CENT	CQ("00")
#define Year	  CQ("07")
const struct test  Vecyrzp[] = {
	/* Testing fields one at a time, expecting to pass, using a larger
	 * allowed length than what is needed.  */
	/* Using tmyrzp for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%C"), OUTSIZE, EXP(CENT) },
	{ CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:60 ")YEAR) },
	{ CQ("%D"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
	{ CQ("%x"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%y"), OUTSIZE, EXP(Year) },
	{ CQ("%Y"), OUTSIZE, EXP(YEAR) },
	#undef EXP
	};
#undef YEAR
#undef CENT
#undef Year
/* Checks for years just under zero.
 * Negative years are not handled by the standard, so the vectors here are
 * verifying the chosen implemtation.  */
const struct tm  tmyrzn = {
	/* Wed Jul  2 23:01:00 EDT -004 */
	.tm_sec 	= 00,
	.tm_min 	= 1,
	.tm_hour	= 23,
	.tm_mday	= 2,
	.tm_mon 	= 6,
	.tm_year	= -4-YEAR_BASE,
	.tm_wday	= 3,
	.tm_yday	= 183,
	.tm_isdst	= 1
	};
#define YEAR	CQ("-004")
#define CENT	CQ("-0")
#define Year	  CQ("04")
const struct test  Vecyrzn[] = {
	/* Testing fields one at a time, expecting to pass, using a larger
	 * allowed length than what is needed.  */
	/* Using tmyrzn for time: */
	#define EXP(s)	sizeof(s)/sizeof(CHAR)-1, s
	{ CQ("%C"), OUTSIZE, EXP(CENT) },
	{ CQ("%c"), OUTSIZE, EXP(CQ("Wed Jul  2 23:01:00 ")YEAR) },
	{ CQ("%D"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%F"), OUTSIZE, EXP(YEAR CQ("-07-02")) },
	{ CQ("%v"), OUTSIZE, EXP(CQ(" 2-Jul-")YEAR) },
	{ CQ("%x"), OUTSIZE, EXP(CQ("07/02/")Year) },
	{ CQ("%y"), OUTSIZE, EXP(Year) },
	{ CQ("%Y"), OUTSIZE, EXP(YEAR) },
	#undef EXP
	};
#undef YEAR
#undef CENT
#undef Year

const struct list  ListYr[] = {
	{ &tmyrzp, Vecyrzp, sizeof(Vecyrzp)/sizeof(Vecyrzp[0]) },
	{ &tmyrzn, Vecyrzn, sizeof(Vecyrzn)/sizeof(Vecyrzn[0]) },
	#if YEAR_BASE == 1900
	{ &tmyr0, Vecyr0, sizeof(Vecyr0)/sizeof(Vecyr0[0]) },
	{ &tmyr1, Vecyr1, sizeof(Vecyr1)/sizeof(Vecyr1[0]) },
	#endif
	};


/* List of tests to be run */
const struct list  List[] = {
	{ &tm0, Vec0, sizeof(Vec0)/sizeof(Vec0[0]) },
	{ &tm1, Vec1, sizeof(Vec1)/sizeof(Vec1[0]) },
	};

#if defined(STUB_getenv_r)
char *
_getenv_r(struct _reent *p, const char *cp) { return getenv(cp); }
#endif

int
main(void)
{
int  i, l, errr=0, erro=0, tot=0;
const char  *cp;
CHAR  out[OUTSIZE];
size_t  ret;

/* Set timezone so that %z and %Z tests come out right */
cp = TZ;
if((i=putenv(cp)))  {
    printf( "putenv(%s) FAILED, ret %d\n", cp, i);
    return(-1);
    }
if(strcmp(getenv("TZ"),strchr(TZ,'=')+1))  {
    printf( "TZ not set properly in environment\n");
    return(-2);
    }
tzset();

#if defined(VERBOSE)
printf("_timezone=%d, _daylight=%d, _tzname[0]=%s, _tzname[1]=%s\n", _timezone, _daylight, _tzname[0], _tzname[1]);
{
long offset;
__tzinfo_type *tz = __gettzinfo ();
/* The sign of this is exactly opposite the envvar TZ.  We
   could directly use the global _timezone for tm_isdst==0,
   but have to use __tzrule for daylight savings.  */
printf("tz->__tzrule[0].offset=%d, tz->__tzrule[1].offset=%d\n", tz->__tzrule[0].offset, tz->__tzrule[1].offset);
}
#endif

/* Run all of the exact-length tests as-given--results should match */
for(l=0; l<sizeof(List)/sizeof(List[0]); l++)  {
    const struct list  *test = &List[l];
    for(i=0; i<test->cnt; i++)  {
	tot++;	/* Keep track of number of tests */
	ret = strftime(out, test->vec[i].max, test->vec[i].fmt, test->tms);
	if(ret != test->vec[i].ret)  {
	    errr++;
	    fprintf(stderr,
		"ERROR:  return %d != %d expected for List[%d].vec[%d]\n",
						ret, test->vec[i].ret, l, i);
	    }
	if(strncmp(out, test->vec[i].out, test->vec[i].max-1))  {
	    erro++;
	    fprintf(stderr,
		"ERROR:  \"%"SFLG"s\" != \"%"SFLG"s\" expected for List[%d].vec[%d]\n",
						out, test->vec[i].out, l, i);
	    }
	}
    }

/* Run all of the exact-length tests with the length made too short--expect to
 * fail.  */
for(l=0; l<sizeof(List)/sizeof(List[0]); l++)  {
    const struct list  *test = &List[l];
    for(i=0; i<test->cnt; i++)  {
	tot++;	/* Keep track of number of tests */
	ret = strftime(out, test->vec[i].max-1, test->vec[i].fmt, test->tms);
	if(ret != 0)  {
	    errr++;
	    fprintf(stderr,
		"ERROR:  return %d != %d expected for List[%d].vec[%d]\n",
						ret, 0, l, i);
	    }
	/* Almost every conversion puts out as many characters as possible, so
	 * go ahead and test the output even though have failed.  (The test
	 * times chosen happen to not hit any of the cases that fail this, so it
	 * works.)  */
	if(strncmp(out, test->vec[i].out, test->vec[i].max-1-1))  {
	    erro++;
	    fprintf(stderr,
		"ERROR:  \"%"SFLG"s\" != \"%"SFLG"s\" expected for List[%d].vec[%d]\n",
						out, test->vec[i].out, l, i);
	    }
	}
    }

/* Run all of the special year test cases */
for(l=0; l<sizeof(ListYr)/sizeof(ListYr[0]); l++)  {
    const struct list  *test = &ListYr[l];
    for(i=0; i<test->cnt; i++)  {
	tot++;	/* Keep track of number of tests */
	ret = strftime(out, test->vec[i].max, test->vec[i].fmt, test->tms);
	if(ret != test->vec[i].ret)  {
	    errr++;
	    fprintf(stderr,
		"ERROR:  return %d != %d expected for ListYr[%d].vec[%d]\n",
						ret, test->vec[i].ret, l, i);
	    }
	if(strncmp(out, test->vec[i].out, test->vec[i].max-1))  {
	    erro++;
	    fprintf(stderr,
		"ERROR:  \"%"SFLG"s\" != \"%"SFLG"s\" expected for ListYr[%d].vec[%d]\n",
						out, test->vec[i].out, l, i);
	    }
	}
    }

#define STRIZE(f)	#f
#define NAME(f)	STRIZE(f)
printf(NAME(strftime) "() test ");
if(errr || erro)  printf("FAILED %d/%d of", errr, erro);
  else    printf("passed");
printf(" %d test cases.\n", tot);

return(errr || erro);
}
#endif /* defined(_REGRESSION_TEST) ] */
