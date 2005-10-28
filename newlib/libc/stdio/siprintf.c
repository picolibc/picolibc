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
        <<iprintf>>, <<fiprintf>>, <<asiprintf>>, <<siprintf>>, <<sniprintf>>---format output

INDEX
	fiprintf
INDEX
	iprintf
INDEX
	asiprintf
INDEX
	siprintf
INDEX
	sniprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int iprintf(const char *<[format]> [, <[arg]>, ...]);
        int fiprintf(FILE *<[fd]>, const char *<[format]> [, <[arg]>, ...]);
        int siprintf(char *<[str]>, const char *<[format]> [, <[arg]>, ...]);
        int asiprintf(char **<[strp]>, const char *<[format]> [, <[arg]>, ...]);
        int sniprintf(char *<[str]>, size_t <[size]>, const char *<[format]>
                      [, <[arg]>, ...]);

TRAD_SYNOPSIS
	#include <stdio.h>

	int iprintf(<[format]> [, <[arg]>, ...])
	char *<[format]>;

	int fiprintf(<[fd]>, <[format]> [, <[arg]>, ...]);
	FILE *<[fd]>;
	char *<[format]>;

	int asiprintf(<[strp]>, <[format]> [, <[arg]>, ...]);
	char **<[strp]>;
	char *<[format]>;

	int siprintf(<[str]>, <[format]> [, <[arg]>, ...]);
	char *<[str]>;
	char *<[format]>;

	int sniprintf(<[str]>, size_t <[size]>, <[format]> [, <[arg]>, ...]);
	char *<[str]>;
        size_t <[size]>;
	char *<[format]>;

DESCRIPTION
        <<iprintf>>, <<fiprintf>>, <<siprintf>>, <<sniprintf>>,
        <<asiprintf>>, are the same as <<printf>>, <<fprintf>>,
	<<sprintf>>, <<snprintf>>, and <<asprintf>>, respectively,
	only that they restrict usage to non-floating-point format
	specifiers.

RETURNS
<<siprintf>> and <<asiprintf>> return the number of bytes in the output string,
save that the concluding <<NULL>> is not counted.
<<iprintf>> and <<fiprintf>> return the number of characters transmitted.
If an error occurs, <<iprintf>> and <<fiprintf>> return <<EOF>> and
<<asiprintf>> returns -1.  No error returns occur for <<siprintf>>.

PORTABILITY
<<iprintf>>, <<fiprintf>>, <<siprintf>>, <<sniprintf>>, and <<asprintf>>
are newlib extensions.

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
  ret = _vfiprintf_r (ptr, &f, fmt, ap);
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
  ret = _vfiprintf_r (_REENT, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  return (ret);
}

#endif
