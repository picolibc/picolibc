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
<<getwchar>>---read a wide character from standard input

INDEX
	getwchar
INDEX
	_getwchar_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wint_t getwchar(void);

	wint_t _getwchar_r(struct _reent *<[reent]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wint_t getwchar();

	wint_t _getwchar_r(<[reent]>)
	char * <[reent]>;

DESCRIPTION
<<getwchar>> function or macro is the wide character equivalent of
the <<getchar>> function.  You can use <<getwchar>> to get the next
wide character from the standard input stream.  As a side effect,
<<getwchar>> advances the standard input's current position indicator.

The alternate function <<_getwchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
The next wide character cast to <<wint_t>>, unless there is no more
data, or the host system reports a read error; in either of these
situations, <<getwchar>> returns <<WEOF>>.

You can distinguish the two situations that cause an <<WEOF>> result by
using `<<ferror(stdin)>>' and `<<feof(stdin)>>'.

PORTABILITY
C99
*/

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"

#undef getwchar

wint_t
_DEFUN (_getwchar_r, (ptr),
	struct _reent *ptr)
{
  return _fgetwc_r (ptr, stdin);
}

/*
 * Synonym for fgetwc(stdin).
 */
wint_t
_DEFUN_VOID (getwchar)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fgetwc (stdin);
}
