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

/*
FUNCTION
<<vfprintf>>, <<vprintf>>, <<vsprintf>>, <<vsnprintf>>, <<vasprintf>>, <<vasnprintf>>---format argument list

INDEX
	vfprintf
INDEX
	_vfprintf_r
INDEX
	vprintf
INDEX
	_vprintf_r
INDEX
	vsprintf
INDEX
	_vsprintf_r
INDEX
	vsnprintf
INDEX
	_vsnprintf_r
INDEX
	vasprintf
INDEX
	_vasprintf_r
INDEX
	vasnprintf
INDEX
	_vasnprintf_r

SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vprintf(const char *<[fmt]>, va_list <[list]>);
	int vfprintf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsprintf(char *<[str]>, const char *<[fmt]>, va_list <[list]>);
	int vsnprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>,
                      va_list <[list]>);
	int vasprintf(char **<[strp]>, const char *<[fmt]>, va_list <[list]>);
	char *vasnprintf(char *<[str]>, size_t *<[size]>, const char *<[fmt]>,
                         va_list <[list]>);

	int _vprintf_r(struct _reent *<[reent]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vfprintf_r(struct _reent *<[reent]>, FILE *<[fp]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vsprintf_r(struct _reent *<[reent]>, char *<[str]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vasprintf_r(struct _reent *<[reent]>, char **<[str]>,
                         const char *<[fmt]>, va_list <[list]>);
	int _vsnprintf_r(struct _reent *<[reent]>, char *<[str]>,
                         size_t <[size]>, const char *<[fmt]>, va_list <[list]>);
	char *_vasnprintf_r(struct _reent *<[reent]>, char *<[str]>,
                            size_t *<[size]>, const char *<[fmt]>, va_list <[list]>);

DESCRIPTION
<<vprintf>>, <<vfprintf>>, <<vasprintf>>, <<vsprintf>>, <<vsnprintf>>,
and <<vasnprintf>> are (respectively) variants of <<printf>>,
<<fprintf>>, <<asprintf>>, <<sprintf>>, <<snprintf>>, and
<<asnprintf>>.  They differ only in allowing their caller to pass the
variable argument list as a <<va_list>> object (initialized by
<<va_start>>) rather than directly accepting a variable number of
arguments.  The caller is responsible for calling <<va_end>>.

<<_vprintf_r>>, <<_vfprintf_r>>, <<_vasprintf_r>>, <<_vsprintf_r>>,
<<_vsnprintf_r>>, and <<_vasnprintf_r>> are reentrant versions of the
above.

RETURNS
The return values are consistent with the corresponding functions.

PORTABILITY
ANSI C requires <<vprintf>>, <<vfprintf>>, <<vsprintf>>, and
<<vsnprintf>>.  The remaining functions are newlib extensions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if defined(LIBC_SCCS) && !defined(lint)
/*static char *sccsid = "from: @(#)vfprintf.c	5.50 (Berkeley) 12/16/92";*/
static char *rcsid = "$Id$";
#endif /* LIBC_SCCS and not lint */

/* Actual printf innards.
   This code is large and complicated...  */
#include <newlib.h>

#define VFPRINTF vfprintf
#ifdef STRING_ONLY
# define _VFPRINTF_R _svfprintf_r
#else
# define _VFPRINTF_R _vfprintf_r
#endif

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
#include "local.h"
#include "../stdlib/local.h"
#include "fvwrite.h"
#include "vfieeefp.h"
#include "nano-vfprintf_local.h"

/* The __ssputs_r function is shared between all versions of vfprintf
   and vfwprintf.  */
