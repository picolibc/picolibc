/*
FUNCTION
        <<siprintf>>---write formatted output (integer only)
INDEX
	siprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int siprintf(char *<[str]>, const char *<[format]> [, <[arg]>, ...]);


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

#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <limits.h>
#include <_ansi.h>
#include <reent.h>
#include "local.h"

int
#ifdef _HAVE_STDC
_DEFUN (siprintf, (str, fmt), char *str _AND _CONST char *fmt _DOTS)
#else
siprintf (str, fmt, va_alist)
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
  f._data = _REENT;
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
