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
<<siprintf>>---write formatted output (integer only)

INDEX
	siprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int siprintf(char *<[str]>, const char *<[format]> [, <[arg]>, ...]);

TRAD_SYNOPSIS
        #include <stdio.h>

        int siprintf(<[str]>, <[format]>, [, <[arg]>, ...])
        char *<[str]>;
        const char *<[format]>;

DESCRIPTION
<<siprintf>> is a restricted version of <<sprintf>>: it has the same
arguments and behavior, save that it cannot perform any floating-point
formatting: the <<f>>, <<g>>, <<G>>, <<e>>, and <<F>> type specifiers
are not recognized.

RETURNS
        <<siprintf>> returns the number of bytes in the output string,
        save that the concluding <<NULL>> is not counted.
        <<siprintf>> returns when the end of the format string is
        encountered.

PORTABILITY
<<siprintf>> is not required by ANSI C.

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
  ret = vfiprintf (&f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}

#endif /* ! _REENT_ONLY */

int
#ifdef _HAVE_STDC
_DEFUN(_siprintf_r, (rptr, str, fmt),
       struct _reent *rptr _AND
       char *str           _AND
       _CONST char *fmt _DOTS)
#else
_siprintf_r(rptr, str, fmt, va_alist)
            struct _reent *rptr;
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
  ret = _vfiprintf_r (rptr, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}

