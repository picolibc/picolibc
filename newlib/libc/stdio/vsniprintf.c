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
<<vsniprintf>>---write formatted output (integer only)

INDEX
	vsniprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int vsniprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>, va_list <[list]>);

TRAD_SYNOPSIS
        #include <stdio.h>

        int vsnprintf(<[str]>, <[size]>, <[fmt]>, <[list]>)
        char *<[str]>;
        size_t <[size]>;
        char *<[fmt]>;
        va_list <[list]>;

DESCRIPTION
<<vsniprintf>> is a restricted version of <<vsnprintf>>: it has the same
arguments and behavior, save that it cannot perform any floating-point
formatting: the <<f>>, <<g>>, <<G>>, <<e>>, and <<F>> type specifiers
are not recognized.

RETURNS
        <<vsniprintf>> returns the number of bytes in the output string,
        save that the concluding <<NULL>> is not counted.
        <<vsniprintf>> returns when the end of the format string is
        encountered.

PORTABILITY
<<vsniprintf>> is not required by ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <limits.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef _REENT_ONLY

int
_DEFUN(vsniprintf, (str, size, fmt, ap),
       char *str        _AND
       size_t size      _AND
       _CONST char *fmt _AND
       va_list ap)
{
  int ret;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? size - 1 : 0);
  f._file = -1;  /* No file. */
  ret = _vfiprintf_r (_REENT, &f, fmt, ap);
  if (size > 0)
    *f._p = 0;
  return ret;
}

#endif /* !_REENT_ONLY */

int
_DEFUN(_vsniprintf_r, (ptr, str, size, fmt, ap),
       struct _reent *ptr _AND
       char *str          _AND
       size_t size        _AND
       _CONST char *fmt   _AND
       va_list ap)
{
  int ret;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = (size > 0 ? size - 1 : 0);
  f._file = -1;  /* No file. */
  ret = _vfiprintf_r (ptr, &f, fmt, ap);
  if (size > 0)
    *f._p = 0;
  return ret;
}
