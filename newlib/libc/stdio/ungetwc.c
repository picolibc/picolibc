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
<<ungetwc>>---push wide character data back into a stream

INDEX
        ungetwc
INDEX
        _ungetwc_r

SYNOPSIS
        #include <stdio.h>
        #include <wchar.h>
        wint_t ungetwc(wint_t <[wc]>, FILE *<[stream]>);

        wint_t _ungetwc_r(struct _reent *<[reent]>, wint_t <[wc]>, FILE *<[stream]>);

DESCRIPTION
<<ungetwc>> is used to return wide characters back to <[stream]> to be
read again.  If <[wc]> is WEOF, the stream is unchanged.  Otherwise, the
wide character <[wc]> is put back on the stream, and subsequent reads will see
the wide chars pushed back in reverse order.  Pushed wide chars are lost if the
stream is repositioned, such as by <<fseek>>, <<fsetpos>>, or
<<rewind>>.

The underlying file is not changed, but it is possible to push back
something different than what was originally read.  Ungetting a
character will clear the end-of-stream marker, and decrement the file
position indicator.  Pushing back beyond the beginning of a file gives
unspecified behavior.

The alternate function <<_ungetwc_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
The wide character pushed back, or <<WEOF>> on error.

PORTABILITY
C99
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
_ungetwc_r (struct _reent *ptr,
	wint_t wc,
	register FILE *fp)
{
  char buf[MB_LEN_MAX];
  size_t len;

  _newlib_flockfile_start (fp);
  ORIENT (fp, 1);
  if (wc == WEOF)
    wc = WEOF;
  else if ((len = _wcrtomb_r(ptr, buf, wc, &fp->_mbstate)) == (size_t)-1)
    {
      fp->_flags |= __SERR;
      wc = WEOF;
    }
  else
    while (len-- != 0)
      if (_ungetc_r(ptr, (unsigned char)buf[len], fp) == EOF)
	{
	  wc = WEOF;
	  break;
	}
  _newlib_flockfile_end (fp);
  return wc;
}

/*
 * MT-safe version.
 */
wint_t
ungetwc (wint_t wc,
	FILE *fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT (reent, fp);
  return _ungetwc_r (reent, wc, fp);
}
