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
<<getwchar>>, <<getwchar_unlocked>>---read a wide character from standard input

INDEX
	getwchar
INDEX
	getwchar_unlocked
INDEX
	_getwchar_r
INDEX
	_getwchar_unlocked_r

SYNOPSIS
	#include <wchar.h>
	wint_t getwchar(void);

	#define _GNU_SOURCE
	#include <wchar.h>
	wint_t getwchar_unlocked(void);

	#include <wchar.h>
	wint_t _getwchar_r(struct _reent *<[reent]>);

	#include <wchar.h>
	wint_t _getwchar_unlocked_r(struct _reent *<[reent]>);

DESCRIPTION
<<getwchar>> function or macro is the wide character equivalent of
the <<getchar>> function.  You can use <<getwchar>> to get the next
wide character from the standard input stream.  As a side effect,
<<getwchar>> advances the standard input's current position indicator.

<<getwchar_unlocked>> is a non-thread-safe version of <<getwchar>>.
<<getwchar_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<getwchar_unlocked>> is equivalent to <<getwchar>>.

The alternate functions <<_getwchar_r>> and <<_getwchar_unlocked_r>> are
reentrant versions of the above.  The extra argument <[reent]> is a pointer to
a reentrancy structure.

RETURNS
The next wide character cast to <<wint_t>>, unless there is no more
data, or the host system reports a read error; in either of these
situations, <<getwchar>> returns <<WEOF>>.

You can distinguish the two situations that cause an <<WEOF>> result by
using `<<ferror(stdin)>>' and `<<feof(stdin)>>'.

PORTABILITY
<<getwchar>> is required by C99.

<<getwchar_unlocked>> is a GNU extension.
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"

#undef getwchar

wint_t
_getwchar_r (struct _reent *ptr)
{
  return _fgetwc_r (ptr, stdin);
}

/*
 * Synonym for fgetwc(stdin).
 */
wint_t
getwchar (void)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fgetwc (stdin);
}
