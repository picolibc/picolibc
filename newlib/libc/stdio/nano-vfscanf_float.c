/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <_ansi.h>
#include <reent.h>
#include <newlib.h>
#include <ctype.h>
#include <wctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "local.h"
#include "../stdlib/local.h"
#include "nano-vfscanf_local.h"

#ifdef FLOATING_POINT
int
_scanf_float (struct _reent *rptr,
	      struct _scan_data_t *pdata,
	      FILE *fp, va_list *ap)
{
  int c;
  char *p;
  float *flp;
  _LONG_DOUBLE *ldp;

  /* Scan a floating point number as if by strtod.  */
  /* This code used to assume that the number of digits is reasonable.
     However, ANSI / ISO C makes no such stipulation; we have to get
     exact results even when there is an unreasonable amount of leading
     zeroes.  */
  long leading_zeroes = 0;
  long zeroes, exp_adjust;
  char *exp_start = NULL;
  unsigned width_left = 0;
  char nancount = 0;
  char infcount = 0;
#ifdef hardway
  if (pdata->width == 0 || pdata->width > BUF - 1)
#else
  /* size_t is unsigned, hence this optimisation.  */
  if (pdata->width - 1 > BUF - 2)
#endif
    {
      width_left = pdata->width - (BUF - 1);
      pdata->width = BUF - 1;
    }
  pdata->flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
  zeroes = 0;
  exp_adjust = 0;
  for (p = pdata->buf; pdata->width; )
    {
      c = *fp->_p;
      /* This code mimicks the integer conversion code,
	 but is much simpler.  */
      switch (c)
	{
	case '0':
	  if (pdata->flags & NDIGITS)
	    {
	      pdata->flags &= ~SIGNOK;
	      zeroes++;
	      if (width_left)
		{
		  width_left--;
		  pdata->width++;
		}
	      goto fskip;
	    }
	/* Fall through.  */
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  if (nancount + infcount == 0)
	    {
	      pdata->flags &= ~(SIGNOK | NDIGITS);
	      goto fok;
	    }
	  break;

	case '+':
	case '-':
	  if (pdata->flags & SIGNOK)
	    {
	      pdata->flags &= ~SIGNOK;
	      goto fok;
	    }
	  break;
	case 'n':
	case 'N':
	  if (nancount == 0 && zeroes == 0
	      && (pdata->flags & (NDIGITS | DPTOK | EXPOK)) ==
				  (NDIGITS | DPTOK | EXPOK))
	    {
	      pdata->flags &= ~(SIGNOK | DPTOK | EXPOK | NDIGITS);
	      nancount = 1;
	      goto fok;
	    }
	  if (nancount == 2)
	    {
	      nancount = 3;
	      goto fok;
	    }
	  if (infcount == 1 || infcount == 4)
	    {
	      infcount++;
	      goto fok;
	    }
	  break;
	case 'a':
	case 'A':
	  if (nancount == 1)
	    {
	      nancount = 2;
	      goto fok;
	    }
	  break;
	case 'i':
	case 'I':
	  if (infcount == 0 && zeroes == 0
	      && (pdata->flags & (NDIGITS | DPTOK | EXPOK)) ==
				  (NDIGITS | DPTOK | EXPOK))
	    {
	      pdata->flags &= ~(SIGNOK | DPTOK | EXPOK | NDIGITS);
	      infcount = 1;
	      goto fok;
	    }
	  if (infcount == 3 || infcount == 5)
	    {
	      infcount++;
	      goto fok;
	    }
	  break;
	case 'f':
	case 'F':
	  if (infcount == 2)
	    {
	      infcount = 3;
	      goto fok;
	    }
	  break;
	case 't':
	case 'T':
	  if (infcount == 6)
	    {
	      infcount = 7;
	      goto fok;
	    }
	  break;
	case 'y':
	case 'Y':
	  if (infcount == 7)
	    {
	      infcount = 8;
	      goto fok;
	    }
	  break;
	case '.':
	  if (pdata->flags & DPTOK)
	    {
	      pdata->flags &= ~(SIGNOK | DPTOK);
	      leading_zeroes = zeroes;
	      goto fok;
	    }
	  break;
	case 'e':
	case 'E':
	  /* No exponent without some digits.  */
	  if ((pdata->flags & (NDIGITS | EXPOK)) == EXPOK
	      || ((pdata->flags & EXPOK) && zeroes))
	    {
	      if (! (pdata->flags & DPTOK))
		{
		  exp_adjust = zeroes - leading_zeroes;
		  exp_start = p;
	        }
	      pdata->flags =
		(pdata->flags & ~(EXPOK | DPTOK)) | SIGNOK | NDIGITS;
	      zeroes = 0;
	      goto fok;
	    }
	  break;
	}
      break;
fok:
      *p++ = c;
fskip:
      pdata->width--;
      ++pdata->nread;
      if (--fp->_r > 0)
	fp->_p++;
      else if (pdata->pfn_refill (rptr, fp))
	/* "EOF".  */
	break;
    }
  if (zeroes)
    pdata->flags &= ~NDIGITS;
  /* We may have a 'N' or possibly even [sign] 'N' 'a' as the
     start of 'NaN', only to run out of chars before it was
     complete (or having encountered a non-matching char).  So
     check here if we have an outstanding nancount, and if so
     put back the chars we did swallow and treat as a failed
     match.

     FIXME - we still don't handle NAN([0xdigits]).  */
  if (nancount - 1U < 2U)
    {
      /* "nancount && nancount < 3".  */
      /* Newlib's ungetc works even if we called __srefill in
	 the middle of a partial parse, but POSIX does not
	 guarantee that in all implementations of ungetc.  */
      while (p > pdata->buf)
	{
	  pdata->pfn_ungetc (rptr, *--p, fp); /* "[-+nNaA]".  */
	  --pdata->nread;
	}
      return MATCH_FAILURE;
    }
  /* Likewise for 'inf' and 'infinity'.	 But be careful that
     'infinite' consumes only 3 characters, leaving the stream
     at the second 'i'.	 */
  if (infcount - 1U < 7U)
    {
      /* "infcount && infcount < 8".  */
      if (infcount >= 3) /* valid 'inf', but short of 'infinity'.  */
	while (infcount-- > 3)
	  {
	    pdata->pfn_ungetc (rptr, *--p, fp); /* "[iInNtT]".  */
	    --pdata->nread;
	  }
      else
        {
	  while (p > pdata->buf)
	    {
	      pdata->pfn_ungetc (rptr, *--p, fp); /* "[-+iInN]".  */
	      --pdata->nread;
	    }
	  return MATCH_FAILURE;
	}
    }
  /* If no digits, might be missing exponent digits
     (just give back the exponent) or might be missing
     regular digits, but had sign and/or decimal point.  */
  if (pdata->flags & NDIGITS)
    {
      if (pdata->flags & EXPOK)
	{
	  /* No digits at all.  */
	  while (p > pdata->buf)
	    {
	      pdata->pfn_ungetc (rptr, *--p, fp); /* "[-+.]".  */
	      --pdata->nread;
	    }
	  return MATCH_FAILURE;
	}
      /* Just a bad exponent (e and maybe sign).  */
      c = *--p;
      --pdata->nread;
      if (c != 'e' && c != 'E')
	{
	  pdata->pfn_ungetc (rptr, c, fp); /* "[-+]".  */
	  c = *--p;
	  --pdata->nread;
	}
      pdata->pfn_ungetc (rptr, c, fp); /* "[eE]".  */
    }
  if ((pdata->flags & SUPPRESS) == 0)
    {
      double fp;
      long new_exp = 0;

      *p = 0;
      if ((pdata->flags & (DPTOK | EXPOK)) == EXPOK)
	{
	  exp_adjust = zeroes - leading_zeroes;
	  new_exp = -exp_adjust;
	  exp_start = p;
	}
      else if (exp_adjust)
	new_exp = _strtol_r (rptr, (exp_start + 1), NULL, 10) - exp_adjust;

      if (exp_adjust)
	{
	  /* If there might not be enough space for the new exponent,
	     truncate some trailing digits to make room.  */
	  if (exp_start >= pdata->buf + BUF - MAX_LONG_LEN)
	    exp_start = pdata->buf + BUF - MAX_LONG_LEN - 1;
	  sprintf (exp_start, "e%ld", new_exp);
	}

      /* Current _strtold routine is markedly slower than
	 _strtod_r.  Only use it if we have a long double
	 result.  */
      fp = _strtod_r (rptr, pdata->buf, NULL);

      /* Do not support long double.  */
      if (pdata->flags & LONG)
	*GET_ARG (N, *ap, double *) = fp;
      else if (pdata->flags & LONGDBL)
	{
	  ldp = GET_ARG (N, *ap, _LONG_DOUBLE *);
	  *ldp = fp;
	}
      else
	{
	  flp = GET_ARG (N, *ap, float *);
	  if (isnan (fp))
	    *flp = nanf ("");
	  else
	    *flp = fp;
	}
      pdata->nassigned++;
    }
  return 0;
}
#endif

