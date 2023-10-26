/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<ecvtbuf>>, <<fcvtbuf>>---double or float to string

INDEX
	ecvtbuf
INDEX
	fcvtbuf

SYNOPSIS
	#include <stdio.h>

	char *ecvtbuf(double <[val]>, int <[chars]>, int *<[decpt]>,
                       int *<[sgn]>, char *<[buf]>);

	char *fcvtbuf(double <[val]>, int <[decimals]>, int *<[decpt]>,
                       int *<[sgn]>, char *<[buf]>);

DESCRIPTION
	<<ecvtbuf>> and <<fcvtbuf>> produce (null-terminated) strings
	of digits representating the <<double>> number <[val]>.

	The only difference between <<ecvtbuf>> and <<fcvtbuf>> is the
	interpretation of the second argument (<[chars]> or
	<[decimals]>). For <<ecvtbuf>>, the second argument <[chars]>
	specifies the total number of characters to write (which is
	also the number of significant digits in the formatted string,
	since these two functions write only digits). For <<fcvtbuf>>,
	the second argument <[decimals]> specifies the number of
	characters to write after the decimal point; all digits for
	the integer part of <[val]> are always included.

	Since <<ecvtbuf>> and <<fcvtbuf>> write only digits in the
	output string, they record the location of the decimal point
	in <<*<[decpt]>>>, and the sign of the number in <<*<[sgn]>>>.
	After formatting a number, <<*<[decpt]>>> contains the number
	of digits to the left of the decimal point.  <<*<[sgn]>>>
	contains <<0>> if the number is positive, and <<1>> if it is
	negative.  For both functions, you supply a pointer <[buf]> to
	an area of memory to hold the converted string.

RETURNS
	Both functions return a pointer to <[buf]>, the string
	containing a character representation of <[val]>.

PORTABILITY
	Neither function is ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdlib.h>
#include <string.h>
#include "mprec.h"
#include "local.h"
#include "atexit.h"

static void
print_f (
	char *buf,
	double invalue,
	int ndigit,
	char type,
	int dot,
	int mode)
{
  int decpt;
  int sign;
  char *p, *start, *end;

  (void) type; /* XXX should be used */
  start = p = __dtoa (invalue, mode, ndigit, &decpt, &sign, &end);
  if (!p) {
    buf[0] = '\0';
    return;
  }

  if (decpt == 9999)
    {
      strcpy (buf, p);
      return;
    }
  while (*p && decpt > 0)
    {
      *buf++ = *p++;
      decpt--;
    }
  /* Even if not in buffer */
  while (decpt > 0)
    {
      *buf++ = '0';
      decpt--;
    }

  if (dot || *p)
    {
      if (p == start)
	*buf++ = '0';
      if (decpt < 0 && ndigit > 0)
	*buf++ = '.';
      while (decpt < 0 && ndigit > 0)
	{
	  *buf++ = '0';
	  decpt++;
	  ndigit--;
	}

      /* Print rest of stuff */
      while (*p && ndigit > 0)
	{
	  *buf++ = *p++;
	  ndigit--;
	}
      /* And trailing zeros */
      while (ndigit > 0)
	{
	  *buf++ = '0';
	  ndigit--;
	}
    }
  *buf++ = 0;
}

/* Print number in e format with width chars after.

   TYPE is one of 'e' or 'E'.  It may also be one of 'g' or 'G' indicating
   that _gcvt is calling us and we should remove trailing zeroes.

   WIDTH is the number of digits of precision after the decimal point.  */

static void
print_e (
	char *buf,
	double invalue,
	int width,
	char type,
	int dot)
{
  int sign;
  char *end;
  char *p;
  int decpt;
  int top;
  int ndigit = width;

  p = __dtoa (invalue, 2, width + 1, &decpt, &sign, &end);
  if (!p) {
    buf[0] = '\0';
    return;
  }

  if (decpt == 9999)
    {
      strcpy (buf, p);
      return;
    }

  *buf++ = *p++;
  if (ndigit > 0)
    dot = 1;

  while (*p && ndigit > 0)
    {
      if (dot) {
	*buf++ = '.';
	dot = 0;
      }
      *buf++ = *p++;
      ndigit--;
    }

  /* Add trailing zeroes to fill out to ndigits unless this is 'g' format.
     Also, convert g/G to e/E.  */

  if (type == 'g')
    type = 'e';
  else if (type == 'G')
    type = 'E';
  else
    {
      while (ndigit > 0)
	{
	  if  (dot) {
	    *buf++ = '.';
	    dot = 0;
	  }
	  *buf++ = '0';
	  ndigit--;
	}
    }

  /* Add the exponent.  */

  *buf++ = type;
  decpt--;
  if (decpt < 0)
    {
      *buf++ = '-';
      decpt = -decpt;
    }
  else
    {
      *buf++ = '+';
    }
  if (decpt > 99)
    {
      int top = decpt / 100;
      *buf++ = top + '0';
      decpt -= top * 100;
    }
  top = decpt / 10;
  *buf++ = top + '0';
  decpt -= top * 10;
  *buf++ = decpt + '0';

  *buf++ = 0;
}

#ifndef _REENT_ONLY

static NEWLIB_THREAD_LOCAL int _cvtlen;
static NEWLIB_THREAD_LOCAL char *_cvtbuf;

static void
_cvtcleanup(void)
{
  if (_cvtbuf) {
    free(_cvtbuf);
    _cvtbuf = NULL;
  }
}

