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

#include <newlib.h>

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>
#include "local.h"
#include "../stdlib/local.h"
#include "fvwrite.h"
#include "vfieeefp.h"
#include "nano-vfprintf_local.h"

/* Decode and print non-floating point data.  */
int
_printf_common (
		struct _prt_data_t *pdata,
		int *realsz,
		FILE *fp,
		int (*pfunc)(FILE *,
			     const char *, size_t len))
{
  int n;
  /*
   * All reasonable formats wind up here.  At this point, `cp'
   * points to a string which (if not flags&LADJUST) should be
   * padded out to `width' places.  If flags&ZEROPAD, it should
   * first be prefixed by any sign or other prefix; otherwise,
   * it should be blank padded before the prefix is emitted.
   * After any left-hand padding and prefixing, emit zeroes
   * required by a decimal [diouxX] precision, then print the
   * string proper, then emit zeroes required by any leftover
   * floating precision; finally, if LADJUST, pad with blanks.
   * If flags&FPT, ch must be in [aAeEfg].
   *
   * Compute actual size, so we know how much to pad.
   * size excludes decimal prec; realsz includes it.
   */
  *realsz = pdata->dprec > pdata->size ? pdata->dprec : pdata->size;
  if (pdata->l_buf[0])
    (*realsz)++;

  if (pdata->flags & HEXPREFIX)
    *realsz += 2;

  /* Right-adjusting blank padding.  */
  if ((pdata->flags & (LADJUST|ZEROPAD)) == 0)
    PAD (pdata->width - *realsz, pdata->blank);

  /* Prefix.  */
  n = 0;
  if (pdata->l_buf[0])
    n++;

  if (pdata->flags & HEXPREFIX)
    {
      pdata->l_buf[n++] = '0';
      pdata->l_buf[n++] = pdata->l_buf[2];
    }

  PRINT (pdata->l_buf, n);
  n = pdata->width - *realsz;
  if ((pdata->flags & (LADJUST|ZEROPAD)) != ZEROPAD || n < 0)
    n = 0;

  if (pdata->dprec > pdata->size)
    n += pdata->dprec - pdata->size;

  PAD (n, pdata->zero);
  return 0;
error:
  return -1;
}
int
_printf_i (struct _prt_data_t *pdata, FILE *fp,
	   int (*pfunc)(FILE *, const char *, size_t len),
	   va_list *ap)
{
  /* Field size expanded by dprec.  */
  int realsz;
  u_quad_t _uquad;
  int base;
  int n;
  char *cp = pdata->buf + BUF;
  char *xdigs = "0123456789ABCDEF";

  /* Decoding the conversion specifier.  */
  switch (pdata->code)
    {
    case 'c':
      *--cp = GET_ARG (N, *ap, int);
      pdata->size = 1;
      goto non_number_nosign;
    case 'd':
    case 'i':
      _uquad = SARG (pdata->flags);
      if ((long) _uquad < 0)
	{
	  _uquad = -_uquad;
	  pdata->l_buf[0] = '-';
	}
      base = 10;
      goto number;
    case 'u':
    case 'o':
      _uquad = UARG (pdata->flags);
      base = (pdata->code == 'o') ? 8 : 10;
      goto nosign;
    case 'X':
      pdata->l_buf[2] = 'X';
      goto hex;
    case 'p':
      /*
       * ``The argument shall be a pointer to void.  The
       * value of the pointer is converted to a sequence
       * of printable characters, in an implementation-
       * defined manner.''
       *	-- ANSI X3J11
       */
      pdata->flags |= HEXPREFIX;
      if (sizeof (void*) > sizeof (int))
	pdata->flags |= LONGINT;
      __PICOLIBC_FALLTHROUGH;
      /* NOSTRICT.  */
    case 'x':
      pdata->l_buf[2] = 'x';
      xdigs = "0123456789abcdef";
hex:
      _uquad = UARG (pdata->flags);
      base = 16;
      if (pdata->flags & ALT)
	pdata->flags |= HEXPREFIX;

      /* Leading 0x/X only if non-zero.  */
      if (_uquad == 0)
	pdata->flags &= ~HEXPREFIX;

      /* Unsigned conversions.  */
nosign:
      pdata->l_buf[0] = '\0';
      /*
       * ``... diouXx conversions ... if a precision is
       * specified, the 0 flag will be ignored.''
       *	-- ANSI X3J11
       */
number:
      if ((pdata->dprec = pdata->prec) >= 0)
	pdata->flags &= ~ZEROPAD;

      /*
       * ``The result of converting a zero value with an
       * explicit precision of zero is no characters.''
       *	-- ANSI X3J11
       */
      if (_uquad != 0 || pdata->prec != 0)
	{
	  do
	    {
	      *--cp = xdigs[_uquad % base];
	      _uquad /= base;
	    }
	  while (_uquad);
	}
      /* For 'o' conversion, '#' increases the precision to force the first
	 digit of the result to be zero.  */
      if (base == 8 && (pdata->flags & ALT) && pdata->prec <= pdata->size)
	*--cp = '0';

      pdata->size = pdata->buf + BUF - cp;
      break;
    case 'n':
      if (pdata->flags & LONGINT)
	*GET_ARG (N, *ap, long_ptr_t) = pdata->ret;
      else if (pdata->flags & SHORTINT)
	*GET_ARG (N, *ap, short_ptr_t) = pdata->ret;
      else
	*GET_ARG (N, *ap, int_ptr_t) = pdata->ret;
      __PICOLIBC_FALLTHROUGH;
    case '\0':
      pdata->size = 0;
      break;
    case 's':
      cp = GET_ARG (N, *ap, char_ptr_t);
      /* Precision gives the maximum number of chars to be written from a
	 string, and take prec == -1 into consideration.
	 Use normal Newlib approach here to support case where cp is not
	 nul-terminated.  */
      if (cp == NULL)
          cp = "(null)";
      char *p = memchr (cp, 0, pdata->prec);

      if (p != NULL)
	pdata->prec = p - cp;

      pdata->size = pdata->prec;
      goto non_number_nosign;
    default:
      /* "%?" prints ?, unless ? is NUL.  */
      /* Pretend it was %c with argument ch.  */
      *--cp = pdata->code;
      pdata->size = 1;
non_number_nosign:
      pdata->l_buf[0] = '\0';
      break;
    }

    /* Output.  */
    n = _printf_common (pdata, &realsz, fp, pfunc);
    if (n == -1)
      goto error;

    PRINT (cp, pdata->size);
    /* Left-adjusting padding (always blank).  */
    if (pdata->flags & LADJUST)
      PAD (pdata->width - realsz, pdata->blank);

    return (pdata->width > realsz ? pdata->width : realsz);
error:
    return -1;
}

