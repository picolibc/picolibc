/*
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
FUNCTION
<<siprintf>>, <<fiprintf>>, <<iprintf>>, <<sniprintf>>, <<asiprintf>>, <<asniprintf>>---format output (integer only)

INDEX
	fiprintf
INDEX
	_fiprintf_r
INDEX
	iprintf
INDEX
	_iprintf_r
INDEX
	siprintf
INDEX
	_siprintf_r
INDEX
	sniprintf
INDEX
	_sniprintf_r
INDEX
	asiprintf
INDEX
	_asiprintf_r
INDEX
	asniprintf
INDEX
	_asniprintf_r

ANSI_SYNOPSIS
        #include <stdio.h>

        int iprintf(const char *<[format]>, ...);
        int fiprintf(FILE *<[fd]>, const char *<[format]> , ...);
        int siprintf(char *<[str]>, const char *<[format]>, ...);
        int sniprintf(char *<[str]>, size_t <[size]>, const char *<[format]>, 
			...);
        int asiprintf(char **<[strp]>, const char *<[format]>, ...);
        char *asniprintf(char *<[str]>, size_t *<[size]>, 
			const char *<[format]>, ...);

        int _iprintf_r(struct _reent *<[ptr]>, const char *<[format]>, ...);
        int _fiprintf_r(struct _reent *<[ptr]>, FILE *<[fd]>,
                        const char *<[format]>, ...);
        int _siprintf_r(struct _reent *<[ptr]>, char *<[str]>,
                        const char *<[format]>, ...);
        int _sniprintf_r(struct _reent *<[ptr]>, char *<[str]>, size_t <[size]>,
                         const char *<[format]>, ...);
        int _asiprintf_r(struct _reent *<[ptr]>, char **<[strp]>,
                         const char *<[format]>, ...);
        char *_asniprintf_r(struct _reent *<[ptr]>, char *<[str]>,
                            size_t *<[size]>, const char *<[format]>, ...);

DESCRIPTION
        <<iprintf>>, <<fiprintf>>, <<siprintf>>, <<sniprintf>>,
        <<asiprintf>>, and <<asniprintf>> are the same as <<printf>>,
        <<fprintf>>, <<sprintf>>, <<snprintf>>, <<asprintf>>, and
        <<asnprintf>>, respectively, except that they restrict usage
        to non-floating-point format specifiers.

        <<_iprintf_r>>, <<_fiprintf_r>>, <<_asiprintf_r>>,
        <<_siprintf_r>>, <<_sniprintf_r>>, <<_asniprintf_r>> are
        simply reentrant versions of the functions above.

RETURNS
Similar to <<printf>>, <<fprintf>>, <<sprintf>>, <<snprintf>>, <<asprintf>>,
and <<asnprintf>>.

PORTABILITY
<<iprintf>>, <<fiprintf>>, <<siprintf>>, <<sniprintf>>, <<asiprintf>>,
and <<asniprintf>> are newlib extensions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <limits.h>
#include "local.h"

int
#ifdef _HAVE_STDC
_DEFUN(_siprintf_r, (ptr, str, fmt),
       struct _reent *ptr _AND
       char *str          _AND
       _CONST char *fmt _DOTS)
#else
_siprintf_r(ptr, str, fmt, va_alist)
           struct _reent *ptr;
           char *str;
           _CONST char *fmt;
           va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = INT_MAX;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = _svfiprintf_r (ptr, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}

#ifndef _REENT_ONLY

int
#ifdef _HAVE_STDC
_DEFUN(siprintf, (str, fmt),
       char *str _AND
       _CONST char *fmt _DOTS)
#else
siprintf(str, fmt, va_alist)
        char *str;
        _CONST char *fmt;
        va_dcl
#endif
{
  int ret;
  va_list ap;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = INT_MAX;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = _svfiprintf_r (_REENT, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}

#endif
