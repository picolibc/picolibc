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
<<viprintf>>, <<vfiprintf>>, <<vsiprintf>>---format argument list

INDEX
	viprintf
INDEX
	vfiprintf
INDEX
	vsiprintf
INDEX
	vsniprintf

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int viprintf(const char *<[fmt]>, va_list <[list]>);
	int vfiprintf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsiprintf(char *<[str]>, const char *<[fmt]>, va_list <[list]>);
	int vasiprintf(char **<[strp]>, const char *<[fmt]>, va_list <[list]>);
	int vsniprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>,
                       va_list <[list]>);

	int _viprintf_r(struct _reent *<[reent]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vfiprintf_r(struct _reent *<[reent]>, FILE *<[fp]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vasiprintf_r(struct _reent *<[reent]>, char **<[str]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vsiprintf_r(struct _reent *<[reent]>, char *<[str]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vsniprintf_r(struct _reent *<[reent]>, char *<[str]>, size_t <[size]>,
                        const char *<[fmt]>, va_list <[list]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <varargs.h>
	int viprintf( <[fmt]>, <[list]>)
	char *<[fmt]>;
	va_list <[list]>;

	int vfiprintf(<[fp]>, <[fmt]>, <[list]>)
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vasiprintf(<[strp]>, <[fmt]>, <[list]>)
	char **<[strp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsiprintf(<[str]>, <[fmt]>, <[list]>)
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsniprintf(<[str]>, <[size]>, <[fmt]>, <[list]>)
	char *<[str]>;
        size_t <[size]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _viprintf_r(<[reent]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vfiprintf_r(<[reent]>, <[fp]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vasiprintf_r(<[reent]>, <[strp]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char **<[strp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsiprintf_r(<[reent]>, <[str]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsniprintf_r(<[reent]>, <[str]>, <[size]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char *<[str]>;
        size_t <[size]>;
	char *<[fmt]>;
	va_list <[list]>;

DESCRIPTION
<<viprintf>>, <<vfiprintf>>, <<vasiprintf>>, <<vsiprintf>> and 
<<vsniprintf>> are (respectively) variants of <<iprintf>>, <<fiprintf>>, 
<<asiprintf>>, <<siprintf>>, and <<sniprintf>>.  They differ only in 
restricting the caller to use non-floating-point format specifiers.

RETURNS
The return values are consistent with the corresponding functions:
<<vasiprintf>>/<<vsiprintf>> returns the number of bytes in the output string,
save that the concluding <<NULL>> is not counted.
<<viprintf>> and <<vfiprintf>> return the number of characters transmitted.
If an error occurs, <<viprintf>> and <<vfiprintf>> return <<EOF>> and
<<vasiprintf>> returns -1.  No error returns occur for <<vsiprintf>>.

PORTABILITY
<<viprintf>>, <<vfiprintf>>, <<vasiprintf>>, <<vsiprintf>> and <<vsniprintf>>
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
#include "local.h"

#ifndef _REENT_ONLY

int
_DEFUN(viprintf, (fmt, ap),
       _CONST char *fmt _AND
       va_list ap)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return _vfiprintf_r (_REENT, _stdout_r (_REENT), fmt, ap);
}

#endif /* !_REENT_ONLY */

int
_DEFUN(_viprintf_r, (ptr, fmt, ap),
       struct _reent *ptr _AND
       _CONST char *fmt   _AND
       va_list ap)
{
  _REENT_SMALL_CHECK_INIT (ptr);
  return _vfiprintf_r (ptr, _stdout_r (ptr), fmt, ap);
}
