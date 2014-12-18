/*-
 * Copyright (c) 2002 Tim J. Robbins.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<putwchar>>---write a wide character to standard output

INDEX
	putwchar
INDEX
	_putwchar_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wint_t putwchar(wchar_t <[wc]>);

	wint_t _putwchar_r(struct _reent *<[reent]>, wchar_t <[wc]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wint_t putwchar(<[wc]>)
	wchar_t <[wc]>;

	wint_t _putwchar_r(<[reent]>, <[wc]>)
	struct _reent *<[reent]>;
	wchar_t <[wc]>;

DESCRIPTION
The <<putwchar>> function or macro is the wide-character equivalent of
the <<putchar>> function. It writes the wide character wc to stdout.

The alternate function <<_putwchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
If successful, <<putwchar>> returns its argument <[wc]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(stdin)>>' to
query for errors.

PORTABILITY
C99
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"

#undef putwchar

wint_t
_DEFUN(_putwchar_r, (ptr, wc),
	struct _reent *ptr _AND
	wchar_t wc)
{
  return _fputwc_r (ptr, wc, stdout);
}

/*
 * Synonym for fputwc(wc, stdout).
 */
wint_t
_DEFUN(putwchar, (wc),
	wchar_t wc)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fputwc (wc, stdout);
}
