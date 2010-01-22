/* NOTE:  This file defines both strftime() and wcsftime().  Take care when
 * making changes.  See also wcsftime.c, and note the (small) overlap in the
 * manual description, taking care to edit both as needed.  */
/*
 * strftime.c
 * Original Author:	G. Haley
 * Additions from:	Eric Blake
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
<<strftime>>---convert date and time to a formatted string

INDEX
	strftime

ANSI_SYNOPSIS
	#include <time.h>
	size_t strftime(char *<[s]>, size_t <[maxsize]>,
			const char *<[format]>, const struct tm *<[timp]>);

TRAD_SYNOPSIS
	#include <time.h>
	size_t strftime(<[s]>, <[maxsize]>, <[format]>, <[timp]>)
	char *<[s]>;
	size_t <[maxsize]>;
	char *<[format]>;
	struct tm *<[timp]>;

DESCRIPTION
<<strftime>> converts a <<struct tm>> representation of the time (at
<[timp]>) into a null-terminated string, starting at <[s]> and occupying
no more than <[maxsize]> characters.

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
`<<Wednesday>>', `<Thursday>>', `<<Friday>>', `<<Saturday>>'. [tm_wday]

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

o %r
Replaced by the time in a.m. and p.m. notation.  In the "C" locale this
is equivalent to "%I:%M:%S %p".  In locales which don't define a.m./p.m.
notations, the result is an empty string. [tm_sec, tm_min, tm_hour]

o %R
The 24-hour time, to the minute.  Equivalent to "%H:%M". [tm_min, tm_hour]

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
The time zone name.  If tm_isdst is negative, no output is generated.
Otherwise, the time zone name is based on the TZ environment variable,
as if by calling tzset(). [tm_isdst]

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

<<strftime>> requires no supporting OS subroutines.

BUGS
<<strftime>> ignores the LC_TIME category of the current locale, hard-coding
the "C" locale settings.
*/

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <wctype.h>
#include "local.h"
#include "../locale/timelocal.h"
 
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
#  define _ctloc(x) (ctloclen = strlen (ctloc = _CurrentTimeLocale->x), ctloc)
#  define TOLOWER(c)	tolower((int)(unsigned char)(c))
# else
#  define strftime	wcsftime	/* Alternate function name */
#  define CHAR		wchar_t		/* string type basis */
#  define CQ(a)		L##a		/* character constant qualifier */
#  define snprintf	swprintf	/* wide-char equivalent function name */
#  define strncmp	wcsncmp		/* wide-char equivalent function name */
#  define TOLOWER(c)	towlower((wint_t)(c))
#  define SFLG		"l"		/* %s flag (l for wide char) */
#  define CTLOCBUFLEN   256		/* Arbitrary big buffer size */
   const wchar_t *
   __ctloc (wchar_t *buf, const char *elem, size_t *len_ret)
   {
     buf[CTLOCBUFLEN - 1] = L'\0';
     *len_ret = mbstowcs (buf, elem, CTLOCBUFLEN - 1);
     if (*len_ret == (size_t) -1 )
       *len_ret = 0;
     return buf;
   }
#  define _ctloc(x) (ctloc = __ctloc (ctlocbuf, _CurrentTimeLocale->x, \
		     &ctloclen))
#endif  /* MAKE_WCSFTIME */

/* Enforce the coding assumptions that YEAR_BASE is positive.  (%C, %Y, etc.) */
#if YEAR_BASE < 0
#  error "YEAR_BASE < 0"
#endif

static _CONST int dname_len[7] =
{6, 6, 7, 9, 8, 6, 8};

/* Using the tm_year, tm_wday, and tm_yday components of TIM_P, return
   -1, 0, or 1 as the adjustment to add to the year for the ISO week
   numbering used in "%g%G%V", avoiding overflow.  */
static int
_DEFUN (iso_year_adjust, (tim_p),
	_CONST struct tm *tim_p)
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