#ifdef STRING_ONLY
int
__ssputs_r (struct _reent *ptr,
       FILE *fp,
       const char *buf,
       size_t len)
{
  register int w;

  w = fp->_w;
  if (len >= w && fp->_flags & (__SMBF | __SOPT))
    {
      /* Must be asprintf family.  */
      unsigned char *str;
      int curpos = (fp->_p - fp->_bf._base);
      /* Choose a geometric growth factor to avoid
       * quadratic realloc behavior, but use a rate less
       * than (1+sqrt(5))/2 to accomodate malloc
       * overhead. asprintf EXPECTS us to overallocate, so
       * that it can add a trailing \0 without
       * reallocating.  The new allocation should thus be
       * max(prev_size*1.5, curpos+len+1).  */
      int newsize = fp->_bf._size * 3 / 2;
      if (newsize < curpos + len + 1)
	newsize = curpos + len + 1;
      if (fp->_flags & __SOPT)
	{
	  /* asnprintf leaves original buffer alone.  */
	  str = (unsigned char *)_malloc_r (ptr, newsize);
	  if (!str)
	    {
	      _REENT_ERRNO(ptr) = ENOMEM;
	      goto err;
	    }
	  memcpy (str, fp->_bf._base, curpos);
	  fp->_flags = (fp->_flags & ~__SOPT) | __SMBF;
	}
      else
	{
	  str = (unsigned char *)_realloc_r (ptr, fp->_bf._base, newsize);
	  if (!str)
	    {
	      /* Free unneeded buffer.  */
	      _free_r (ptr, fp->_bf._base);
	      /* Ensure correct errno, even if free changed it.  */
	      _REENT_ERRNO(ptr) = ENOMEM;
	      goto err;
	    }
	}
      fp->_bf._base = str;
      fp->_p = str + curpos;
      fp->_bf._size = newsize;
      w = len;
      fp->_w = newsize - curpos;
    }
  if (len < w)
    w = len;

  (void)memmove ((void *) fp->_p, (void *) buf, (size_t) (w));
  fp->_w -= w;
  fp->_p += w;
  return 0;

err:
  fp->_flags |= __SERR;
  return EOF;
}
/* __ssprint_r is the original implementation of __SPRINT.  In nano
   version formatted IO it is reimplemented as __ssputs_r for non-wide
   char output, but __ssprint_r cannot be discarded because it is used
   by a serial of functions like svfwprintf for wide char output.  */
int
__ssprint_r (struct _reent *ptr,
       FILE *fp,
       register struct __suio *uio)
{
  register size_t len;
  register int w;
  register struct __siov *iov;
  register const char *p = NULL;

  iov = uio->uio_iov;
  len = 0;

  if (uio->uio_resid == 0)
    {
      uio->uio_iovcnt = 0;
      return (0);
    }

  do
    {
      while (len == 0)
	{
	  p = iov->iov_base;
	  len = iov->iov_len;
	  iov++;
	}
      w = fp->_w;
      if (len >= w && fp->_flags & (__SMBF | __SOPT))
	{
	  /* Must be asprintf family.  */
	  unsigned char *str;
	  int curpos = (fp->_p - fp->_bf._base);
	  /* Choose a geometric growth factor to avoid
	   * quadratic realloc behavior, but use a rate less
	   * than (1+sqrt(5))/2 to accomodate malloc
	   * overhead. asprintf EXPECTS us to overallocate, so
	   * that it can add a trailing \0 without
	   * reallocating.  The new allocation should thus be
	   * max(prev_size*1.5, curpos+len+1).  */
	  int newsize = fp->_bf._size * 3 / 2;
	  if (newsize < curpos + len + 1)
	    newsize = curpos + len + 1;

	  if (fp->_flags & __SOPT)
	    {
	      /* asnprintf leaves original buffer alone.  */
	      str = (unsigned char *)_malloc_r (ptr, newsize);
	      if (!str)
		{
		  _REENT_ERRNO(ptr) = ENOMEM;
		  goto err;
		}
	      memcpy (str, fp->_bf._base, curpos);
	      fp->_flags = (fp->_flags & ~__SOPT) | __SMBF;
	    }
	  else
	    {
	      str = (unsigned char *)_realloc_r (ptr, fp->_bf._base,
						 newsize);
	      if (!str)
		{
		  /* Free unneeded buffer.  */
		  _free_r (ptr, fp->_bf._base);
		  /* Ensure correct errno, even if free changed it.  */
		  _REENT_ERRNO(ptr) = ENOMEM;
		  goto err;
		}
	    }
	  fp->_bf._base = str;
	  fp->_p = str + curpos;
	  fp->_bf._size = newsize;
	  w = len;
	  fp->_w = newsize - curpos;
	}
      if (len < w)
	w = len;

      (void)memmove ((void *) fp->_p, (void *) p, (size_t) (w));
      fp->_w -= w;
      fp->_p += w;
      /* Pretend we copied all.  */
      w = len;
      p += w;
      len -= w;
    }
  while ((uio->uio_resid -= w) != 0);

  uio->uio_resid = 0;
  uio->uio_iovcnt = 0;
  return 0;

err:
  fp->_flags |= __SERR;
  uio->uio_resid = 0;
  uio->uio_iovcnt = 0;
  return EOF;
}
#else
/* As __ssputs_r, __sprint_r is used by output functions for wide char,
   like vfwprint.  */
