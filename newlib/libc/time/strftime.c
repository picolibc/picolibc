/*
 * strftime.c
 * Original Author:	G. Haley
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
<<strftime>>---flexible calendar time formatter

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
<[timp]>) into a string, starting at <[s]> and occupying no more than
<[maxsize]> characters.

You control the format of the output using the string at <[format]>.
<<*<[format]>>> can contain two kinds of specifications: text to be
copied literally into the formatted string, and time conversion
specifications.  Time conversion specifications are two-character
sequences beginning with `<<%>>' (use `<<%%>>' to include a percent
sign in the output).  Each defined conversion specification selects a
field of calendar time data from <<*<[timp]>>>, and converts it to a
string in one of the following ways:

o+
o %a
An abbreviation for the day of the week.

o %A
The full name for the day of the week.

o %b
An abbreviation for the month name.

o %B
The full name of the month.

o %c
A string representing the complete date and time, in the form
. Mon Apr 01 13:13:13 1992

o %d
The day of the month, formatted with two digits.

o %e
The day of the month, formatted with leading space if single digit.

o %H
The hour (on a 24-hour clock), formatted with two digits.

o %I
The hour (on a 12-hour clock), formatted with two digits.

o %j
The count of days in the year, formatted with three digits
(from `<<001>>' to `<<366>>').

o %m
The month number, formatted with two digits.

o %M
The minute, formatted with two digits.

o %p
Either `<<AM>>' or `<<PM>>' as appropriate.

o %S
The second, formatted with two digits.

o %U
The week number, formatted with two digits (from `<<00>>' to `<<53>>';
week number 1 is taken as beginning with the first Sunday in a year).
See also <<%W>>.

o %w
A single digit representing the day of the week: Sunday is day <<0>>.

o %W
Another version of the week number: like `<<%U>>', but counting week 1
as beginning with the first Monday in a year.

o
o %x
A string representing the complete date, in a format like
. Mon Apr 01 1992

o %X
A string representing the full time of day (hours, minutes, and
seconds), in a format like
. 13:13:13

o %y
The last two digits of the year.

o %Y
The full year, formatted with four digits to include the century.

o %Z
The time zone name.  If tm_isdst is -1, no output is generated.
Otherwise, the time zone name based on the TZ environment variable
is used.

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
<[maxsize]> characters.

<<strftime>> requires no supporting OS subroutines.
*/

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "local.h"

static _CONST int dname_len[7] =
{6, 6, 7, 9, 8, 6, 8};

static _CONST char *_CONST dname[7] =
{"Sunday", "Monday", "Tuesday", "Wednesday",
 "Thursday", "Friday", "Saturday"};

static _CONST int mname_len[12] =
{7, 8, 5, 5, 3, 4, 4, 6, 9, 7, 8, 8};

static _CONST char *_CONST mname[12] =
{"January", "February", "March", "April",
 "May", "June", "July", "August", "September", "October", "November",
 "December"};

