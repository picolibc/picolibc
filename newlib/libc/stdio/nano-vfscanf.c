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
<<vfscanf>>, <<vscanf>>, <<vsscanf>>---format argument list

INDEX
	vfscanf
INDEX
	_vfscanf_r
INDEX
	vscanf
INDEX
	_vscanf_r
INDEX
	vsscanf
INDEX
	_vsscanf_r

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vscanf(const char *<[fmt]>, va_list <[list]>);
	int vfscanf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsscanf(const char *<[str]>, const char *<[fmt]>, va_list <[list]>);

	int _vscanf_r(struct _reent *<[reent]>, const char *<[fmt]>,
                       va_list <[list]>);
	int _vfscanf_r(struct _reent *<[reent]>, FILE *<[fp]>, const char *<[fmt]>,
                       va_list <[list]>);
	int _vsscanf_r(struct _reent *<[reent]>, const char *<[str]>,
                       const char *<[fmt]>, va_list <[list]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <varargs.h>
	int vscanf( <[fmt]>, <[ist]>)
	char *<[fmt]>;
	va_list <[list]>;

	int vfscanf( <[fp]>, <[fmt]>, <[list]>)
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsscanf( <[str]>, <[fmt]>, <[list]>)
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vscanf_r( <[reent]>, <[fmt]>, <[ist]>)
	struct _reent *<[reent]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vfscanf_r( <[reent]>, <[fp]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsscanf_r( <[reent]>, <[str]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

DESCRIPTION
<<vscanf>>, <<vfscanf>>, and <<vsscanf>> are (respectively) variants
of <<scanf>>, <<fscanf>>, and <<sscanf>>.  They differ only in
allowing their caller to pass the variable argument list as a
<<va_list>> object (initialized by <<va_start>>) rather than
directly accepting a variable number of arguments.

RETURNS
The return values are consistent with the corresponding functions:
<<vscanf>> returns the number of input fields successfully scanned,
converted, and stored; the return value does not include scanned
fields which were not stored.

If <<vscanf>> attempts to read at end-of-file, the return value
is <<EOF>>.

If no fields were stored, the return value is <<0>>.

The routines <<_vscanf_r>>, <<_vfscanf_f>>, and <<_vsscanf_r>> are
reentrant versions which take an additional first parameter which points to the
reentrancy structure.

PORTABILITY
These are GNU extensions.

Supporting OS subroutines required:
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

#define VFSCANF vfscanf
#define _VFSCANF_R _vfscanf_r
#define __SVFSCANF __svfscanf
#ifdef STRING_ONLY
#  define __SVFSCANF_R __ssvfscanf_r
#else
#  define __SVFSCANF_R __svfscanf_r
#endif

/* vfscanf.  */

#ifndef STRING_ONLY

#ifndef _REENT_ONLY

int
_DEFUN(VFSCANF, (fp, fmt, ap),
       register FILE *fp _AND
       _CONST char *fmt _AND
       va_list ap)
{
  CHECK_INIT(_REENT, fp);
  return __SVFSCANF_R (_REENT, fp, fmt, ap);
}

int
_EXFUN(vfiscanf, (FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("vfscanf"))));

int
_DEFUN(__SVFSCANF, (fp, fmt0, ap),
       register FILE *fp _AND
       char _CONST *fmt0 _AND
       va_list ap)
{
  return __SVFSCANF_R (_REENT, fp, fmt0, ap);
}

#endif

int
_DEFUN(_VFSCANF_R, (data, fp, fmt, ap),
       struct _reent *data _AND
       register FILE *fp   _AND
       _CONST char *fmt    _AND
       va_list ap)
{
  CHECK_INIT(data, fp);
  return __SVFSCANF_R (data, fp, fmt, ap);
}

int
_EXFUN(_vfiscanf_r, (struct _reent *, FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("_vfscanf_r"))));
#endif /* !STRING_ONLY.  */

#if defined (STRING_ONLY)
/* When dealing with the sscanf family, we don't want to use the
   regular ungetc which will drag in file I/O items we don't need.
   So, we create our own trimmed-down version.  */
int
_DEFUN(_sungetc_r, (data, fp, ch),
	struct _reent *data _AND
	int c               _AND
	register FILE *fp)
{
  if (c == EOF)
    return (EOF);

  /* After ungetc, we won't be at eof anymore.  */
  fp->_flags &= ~__SEOF;
  c = (unsigned char) c;

  /* If we are in the middle of ungetc'ing, just continue.
     This may require expanding the current ungetc buffer.  */

  if (HASUB (fp))
    {
      if (fp->_r >= fp->_ub._size && __submore (data, fp))
        return EOF;

      *--fp->_p = c;
      fp->_r++;
      return c;
    }

  /* If we can handle this by simply backing up, do so,
     but never replace the original character.
     (This makes sscanf() work when scanning `const' data).  */
  if (fp->_bf._base != NULL && fp->_p > fp->_bf._base && fp->_p[-1] == c)
    {
      fp->_p--;
      fp->_r++;
      return c;
    }

  /* Create an ungetc buffer.
     Initially, we will use the `reserve' buffer.  */
  fp->_ur = fp->_r;
  fp->_up = fp->_p;
  fp->_ub._base = fp->_ubuf;
  fp->_ub._size = sizeof (fp->_ubuf);
  fp->_ubuf[sizeof (fp->_ubuf) - 1] = c;
  fp->_p = &fp->_ubuf[sizeof (fp->_ubuf) - 1];
  fp->_r = 1;
  return c;
}

/* String only version of __srefill_r for sscanf family.  */
int
_DEFUN(__ssrefill_r, (ptr, fp),
       struct _reent * ptr _AND
       register FILE * fp)
{
  /* Our only hope of further input is the ungetc buffer.
     If there is anything in that buffer to read, return.  */
  if (HASUB (fp))
    {
      FREEUB (ptr, fp);
      if ((fp->_r = fp->_ur) != 0)
        {
          fp->_p = fp->_up;
	  return 0;
        }
    }

  /* Otherwise we are out of character input.  */
  fp->_p = fp->_bf._base;
  fp->_r = 0;
  fp->_flags |= __SEOF;
  return EOF;
}

#else
int _EXFUN (_sungetc_r, (struct _reent *, int, register FILE *));
int _EXFUN (__ssrefill_r, (struct _reent *, register FILE *));
size_t _EXFUN (_sfread_r, (struct _reent *, _PTR buf, size_t, size_t, FILE *));
#endif /* !STRING_ONLY.  */

int
_DEFUN(__SVFSCANF_R, (rptr, fp, fmt0, ap),
       struct _reent *rptr _AND
       register FILE *fp   _AND
       char _CONST *fmt0   _AND
       va_list ap)
{
  register u_char *fmt = (u_char *) fmt0;
  register int c;		/* Character from format, or conversion.  */
  register char *p;		/* Points into all kinds of strings.  */
  char ccltab[256];		/* Character class table for %[...].  */

  int ret;
  char *cp;

  struct _scan_data_t scan_data;
  int (*scan_func)(struct _reent*, struct _scan_data_t*, FILE *, va_list *);

  _newlib_flockfile_start (fp);

  scan_data.nassigned = 0;
  scan_data.nread = 0;
  scan_data.ccltab = ccltab;
  scan_data.pfn_ungetc = _ungetc_r;
  scan_data.pfn_refill = __srefill_r;

  for (;;)
    {
      if (*fmt == 0)
	goto all_done;

      if (isspace (*fmt))
	{
	  while ((fp->_r > 0 || !scan_data.pfn_refill(rptr, fp))
		 && isspace (*fp->_p))
	    {
	      scan_data.nread++;
	      fp->_r--;
	      fp->_p++;
	    }
	  fmt++;
	  continue;
	}
      if ((c = *fmt++) != '%')
	goto literal;

      scan_data.width = 0;
      scan_data.flags = 0;

      if (*fmt == '*')
	{
	  scan_data.flags |= SUPPRESS;
	  fmt++;
	}

      for (; is_digit (*fmt); fmt++)
	scan_data.width = 10 * scan_data.width + to_digit (*fmt);

      /* The length modifiers.  */
      p = "hlL";
      if ((cp = memchr (p, *fmt, 3)) != NULL) {
	scan_data.flags |= (SHORT << (cp - p));
	fmt++;
      }

      /* Switch on the format.  continue if done; break once format
	 type is derived.  */
      c = *fmt++;
      switch (c)
	{
	case '%':
	literal:
	  if ((fp->_r <= 0 && scan_data.pfn_refill(rptr, fp)))
	    goto input_failure;
	  if (*fp->_p != c)
	    goto match_failure;
	  fp->_r--, fp->_p++;
	  scan_data.nread++;
	  continue;

	case 'p':
	  scan_data.flags |= POINTER;
	case 'x':
	case 'X':
	  scan_data.flags |= PFXOK;
	  scan_data.base = 16;
	  goto number;
	case 'd':
	case 'u':
	  scan_data.base = 10;
	  goto number;
	case 'i':
	  scan_data.base = 0;
	  goto number;
	case 'o':
	  scan_data.base = 8;
	number:
	  scan_data.code = (c < 'o') ? CT_INT : CT_UINT;
	  break;

	case '[':
	  fmt = (u_char *) __sccl (ccltab, (unsigned char *) fmt);
	  scan_data.flags |= NOSKIP;
	  scan_data.code = CT_CCL;
	  break;
	case 'c':
	  scan_data.flags |= NOSKIP;
	  scan_data.code = CT_CHAR;
	  break;
	case 's':
	  scan_data.code = CT_STRING;
	  break;

	case 'n':
	  if (scan_data.flags & SUPPRESS)	/* ???  */
	    continue;

	  if (scan_data.flags & SHORT)
	    *GET_ARG (N, ap, short *) = scan_data.nread;
	  else if (scan_data.flags & LONG)
	    *GET_ARG (N, ap, long *) = scan_data.nread;
	  else
	    *GET_ARG (N, ap, int *) = scan_data.nread;

	  continue;

	/* Disgusting backwards compatibility hacks.	XXX.  */
	case '\0':		/* compat.  */
	  _newlib_flockfile_exit (fp);
	  return EOF;

#ifdef FLOATING_POINT
	case 'e': case 'E':
	case 'f': case 'F':
	case 'g': case 'G':
	  scan_data.code = CT_FLOAT;
	  break;
#endif
	default:		/* compat.  */
	  scan_data.code = CT_INT;
	  scan_data.base = 10;
	  break;
	}

      /* We have a conversion that requires input.  */
      if ((fp->_r <= 0 && scan_data.pfn_refill (rptr, fp)))
	goto input_failure;

      /* Consume leading white space, except for formats that
	 suppress this.  */
      if ((scan_data.flags & NOSKIP) == 0)
	{
	  while (isspace (*fp->_p))
	    {
	      scan_data.nread++;
	      if (--fp->_r > 0)
		fp->_p++;
	      else if (scan_data.pfn_refill (rptr, fp))
		goto input_failure;
	    }
	  /* Note that there is at least one character in the
	     buffer, so conversions that do not set NOSKIP ca
	     no longer result in an input failure.  */
	}
      ret = 0;
      if (scan_data.code < CT_INT)
	ret = _scanf_chars (rptr, &scan_data, fp, &ap);
      else if (scan_data.code < CT_FLOAT)
	ret = _scanf_i (rptr, &scan_data, fp, &ap);
#ifdef FLOATING_POINT
      else if (_scanf_float)
	ret = _scanf_float (rptr, &scan_data, fp, &ap);
#endif

      if (ret == MATCH_FAILURE)
	goto match_failure;
      else if (ret == INPUT_FAILURE)
	goto input_failure;
    }
input_failure:
  /* On read failure, return EOF failure regardless of matches; errno
     should have been set prior to here.  On EOF failure (including
     invalid format string), return EOF if no matches yet, else number
     of matches made prior to failure.  */
  _newlib_flockfile_exit (fp);
  return scan_data.nassigned && !(fp->_flags & __SERR) ? scan_data.nassigned
						       : EOF;
match_failure:
all_done:
  /* Return number of matches, which can be 0 on match failure.  */
  _newlib_flockfile_end (fp);
  return scan_data.nassigned;
}

#ifdef STRING_ONLY
int
_EXFUN(__ssvfiscanf_r, (struct _reent *, FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("__ssvfscanf_r"))));
#else
int
_EXFUN(__svfiscanf_r, (struct _reent *, FILE *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("__svfscanf_r"))));
#endif