size_t
_DEFUN (strftime, (s, maxsize, format, tim_p),
	CHAR *s _AND
	size_t maxsize _AND
	_CONST CHAR *format _AND
	_CONST struct tm *tim_p)
{
  size_t count = 0;
  int i, len;
  const CHAR *ctloc;
#ifdef MAKE_WCSFTIME
  CHAR ctlocbuf[CTLOCBUFLEN];
#endif
  size_t ctloclen;

  struct lc_time_T *_CurrentTimeLocale = __get_current_time_locale ();
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
      if (*format == CQ('E') || *format == CQ('O'))
	format++;

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
	  _ctloc (c_fmt);
	  goto recurse;
	case CQ('r'):
	  _ctloc (ampm_fmt);
	  goto recurse;
	case CQ('x'):
	  _ctloc (x_fmt);
	  goto recurse;
	case CQ('X'):
	  _ctloc (X_fmt);
recurse:
	  if (*ctloc)
	    {
	      /* Recurse to avoid need to replicate %Y formation. */
	      size_t adjust = strftime (&s[count], maxsize - count, ctloc,
					tim_p);
	      if (adjust > 0)
		count += adjust;
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
	    int neg = tim_p->tm_year < -YEAR_BASE;
	    int century = tim_p->tm_year >= 0
	      ? tim_p->tm_year / 100 + YEAR_BASE / 100
	      : abs (tim_p->tm_year + YEAR_BASE) / 100;
            len = snprintf (&s[count], maxsize - count, CQ("%s%.*d"),
                               neg ? CQ("-") : CQ(""), 2 - neg, century);
            if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  }
	  break;
	case CQ('d'):
	case CQ('e'):
	  len = snprintf (&s[count], maxsize - count,
			*format == CQ('d') ? CQ("%.2d") : CQ("%2d"),
			tim_p->tm_mday);
	  if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('D'):
	  /* %m/%d/%y */
	  len = snprintf (&s[count], maxsize - count,
			CQ("%.2d/%.2d/%.2d"),
			tim_p->tm_mon + 1, tim_p->tm_mday,
			tim_p->tm_year >= 0 ? tim_p->tm_year % 100
			: abs (tim_p->tm_year + YEAR_BASE) % 100);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('F'):
	  { /* %F is equivalent to "%Y-%m-%d" */
	    /* Recurse to avoid need to replicate %Y formation. */
	    size_t adjust = strftime (&s[count], maxsize - count,
				      CQ("%Y-%m-%d"), tim_p);
	    if (adjust > 0)
	      count += adjust;
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
            if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  }
          break;
	case CQ('G'):
	  {
	    /* See the comments for 'C' and 'Y'; this is a variable length
	       field.  Although there is no requirement for a minimum number
	       of digits, we use 4 for consistency with 'Y'.  */
	    int neg = tim_p->tm_year < -YEAR_BASE;
	    int adjust = iso_year_adjust (tim_p);
	    int century = tim_p->tm_year >= 0
	      ? tim_p->tm_year / 100 + YEAR_BASE / 100
	      : abs (tim_p->tm_year + YEAR_BASE) / 100;
	    int year = tim_p->tm_year >= 0 ? tim_p->tm_year % 100
	      : abs (tim_p->tm_year + YEAR_BASE) % 100;
	    if (adjust < 0 && tim_p->tm_year <= -YEAR_BASE)
	      neg = adjust = 1;
	    else if (adjust > 0 && neg)
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
            len = snprintf (&s[count], maxsize - count, CQ("%s%.*d%.2d"),
                               neg ? CQ("-") : CQ(""), 2 - neg, century, year);
            if (len < 0  ||  (count+=len) >= maxsize)
              return 0;
	  }
          break;
	case CQ('H'):
	case CQ('k'):	/* newlib extension */
	  len = snprintf (&s[count], maxsize - count,
			*format == CQ('k') ? CQ("%2d") : CQ("%.2d"),
			tim_p->tm_hour);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('I'):
	case CQ('l'):	/* newlib extension */
	  {
	    register int  h12;
	    h12 = (tim_p->tm_hour == 0 || tim_p->tm_hour == 12)  ?
						12  :  tim_p->tm_hour % 12;
	    len = snprintf (&s[count], maxsize - count,
			*format == CQ('I') ? CQ("%.2d") : CQ("%2d"),
			h12);
	    if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  }
	  break;
	case CQ('j'):
	  len = snprintf (&s[count], maxsize - count, CQ("%.3d"),
			tim_p->tm_yday + 1);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('m'):
	  len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			tim_p->tm_mon + 1);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('M'):
	  len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			tim_p->tm_min);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
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
	case CQ('R'):
          len = snprintf (&s[count], maxsize - count, CQ("%.2d:%.2d"),
			tim_p->tm_hour, tim_p->tm_min);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
          break;
	case CQ('S'):
	  len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			tim_p->tm_sec);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
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
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
          break;
	case CQ('u'):
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
	  len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
		       (tim_p->tm_yday + 7 -
			tim_p->tm_wday) / 7);
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
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
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"), week);
            if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  }
          break;
	case CQ('w'):
	  if (count < maxsize - 1)
            s[count++] = CQ('0') + tim_p->tm_wday;
	  else
	    return 0;
	  break;
	case CQ('W'):
	  {
	    int wday = (tim_p->tm_wday) ? tim_p->tm_wday - 1 : 6;
	    len = snprintf (&s[count], maxsize - count, CQ("%.2d"),
			(tim_p->tm_yday + 7 - wday) / 7);
            if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  }
	  break;
	case CQ('y'):
	    {
	      /* Be careful of both overflow and negative years, thanks to
		 the asymmetric range of years.  */
	      int year = tim_p->tm_year >= 0 ? tim_p->tm_year % 100
		: abs (tim_p->tm_year + YEAR_BASE) % 100;
	      len = snprintf (&s[count], maxsize - count, CQ("%.2d"), year);
              if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	    }
	  break;
	case CQ('Y'):
	  /* An implementation choice is to have %Y match %C%y, so that it
	   * gives at least 4 digits, with leading zeros as needed.  */
	  if(tim_p->tm_year <= INT_MAX-YEAR_BASE)  {
	    /* For normal, non-overflow case.  */
	    len = snprintf (&s[count], maxsize - count, CQ("%04d"),
				tim_p->tm_year + YEAR_BASE);
	  }
	  else  {
	    /* int would overflow, so use unsigned instead.  */
	    register unsigned year;
	    year = (unsigned) tim_p->tm_year + (unsigned) YEAR_BASE;
	    len = snprintf (&s[count], maxsize - count, CQ("%04u"),
				tim_p->tm_year + YEAR_BASE);
	  }
          if (len < 0  ||  (count+=len) >= maxsize)  return 0;
	  break;
	case CQ('z'):
          if (tim_p->tm_isdst >= 0)
            {
	      long offset;
	      __tzinfo_type *tz = __gettzinfo ();
	      TZ_LOCK;
	      /* The sign of this is exactly opposite the envvar TZ.  We
	         could directly use the global _timezone for tm_isdst==0,
	         but have to use __tzrule for daylight savings.  */
	      offset = -tz->__tzrule[tim_p->tm_isdst > 0].offset;
	      TZ_UNLOCK;
	      len = snprintf (&s[count], maxsize - count, CQ("%+03ld%.2ld"),
			offset / SECSPERHOUR,
			labs (offset / SECSPERMIN) % 60L);
              if (len < 0  ||  (count+=len) >= maxsize)  return 0;
            }
          break;
	case CQ('Z'):
	  if (tim_p->tm_isdst >= 0)
	    {
	      int size;
	      TZ_LOCK;
	      size = strlen(_tzname[tim_p->tm_isdst > 0]);
	      for (i = 0; i < size; i++)
		{
		  if (count < maxsize - 1)
		    s[count++] = _tzname[tim_p->tm_isdst > 0][i];
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
	{ CQ("%r"), 11+1, EXP(CQ("09:53:47 AM")) },
	{ CQ("%R"), 5+1, EXP(CQ("09:53")) },
	{ CQ("%S"), 2+1, EXP(CQ("47")) },
	{ CQ("%t"), 1+1, EXP(CQ("\t")) },
	{ CQ("%T"), 8+1, EXP(CQ("09:53:47")) },
	{ CQ("%u"), 1+1, EXP(CQ("2")) },
	{ CQ("%U"), 2+1, EXP(CQ("52")) },
	{ CQ("%V"), 2+1, EXP(CQ("01")) },
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
	{ CQ("%r"), 11+1, EXP(CQ("11:01:13 PM")) },
	{ CQ("%R"), 5+1, EXP(CQ("23:01")) },
	{ CQ("%S"), 2+1, EXP(CQ("13")) },
	{ CQ("%t"), 1+1, EXP(CQ("\t")) },
	{ CQ("%T"), 8+1, EXP(CQ("23:01:13")) },
	{ CQ("%u"), 1+1, EXP(CQ("3")) },
	{ CQ("%U"), 2+1, EXP(CQ("26")) },
	{ CQ("%V"), 2+1, EXP(CQ("27")) },
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
