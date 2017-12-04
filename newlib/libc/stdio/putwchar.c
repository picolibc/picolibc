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
<<putwchar>>, <<putwchar_unlocked>>---write a wide character to standard output

INDEX
	putwchar
INDEX
	putwchar_unlocked
INDEX
	_putwchar_r
INDEX
	_putwchar_unlocked_r

SYNOPSIS
	#include <wchar.h>
	wint_t putwchar(wchar_t <[wc]>);

	#include <wchar.h>
	wint_t putwchar_unlocked(wchar_t <[wc]>);

	#include <wchar.h>
	wint_t _putwchar_r(struct _reent *<[reent]>, wchar_t <[wc]>);

	#include <wchar.h>
	wint_t _putwchar_unlocked_r(struct _reent *<[reent]>, wchar_t <[wc]>);

DESCRIPTION
The <<putwchar>> function or macro is the wide-character equivalent of
the <<putchar>> function. It writes the wide character wc to stdout.

<<putwchar_unlocked>> is a non-thread-safe version of <<putwchar>>.
<<putwchar_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<putwchar_unlocked>> is equivalent to <<putwchar>>.

The alternate functions <<_putwchar_r>> and <<_putwchar_unlocked_r>> are
reentrant versions of the above.  The extra argument <[reent]> is a pointer
to a reentrancy structure.

RETURNS
If successful, <<putwchar>> returns its argument <[wc]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(stdin)>>' to
query for errors.

PORTABILITY
<<putwchar>> is required by C99.

<<putwchar_unlocked>> is a GNU extension.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"

#undef putwchar

wint_t
_putwchar_r (struct _reent *ptr,
	wchar_t wc)
{
  return _fputwc_r (ptr, wc, stdout);
}

/*
 * Synonym for fputwc(wc, stdout).
 */
wint_t
putwchar (wchar_t wc)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fputwc (wc, stdout);
}
