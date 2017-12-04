/*-
 * Copyright (c) 2002-2004 Tim J. Robbins.
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
<<fputwc>>, <<putwc>>, <<fputwc_unlocked>>, <<putwc_unlocked>>---write a wide character on a stream or file

INDEX
	fputwc
INDEX
	fputwc_unlocked
INDEX
	_fputwc_r
INDEX
	_fputwc_unlocked_r
INDEX
	putwc
INDEX
	putwc_unlocked
INDEX
	_putwc_r
INDEX
	_putwc_unlocked_r

SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fputwc(wchar_t <[wc]>, FILE *<[fp]>);

	#define _GNU_SOURCE
	#include <stdio.h>
	#include <wchar.h>
	wint_t fputwc_unlocked(wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fputwc_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fputwc_unlocked_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t putwc(wchar_t <[wc]>, FILE *<[fp]>);

	#define _GNU_SOURCE
	#include <stdio.h>
	#include <wchar.h>
	wint_t putwc_unlocked(wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _putwc_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _putwc_unlocked_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

DESCRIPTION
<<fputwc>> writes the wide character argument <[wc]> to the file or
stream identified by <[fp]>.

If the file was opened with append mode (or if the stream cannot
support positioning), then the new wide character goes at the end of the
file or stream.  Otherwise, the new wide character is written at the
current value of the position indicator, and the position indicator
oadvances by one.

<<fputwc_unlocked>> is a non-thread-safe version of <<fputwc>>.
<<fputwc_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fputwc_unlocked>> is equivalent to <<fputwc>>.

The <<putwc>> and <<putwc_unlocked>> functions or macros function identically
to <<fputwc>> and <<fputwc_unlocked>>.  They may be implemented as a macro, and
may evaluate its argument more than once. There is no reason ever to use them.

The <<_fputwc_r>>, <<_putwc_r>>, <<_fputwc_unlocked_r>>, and
<<_putwc_unlocked_r>> functions are simply reentrant versions of the above
that take an additional reentrant structure argument: <[ptr]>.

RETURNS
If successful, <<fputwc>> and <<putwc>> return their argument <[wc]>.
If an error intervenes, the result is <<EOF>>.  You can use
`<<ferror(<[fp]>)>>' to query for errors.

PORTABILITY
<<fputwc>> and <<putwc>> are required by C99 and POSIX.1-2001.

<<fputwc_unlocked>> and <<putwc_unlocked>> are GNU extensions.
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

wint_t
__fputwc (struct _reent *ptr,
	wchar_t wc,
	FILE *fp)
{
  char buf[MB_LEN_MAX];
  size_t i, len;

  if (MB_CUR_MAX == 1 && wc > 0 && wc <= UCHAR_MAX)
    {
      /*
       * Assume single-byte locale with no special encoding.
       * A more careful test would be to check
       * _CurrentRuneLocale->encoding.
       */
      *buf = (unsigned char)wc;
      len = 1;
    }
  else
    {
      if ((len = _wcrtomb_r (ptr, buf, wc, &fp->_mbstate)) == (size_t) -1)
	{
	  fp->_flags |= __SERR;
	  return WEOF;
	}
    }

  for (i = 0; i < len; i++)
    if (__sputc_r (ptr, (unsigned char) buf[i], fp) == EOF)
      return WEOF;

  return (wint_t) wc;
}

wint_t
_fputwc_r (struct _reent *ptr,
	wchar_t wc,
	FILE *fp)
{
  wint_t r;

  _newlib_flockfile_start (fp);
  ORIENT(fp, 1);
  r = __fputwc(ptr, wc, fp);
  _newlib_flockfile_end (fp);
  return r;
}

wint_t
fputwc (wchar_t wc,
	FILE *fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT(reent, fp);
  return _fputwc_r (reent, wc, fp);
}
