/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
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
<<viprintf>>, <<vfiprintf>>, <<vsiprintf>>, <<vsniprintf>>, <<vasiprintf>>, <<vasniprintf>>---format argument list (integer only)

INDEX
	viprintf
INDEX
	_viprintf_r
INDEX
	vfiprintf
INDEX
	_vfiprintf_r
INDEX
	vsiprintf
INDEX
	_vsiprintf_r
INDEX
	vsniprintf
INDEX
	_vsniprintf_r
INDEX
	vasiprintf
INDEX
	_vasiprintf_r
INDEX
	vasniprintf
INDEX
	_vasniprintf_r

SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int viprintf(const char *<[fmt]>, va_list <[list]>);
	int vfiprintf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsiprintf(char *<[str]>, const char *<[fmt]>, va_list <[list]>);
	int vsniprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>,
                       va_list <[list]>);
	int vasiprintf(char **<[strp]>, const char *<[fmt]>, va_list <[list]>);
	char *vasniprintf(char *<[str]>, size_t *<[size]>, const char *<[fmt]>,
                          va_list <[list]>);

	int viprintf( const char *<[fmt]>,
                        va_list <[list]>);
	int vfiprintf( FILE *<[fp]>,
                        const char *<[fmt]>, va_list <[list]>);
	int vsiprintf( char *<[str]>,
                        const char *<[fmt]>, va_list <[list]>);
	int vsniprintf( char *<[str]>,
                          size_t <[size]>, const char *<[fmt]>, va_list <[list]>);
	int vasiprintf( char **<[str]>,
                          const char *<[fmt]>, va_list <[list]>);
	char *vasniprintf( char *<[str]>,
                             size_t *<[size]>, const char *<[fmt]>, va_list <[list]>);

DESCRIPTION
<<viprintf>>, <<vfiprintf>>, <<vasiprintf>>, <<vsiprintf>>,
<<vsniprintf>>, and <<vasniprintf>> are (respectively) variants of
<<iprintf>>, <<fiprintf>>, <<asiprintf>>, <<siprintf>>, <<sniprintf>>,
and <<asniprintf>>.  They differ only in allowing their caller to pass
the variable argument list as a <<va_list>> object (initialized by
<<va_start>>) rather than directly accepting a variable number of
arguments.  The caller is responsible for calling <<va_end>>.

<<_viprintf_r>>, <<_vfiprintf_r>>, <<_vasiprintf_r>>,
<<_vsiprintf_r>>, <<_vsniprintf_r>>, and <<_vasniprintf_r>> are
reentrant versions of the above.

RETURNS
The return values are consistent with the corresponding functions:

PORTABILITY
All of these functions are newlib extensions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdarg.h>
#include "local.h"

int
viprintf (const char *fmt,
       va_list ap)
{
  _REENT_SMALL_CHECK_INIT (reent);
  return vfiprintf ( _stdout_r (reent), fmt, ap);
}
