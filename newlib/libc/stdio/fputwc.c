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
<<fputwc>>, <<putwc>>---write a wide character on a stream or file

INDEX
	fputwc
INDEX
	_fputwc_r
INDEX
	putwc
INDEX
	_putwc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fputwc(wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fputwc_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t putwc(wchar_t <[wc]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _putwc_r(struct _reent *<[ptr]>, wchar_t <[wc]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fputwc(<[wc]>, <[fp]>)
	wchar_t <[wc]>;
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fputwc_r(<[ptr]>, <[wc]>, <[fp]>)
	struct _reent *<[ptr]>;
	wchar_t <[wc]>;
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t putwc(<[wc]>, <[fp]>)
	wchar_t <[wc]>;
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t _putwc_r(<[ptr]>, <[wc]>, <[fp]>)
	struct _reent *<[ptr]>;
	wchar_t <[wc]>;
	FILE *<[fp]>;

DESCRIPTION
<<fputwc>> writes the wide character argument <[wc]> to the file or
stream identified by <[fp]>.

If the file was opened with append mode (or if the stream cannot
support positioning), then the new wide character goes at the end of the
file or stream.  Otherwise, the new wide character is written at the
current value of the position indicator, and the position indicator
oadvances by one.

The <<putwc>> function or macro functions identically to <<fputwc>>.  It
may be implemented as a macro, and may evaluate its argument more than
once. There is no reason ever to use it.

The <<_fputwc_r>> and <<_putwc_r>> functions are simply reentrant versions
of <<fputwc>> and <<putwc>> that take an additional reentrant structure
argument: <[ptr]>.

RETURNS
If successful, <<fputwc>> and <<putwc>> return their argument <[wc]>.
If an error intervenes, the result is <<EOF>>.  You can use
`<<ferror(<[fp]>)>>' to query for errors.

PORTABILITY
C99, POSIX.1-2001
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

static wint_t
_DEFUN(__fputwc, (ptr, wc, fp),
	struct _reent *ptr _AND
	wchar_t wc _AND
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
_DEFUN(_fputwc_r, (ptr, wc, fp),
	struct _reent *ptr _AND
	wchar_t wc _AND
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
_DEFUN(fputwc, (wc, fp),
	wchar_t wc _AND
	FILE *fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT(reent, fp);
  return _fputwc_r (reent, wc, fp);
}
