/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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

#if defined(LIBC_SCCS) && !defined(lint)
/*static char *sccsid = "from: @(#)vfprintf.c	5.50 (Berkeley) 12/16/92";*/
static char *rcsid = "$Id$";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>
#include <string.h>
#include <newlib.h>
#include "../../stdio/local.h"
#include "../../stdlib/local.h"
#include "../../stdio/fvwrite.h"
#include "../../stdio/nano-vfprintf_local.h"

/* Bypass *putc* fns called by the default _sfputs_r to save code size.
   Among other things, this means there is no buffering of the string before
   it is sent to <<write>>, but <<write>> does its own buffering so we won't
   lose chars when buf is larger than sizeof(CIOBUF).  */
int
__tiny__sfputs_r (struct _reent *ptr,
       FILE *fp,
       const char *buf,
       size_t len)
{
  return write (1, buf, len);
}

/* VFPRINTF_R from nano-vfprintf.c but:
   - No support for reentrancy
   - No support for streams
   - __SINGLE_THREAD__ assumed.
   - No STRING_ONLY variant (either way the formatted string would end up
     being sent to <<write>> via <<__tiny__sfputs_r>>.
   Basically, format the string as normal, and send it to write.  */
int
__tiny_vfprintf_r (struct _reent *data,
       FILE * fp,
       const char *fmt0,
       va_list ap)
{
  register char *fmt;	/* Format string.  */
  register int n, m;	/* Handy integers (short term usage).  */
  register char *cp;	/* Handy char pointer (short term usage).  */
  const char *flag_chars;
  struct _prt_data_t prt_data;	/* All data for decoding format string.  */
  va_list ap_copy;

  /* Output function pointer.  */
  int (*pfunc)(struct _reent *, FILE *, const char *, size_t len);

  pfunc = __tiny__sfputs_r;

  fmt = (char *)fmt0;
  prt_data.ret = 0;
  prt_data.blank = ' ';
  prt_data.zero = '0';

  /* GCC PR 14577 at https://gcc.gnu.org/bugzilla/show_bug.cgi?id=14557 */
  va_copy (ap_copy, ap);

  /* Scan the format for conversions (`%' character).  */
  for (;;)
    {
      cp = fmt;
      while (*fmt != '\0' && *fmt != '%')
	fmt += 1;

      if ((m = fmt - cp) != 0)
	{
	  PRINT (cp, m);
	  prt_data.ret += m;
	}
      if (*fmt == '\0')
	goto done;

      fmt++;		/* Skip over '%'.  */

      prt_data.flags = 0;
      prt_data.width = 0;
      prt_data.prec = -1;
      prt_data.dprec = 0;
      prt_data.l_buf[0] = '\0';
#ifdef FLOATING_POINT
      prt_data.lead = 0;
#endif
      /* The flags.  */
      /*
       * ``Note that 0 is taken as a flag, not as the
       * beginning of a field width.''
       *	-- ANSI X3J11
       */
      flag_chars = "#-0+ ";
      for (; cp = memchr (flag_chars, *fmt, 5); fmt++)
	prt_data.flags |= (1 << (cp - flag_chars));

      if (prt_data.flags & SPACESGN)
	prt_data.l_buf[0] = ' ';

      /*
       * ``If the space and + flags both appear, the space
       * flag will be ignored.''
       *	-- ANSI X3J11
       */
      if (prt_data.flags & PLUSSGN)
	prt_data.l_buf[0] = '+';

      /* The width.  */
      if (*fmt == '*')
	{
	  /*
	   * ``A negative field width argument is taken as a
	   * - flag followed by a positive field width.''
	   *	-- ANSI X3J11
	   * They don't exclude field widths read from args.
	   */
	  prt_data.width = GET_ARG (n, ap_copy, int);
	  if (prt_data.width < 0)
	    {
	      prt_data.width = -prt_data.width;
	      prt_data.flags |= LADJUST;
	    }
	  fmt++;
	}
      else
        {
	  for (; is_digit (*fmt); fmt++)
	    prt_data.width = 10 * prt_data.width + to_digit (*fmt);
	}

      /* The precision.  */
      if (*fmt == '.')
	{
	  fmt++;
	  if (*fmt == '*')
	    {
	      fmt++;
	      prt_data.prec = GET_ARG (n, ap_copy, int);
	      if (prt_data.prec < 0)
		prt_data.prec = -1;
	    }
	  else
	    {
	      prt_data.prec = 0;
	      for (; is_digit (*fmt); fmt++)
		prt_data.prec = 10 * prt_data.prec + to_digit (*fmt);
	    }
	}

      /* The length modifiers.  */
      flag_chars = "hlL";
      if ((cp = memchr (flag_chars, *fmt, 3)) != NULL)
	{
	  prt_data.flags |= (SHORTINT << (cp - flag_chars));
	  fmt++;
	}

      /* The conversion specifiers.  */
      prt_data.code = *fmt++;
      cp = memchr ("efgEFG", prt_data.code, 6);
#ifdef FLOATING_POINT
      /* If cp is not NULL, we are facing FLOATING POINT NUMBER.  */
      if (cp)
	{
	  /* Consume floating point argument if _printf_float is not
	     linked.  */
	  if (_printf_float == NULL)
	    {
	      if (prt_data.flags & LONGDBL)
		GET_ARG (N, ap_copy, _LONG_DOUBLE);
	      else
		GET_ARG (N, ap_copy, double);
	    }
	  else
            n = _printf_float (data, &prt_data, fp, pfunc, &ap_copy);
	}
      else
#endif
	n = _printf_i (data, &prt_data, fp, pfunc, &ap_copy);

      if (n == -1)
	goto error;

      prt_data.ret += n;
    }
done:
error:
  va_end (ap_copy);
  return (__sferror (fp) ? EOF : prt_data.ret);
}

int
__wrap_printf (const char *__restrict fmt, ...)
{
  int ret;
  va_list ap;
  struct _reent *ptr = _REENT;

  va_start (ap, fmt);
  ret = __tiny_vfprintf_r (ptr, _stdout_r (ptr), fmt, ap);
  va_end (ap);
  return ret;
}

int printf (const char *__restrict fmt, ...) __attribute__ ((weak, alias ("__wrap_printf")));
