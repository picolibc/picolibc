/*
 * Copyright (c) 2012-2014 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

int
_scanf_chars (struct _reent *rptr,
	      struct _scan_data_t *pdata,
	      FILE *fp, va_list *ap)
{
  int n;
  char *p;

  if (pdata->width == 0)
    pdata->width = (pdata->code == CT_CHAR) ? 1 : (size_t)~0;

  n = 0;
  if ((pdata->flags & SUPPRESS) == 0)
    p = GET_ARG (N, *ap, char *);

  /* It's impossible to have EOF when we get here.  */
  while ((pdata->code == CT_CHAR)
	 || (pdata->code == CT_CCL && pdata->ccltab[*fp->_p])
	 || (pdata->code == CT_STRING && !isspace (*fp->_p)))
    {
      n++;
      if ((pdata->flags & SUPPRESS) == 0)
	*p++ = *fp->_p;
	
      fp->_r--, fp->_p++;
      if (--pdata->width == 0)
	break;

      if ((fp->_r <= 0 && pdata->pfn_refill (rptr, fp)))
	break;
    }
  /* For CT_CHAR, it is impossible to have input_failure(n == 0) here.
     For CT_CCL, it is impossible to have input_failure here.
     For CT_STRING, it is possible to have empty string.  */
  if (n == 0 && pdata->code == CT_CCL)
    return MATCH_FAILURE;

  if ((pdata->flags & SUPPRESS) == 0)
    {
      pdata->nassigned++;
      if (pdata->code != CT_CHAR)
	*p = 0;
    }
  pdata->nread += n;
  return 0;
}
int
_scanf_i (struct _reent *rptr,
	  struct _scan_data_t *pdata,
	  FILE *fp, va_list *ap)
{
#define CCFN_PARAMS	(struct _reent *, const char *, char **, int)
  /* Conversion function (strtol/strtoul).  */
  u_long (*ccfn)CCFN_PARAMS=0;
  char *p;
  int n;
  char *xdigits = "A-Fa-f8901234567]";
  char *prefix_chars[3] = {"+-", "00", "xX"};

  /* Scan an integer as if by strtol/strtoul.  */
  unsigned width_left = 0;
  int skips = 0;

  ccfn = (pdata->code == CT_INT) ? (u_long (*)CCFN_PARAMS)_strtol_r : _strtoul_r;
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
  p = pdata->buf;
  pdata->flags |= NDIGITS | NZDIGITS | NNZDIGITS;

  /* Process [sign] [0] [xX] prefixes sequently.  */
  for (n = 0; n < 3; n++)
    {
      if (!memchr (prefix_chars[n], *fp->_p, 2))
	continue;

      if (n == 1)
	{
	  if (pdata->base == 0)
	    {
	      pdata->base = 8;
	      pdata->flags |= PFXOK;
	    }
	  pdata->flags &= ~(NZDIGITS | NDIGITS);
	}
      else if (n == 2)
	{
	  if ((pdata->flags & (PFXOK | NZDIGITS)) != PFXOK)
	    continue;
	  pdata->base = 16;

	  /* We must reset the NZDIGITS and NDIGITS
	     flags that would have been unset by seeing
	     the zero that preceded the X or x.

	     ??? It seems unnecessary to reset the NZDIGITS.  */
	  pdata->flags |= NDIGITS;
	}
      if (pdata->width-- > 0)
	{
	  *p++ = *fp->_p++;
	  fp->_r--;
	  if ((fp->_r <= 0 && pdata->pfn_refill (rptr, fp)))
	    goto match_end;
	}
    }

  if (pdata->base == 0)
    pdata->base = 10;

  /* The check is un-necessary if xdigits points to exactly the string:
     "A-Fa-f8901234567]".  The code is kept only for reading's sake.  */
#if 0
  if (pdata->base != 16)
#endif
  xdigits = xdigits + 16 - pdata->base;

  /* Initilize ccltab according to pdata->base.  */
  __sccl (pdata->ccltab, (unsigned char *) xdigits);
  for (; pdata->width; pdata->width--)
    {
      n = *fp->_p;
      if (pdata->ccltab[n] == 0)
	break;
      else if (n == '0' && (pdata->flags & NNZDIGITS))
	{
	  ++skips;
	  if (width_left)
	    {
	      width_left--;
	      pdata->width++;
	    }
	  goto skip;
	}
      pdata->flags &= ~(NDIGITS | NNZDIGITS);
      /* Char is legal: store it and look at the next.  */
      *p++ = *fp->_p;
skip:
      if (--fp->_r > 0)
	fp->_p++;
      else if (pdata->pfn_refill (rptr, fp))
	/* "EOF".  */
	break;
    }
  /* If we had only a sign, it is no good; push back the sign.
     If the number ends in `x', it was [sign] '0' 'x', so push back
     the x and treat it as [sign] '0'.
     Use of ungetc here and below assumes ASCII encoding; we are only
     pushing back 7-bit characters, so casting to unsigned char is
     not necessary.  */
match_end:
  if (pdata->flags & NDIGITS)
    {
      if (p > pdata->buf)
	pdata->pfn_ungetc (rptr, *--p, fp); /* "[-+xX]".  */

      if (p == pdata->buf)
	return MATCH_FAILURE;
    }
  if ((pdata->flags & SUPPRESS) == 0)
    {
      u_long ul;
      *p = 0;
      ul = (*ccfn) (rptr, pdata->buf, (char **) NULL, pdata->base);
      if (pdata->flags & POINTER)
	*GET_ARG (N, *ap, void **) = (void *) (uintptr_t) ul;
      else if (pdata->flags & SHORT)
	*GET_ARG (N, *ap, short *) = ul;
      else if (pdata->flags & LONG)
	*GET_ARG (N, *ap, long *) = ul;
      else
	*GET_ARG (N, *ap, int *) = ul;
      
      pdata->nassigned++;
    }
  pdata->nread += p - pdata->buf + skips;
  return 0;
}