/* Flush out all the vectors defined by the given uio,
   then reset it so that it can be reused.  */
int
__sprint_r (struct _reent *ptr,
       FILE *fp,
       register struct __suio *uio)
{
  register int err = 0;

  if (uio->uio_resid == 0)
    {
      uio->uio_iovcnt = 0;
      return 0;
    }
#if defined _WIDE_ORIENT && (!defined _ELIX_LEVEL || _ELIX_LEVEL >= 4)
    if (fp->_flags2 & __SWID)
      {
	struct __siov *iov;
	wchar_t *p;
	int i, len;

	iov = uio->uio_iov;
	for (; uio->uio_resid != 0;
	     uio->uio_resid -= len * sizeof (wchar_t), iov++)
	  {
	    p = (wchar_t *) iov->iov_base;
	    len = iov->iov_len / sizeof (wchar_t);
	    for (i = 0; i < len; i++)
	      {
		if (_fputwc_r (ptr, p[i], fp) == WEOF)
		  {
		    err = -1;
		    goto out;
		  }
	      }
	  }
	}
      else
#endif
	err = __sfvwrite_r(ptr, fp, uio);
out:
  uio->uio_resid = 0;
  uio->uio_iovcnt = 0;
  return err;
}

_NOINLINE_STATIC int
__sfputc_r (struct _reent *ptr,
       int c,
       FILE *fp)
{
  if (--fp->_w >= 0 || (fp->_w >= fp->_lbfsize && (char)c != '\n'))
    return (*fp->_p++ = c);
  else
    return (__swbuf_r(ptr, c, fp));
}

int
__sfputs_r (struct _reent *ptr,
       FILE *fp,
       const char *buf,
       size_t len)
{
  register int i;

#if defined _WIDE_ORIENT && (!defined _ELIX_LEVEL || _ELIX_LEVEL >= 4)
  if (fp->_flags2 & __SWID)
    {
      wchar_t *p;

      p = (wchar_t *) buf;
      for (i = 0; i < (len / sizeof (wchar_t)); i++)
	{
	  if (_fputwc_r (ptr, p[i], fp) == WEOF)
	    return -1;
	}
    }
  else
#endif
    {
      for (i = 0; i < len; i++)
	{
	  /* Call __sfputc_r to skip _fputc_r.  */
	  if (__sfputc_r (ptr, (int)buf[i], fp) == EOF)
	    return -1;
	}
    }
  return (0);
}
#endif /* STRING_ONLY.  */

int _VFPRINTF_R (struct _reent *, FILE *, const char *, va_list);

#ifndef STRING_ONLY
int
VFPRINTF (FILE * fp,
       const char *fmt0,
       va_list ap)
{
  int result;
  result = _VFPRINTF_R (_REENT, fp, fmt0, ap);
  return result;
}

int
vfiprintf (FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("vfprintf")));
#endif

#ifdef STRING_ONLY
# define __SPRINT __ssputs_r
#else
# define __SPRINT __sfputs_r
#endif

/* Do not need FLUSH for all sprintf functions.  */
#ifdef STRING_ONLY
# define FLUSH()
#else
# define FLUSH()
#endif

int
_VFPRINTF_R (struct _reent *data,
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

  pfunc = __SPRINT;

#ifndef STRING_ONLY
  /* Initialize std streams if not dealing with sprintf family.  */
  CHECK_INIT (data, fp);
  _newlib_flockfile_start (fp);

  /* Sorry, fprintf(read_only_file, "") returns EOF, not 0.  */
  if (cantwrite (data, fp))
    {
      _newlib_flockfile_exit (fp);
      return (EOF);
    }

#else
  /* Create initial buffer if we are called by asprintf family.  */
  if (fp->_flags & __SMBF && !fp->_bf._base)
    {
      fp->_bf._base = fp->_p = _malloc_r (data, 64);
      if (!fp->_p)
	{
	  _REENT_ERRNO(data) = ENOMEM;
	  return EOF;
	}
      fp->_bf._size = 64;
    }
#endif

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
  FLUSH ();
error:
#ifndef STRING_ONLY
  _newlib_flockfile_end (fp);
#endif
  va_end (ap_copy);
  return (__sferror (fp) ? EOF : prt_data.ret);
}

#ifdef STRING_ONLY
int
_svfiprintf_r (struct _reent *, FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("_svfprintf_r")));
#else
int
_vfiprintf_r (struct _reent *, FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("_vfprintf_r")));
#endif
