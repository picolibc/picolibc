/*
FUNCTION
        <<fiprintf>>---format output to file (integer only)
INDEX
	fiprintf

ANSI_SYNOPSIS
        #include <stdio.h>

        int fiprintf(FILE *<[fd]>, const char *<[format]>, ...);

TRAD_SYNOPSIS
	#include <stdio.h>

	int fiprintf(<[fd]>, <[format]> [, <[arg]>, ...]);
	FILE *<[fd]>;
	char *<[format]>;

DESCRIPTION
<<fiprintf>> is a restricted version of <<fprintf>>: it has the same
arguments and behavior, save that it cannot perform any floating-point
formatting---the <<f>>, <<g>>, <<G>>, <<e>>, and <<F>> type specifiers
are not recognized.

RETURNS
        <<fiprintf>> returns the number of bytes in the output string,
        save that the concluding <<NULL>> is not counted.
        <<fiprintf>> returns when the end of the format string is
        encountered.  If an error occurs, <<fiprintf>>
        returns <<EOF>>.

PORTABILITY
<<fiprintf>> is not required by ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>

#ifdef _HAVE_STDC

#include <stdarg.h>

int
fiprintf (FILE * fp, const char *fmt,...)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = vfiprintf (fp, fmt, ap);
  va_end (ap);
  return ret;
}

#else

#include <varargs.h>

int
fiprintf (fp, fmt, va_alist)
     FILE *fp;
     char *fmt;
     va_dcl
{
  int ret;
  va_list ap;

  va_start (ap);
  ret = vfiprintf (fp, fmt, ap);
  va_end (ap);
  return ret;
}

#endif