/* Undocumented behaviour: when given NULL as a buffer, return a
   pointer to static space in the rent structure.  This is only to
   support ecvt and fcvt, which aren't ANSI anyway.  */

char *
fcvtbuf (double invalue,
	int ndigit,
	int *decpt,
	int *sign,
	char *fcvt_buf)
{
  char *save;
  char *p;
  char *end;
  int done = 0;

  if (fcvt_buf == NULL)
    {
      if (_cvtlen <= ndigit + 35)
	{
	  if  (!_cvtbuf)
	    if (atexit(_cvtcleanup) != 0)
	      return NULL;
	  if ((fcvt_buf = (char *) realloc (_cvtbuf,
					    ndigit + 36)) == NULL)
	    return NULL;
	  _cvtlen = ndigit + 36;
	  _cvtbuf = fcvt_buf;
	}

      fcvt_buf = _cvtbuf ;
    }

  save = fcvt_buf;

  p = __dtoa (invalue, 3, ndigit, decpt, sign, &end);
  if (!p)
    return NULL;

  if (*decpt == 9999)
    {
      strcpy(fcvt_buf, p);
      return fcvt_buf;
    }

  /* Now copy */

  done = -*decpt;

  while (p < end)
    {
      *fcvt_buf++ = *p++;
      done++;
    }
  /* And unsuppress the trailing zeroes */
  while (done < ndigit)
    {
      *fcvt_buf++ = '0';
      done++;
    }
  *fcvt_buf++ = 0;
  return save;
}

char *
ecvtbuf (double invalue,
	int ndigit,
	int *decpt,
	int *sign,
	char *fcvt_buf)
{
  char *save;
  char *p;
  char *end;
  int done = 0;

  if (fcvt_buf == NULL)
    {
      if (_cvtlen <= ndigit)
	{
	  if ((fcvt_buf = (char *) realloc (_cvtbuf,
					    ndigit + 1)) == NULL)
	    return NULL;
	  _cvtlen = ndigit + 1;
	  _cvtbuf = fcvt_buf;
	}

      fcvt_buf = _cvtbuf ;
    }

  save = fcvt_buf;

  p = __dtoa (invalue, 2, ndigit, decpt, sign, &end);
  if (!p)
    return NULL;

  if (*decpt == 9999)
    {
      strcpy(fcvt_buf, p);
      return fcvt_buf;
    }

  /* Now copy */

  while (p < end)
    {
      *fcvt_buf++ = *p++;
      done++;
    }
  /* And unsuppress the trailing zeroes */
  while (done < ndigit)
    {
      *fcvt_buf++ = '0';
      done++;
    }
  *fcvt_buf++ = 0;
  return save;
}

#endif

char *
_gcvt (
	double invalue,
	int ndigit,
	char *buf,
	char type,
	int dot)
{
  char *save = buf;

  if (invalue < 0)
    {
      invalue = -invalue;
    }

  if (invalue == 0)
    {
      *buf++ = '0';
      *buf = '\0';
    }
  else
    /* Which one to print ?
       ANSI says that anything with more that 4 zeros after the . or more
       than precision digits before is printed in e with the qualification
       that trailing zeroes are removed from the fraction portion.  */

  if (0.0001 >= invalue || invalue >= _mprec_log10 (ndigit))
    {
      /* We subtract 1 from ndigit because in the 'e' format the precision is
	 the number of digits after the . but in 'g' format it is the number
	 of significant digits.

	 We defer changing type to e/E so that print_e() can know it's us
	 calling and thus should remove trailing zeroes.  */

      print_e (buf, invalue, ndigit - 1, type, dot);
    }
  else
    {
      int decpt;
      int sign;
      char *end;
      char *p;

      /* We always want ndigits of precision, even if that means printing
       * a bunch of leading zeros for numbers < 1.0
       */
      p = __dtoa (invalue, 2, ndigit, &decpt, &sign, &end);
      if (!p)
	return NULL;

      if (decpt == 9999)
	{
	  strcpy (buf, p);
	  return save;
	}
      while (*p && decpt > 0)
	{
	  *buf++ = *p++;
	  decpt--;
	  ndigit--;
	}
      /* Even if not in buffer */
      while (decpt > 0 && ndigit > 0)
	{
	  *buf++ = '0';
	  decpt--;
	  ndigit--;
	}

      if (dot || *p)
	{
	  if (buf == save)
	    *buf++ = '0';
	  *buf++ = '.';

	  /* Leading zeros don't count towards 'ndigit' */
	  while (decpt < 0)
	    {
	      *buf++ = '0';
	      decpt++;
	    }

	  /* Print rest of stuff */
	  while (*p && ndigit > 0)
	    {
	      *buf++ = *p++;
	      ndigit--;
	    }
	  /* And trailing zeros */
	  if (dot)
	    {
	      while (ndigit > 0)
		{
		  *buf++ = '0';
		  ndigit--;
		}
	    }
	}
      *buf++ = 0;
    }

  return save;
}

char *
_dcvt (
	char *buffer,
	double invalue,
	int precision,
	int width,
	char type,
	int dot)
{
  (void) width; /* XXX should be used */
  switch (type)
    {
    case 'f':
    case 'F':
      print_f (buffer, invalue, precision, type, precision == 0 ? dot : 1, 3);
      break;
    case 'g':
    case 'G':
      if (precision == 0)
	precision = 1;
      _gcvt (invalue, precision, buffer, type, dot);
      break;
    case 'e':
    case 'E':
      print_e (buffer, invalue, precision, type, dot);
    }
  return buffer;
}