size_t
_DEFUN (strftime, (s, maxsize, format, tim_p),
	char *s _AND
	size_t maxsize _AND
	_CONST char *format _AND
	_CONST struct tm *tim_p)
{
  size_t count = 0;
  int i;

  for (;;)
    {
      while (*format && *format != '%')
	{
	  if (count < maxsize - 1)
	    s[count++] = *format++;
	  else
	    return 0;
	}

      if (*format == '\0')
	break;

      format++;
      switch (*format)
	{
	case 'a':
	  for (i = 0; i < 3; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] =
		  dname[tim_p->tm_wday][i];
	      else
		return 0;
	    }
	  break;
	case 'A':
	  for (i = 0; i < dname_len[tim_p->tm_wday]; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] =
		  dname[tim_p->tm_wday][i];
	      else
		return 0;
	    }
	  break;
	case 'b':
	case 'h':
	  for (i = 0; i < 3; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] =
		  mname[tim_p->tm_mon][i];
	      else
		return 0;
	    }
	  break;
	case 'B':
	  for (i = 0; i < mname_len[tim_p->tm_mon]; i++)
	    {
	      if (count < maxsize - 1)
		s[count++] =
		  mname[tim_p->tm_mon][i];
	      else
		return 0;
	    }
	  break;
	case 'c':
	  if (count < maxsize - 24)
	    {
	      for (i = 0; i < 3; i++)
		s[count++] =
		  dname[tim_p->tm_wday][i];
	      s[count++] = ' ';
	      for (i = 0; i < 3; i++)
		s[count++] =
		  mname[tim_p->tm_mon][i];

	      sprintf (&s[count],
		       " %.2d %2.2d:%2.2d:%2.2d %.4d",
		       tim_p->tm_mday, tim_p->tm_hour,
		       tim_p->tm_min,
		       tim_p->tm_sec, 1900 +
		       tim_p->tm_year);
	      count += 17;
	    }
	  else
	    return 0;
	  break;
	case 'd':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%.2d",
		       tim_p->tm_mday);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'e':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%2d",
		       tim_p->tm_mday);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'H':
	case 'k':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], *format == 'k' ? "%2d" : "%2.2d",
		       tim_p->tm_hour);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'I':
	case 'l':
	  if (count < maxsize - 2)
	    {
	      if (tim_p->tm_hour == 0 ||
		  tim_p->tm_hour == 12)
		{
		  s[count++] = '1';
		  s[count++] = '2';
		}
	      else
		{
		  sprintf (&s[count], (*format == 'I') ? "%.2d" : "%2d",
			   tim_p->tm_hour % 12);
		  count += 2;
		}
	    }
	  else
	    return 0;
	  break;
	case 'j':
	  if (count < maxsize - 3)
	    {
	      sprintf (&s[count], "%.3d",
		       tim_p->tm_yday + 1);
	      count += 3;
	    }
	  else
	    return 0;
	  break;
	case 'm':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%.2d",
		       tim_p->tm_mon + 1);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'M':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%2.2d",
		       tim_p->tm_min);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'p':
	  if (count < maxsize - 2)
	    {
	      if (tim_p->tm_hour < 12)
		s[count++] = 'A';
	      else
		s[count++] = 'P';

	      s[count++] = 'M';
	    }
	  else
	    return 0;
	  break;
	case 'S':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%2.2d",
		       tim_p->tm_sec);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'U':
	  if (count < maxsize - 2)
	    {
	      sprintf (&s[count], "%2.2d",
		       (tim_p->tm_yday + 7 -
			tim_p->tm_wday) / 7);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'w':
	  if (count < maxsize - 1)
	    {
	      sprintf (&s[count], "%1.1d",
		       tim_p->tm_wday);
	      count++;
	    }
	  else
	    return 0;
	  break;
	case 'W':
	  if (count < maxsize - 2)
	    {
	      int wday = (tim_p->tm_wday) ? tim_p->tm_wday - 1 : 6;
	      sprintf (&s[count], "%2.2d",
		       (tim_p->tm_yday + 7 -
			wday) / 7);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'x':
	  if (count < maxsize - 15)
	    {
	      for (i = 0; i < 3; i++)
		s[count++] =
		  dname[tim_p->tm_wday][i];
	      s[count++] = ' ';
	      for (i = 0; i < 3; i++)
		s[count++] =
		  mname[tim_p->tm_mon][i];

	      sprintf (&s[count],
		       " %.2d %.4d", tim_p->tm_mday,
		       1900 + tim_p->tm_year);
	      count += 8;
	    }
	  else
	    return 0;
	  break;
	case 'X':
	  if (count < maxsize - 8)
	    {
	      sprintf (&s[count],
		       "%2.2d:%2.2d:%2.2d",
		       tim_p->tm_hour, tim_p->tm_min,
		       tim_p->tm_sec);
	      count += 8;
	    }
	  else
	    return 0;
	  break;
	case 'y':
	  if (count < maxsize - 2)
	    {
	      /* The year could be greater than 100, so we need the value
		 modulo 100.  The year could be negative, so we need to
		 correct for a possible negative remainder.  */
	      sprintf (&s[count], "%2.2d",
		       (tim_p->tm_year % 100 + 100) % 100);
	      count += 2;
	    }
	  else
	    return 0;
	  break;
	case 'Y':
	  if (count < maxsize - 4)
	    {
	      sprintf (&s[count], "%.4d",
		       1900 + tim_p->tm_year);
	      count += 4;
	    }
	  else
	    return 0;
	  break;
	case 'Z':
	  if (tim_p->tm_isdst >= 0)
	    {
	      int size;
	      TZ_LOCK;
	      size = strlen(_tzname[tim_p->tm_isdst]);
	      for (i = 0; i < size; i++)
		{
		  if (count < maxsize - 1)
		    s[count++] = _tzname[tim_p->tm_isdst][i];
		  else
		    {
		      TZ_UNLOCK;
		      return 0;
		    }
		}
	      TZ_UNLOCK;
	    }
	  break;
	case '%':
	  if (count < maxsize - 1)
	    s[count++] = '%';
	  else
	    return 0;
	  break;
	}
      if (*format)
	format++;
      else
	break;
    }
  s[count] = '\0';

  return count;
}
