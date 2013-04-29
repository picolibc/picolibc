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
<<fgetwc>>, <<getwc>>---get a wide character from a file or stream

INDEX
	fgetwc
INDEX
	_fgetwc_r
INDEX
	getwc
INDEX
	_getwc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc(FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fgetwc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc(FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t _getwc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc(<[fp]>)
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t _fgetwc_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc(<[fp]>)
	FILE *<[fp]>;

	#include <stdio.h>
	#include <wchar.h>
	wint_t _getwc_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
Use <<fgetwc>> to get the next wide character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetwc>> advances the file's
current position indicator.

The  <<getwc>>  function  or macro functions identically to <<fgetwc>>.  It
may be implemented as a macro, and may evaluate its argument more  than
once. There is no reason ever to use it.

<<_fgetwc_r>> and <<_getwc_r>> are simply reentrant versions of
<<fgetwc>> and <<getwc>> that are passed the additional reentrant
structure pointer argument: <[ptr]>.

RETURNS
The next wide character cast to <<wint_t>>), unless there is no more data,
or the host system reports a read error; in either of these situations,
<<fgetwc>> and <<getwc>> return <<WEOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
C99, POSIX.1-2001
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

static wint_t
_DEFUN(__fgetwc, (ptr, fp),
	struct _reent *ptr _AND
	register FILE *fp)
{
  wchar_t wc;
  size_t nconv;

  if (fp->_r <= 0 && __srefill_r (ptr, fp))
    return (WEOF);
  if (MB_CUR_MAX == 1)
    {
      /* Fast path for single-byte encodings. */
      wc = *fp->_p++;
      fp->_r--;
      return (wc);
    }
  do
    {
      nconv = _mbrtowc_r (ptr, &wc, (char *) fp->_p, fp->_r, &fp->_mbstate);
      if (nconv == (size_t)-1)
	break;
      else if (nconv == (size_t)-2)
	continue;
      else if (nconv == 0)
	{
	  /*
	   * Assume that the only valid representation of
	   * the null wide character is a single null byte.
	   */
	  fp->_p++;
	  fp->_r--;
	  return (L'\0');
	}
      else
        {
	  fp->_p += nconv;
	  fp->_r -= nconv;
	  return (wc);
	}
    }
  while (__srefill_r(ptr, fp) == 0);
  fp->_flags |= __SERR;
  errno = EILSEQ;
  return (WEOF);
}

wint_t
_DEFUN(_fgetwc_r, (ptr, fp),
	struct _reent *ptr _AND
	register FILE *fp)
{
  wint_t r;

  _newlib_flockfile_start (fp);
  ORIENT(fp, 1);
  r = __fgetwc (ptr, fp);
  _newlib_flockfile_end (fp);
  return r;
}

wint_t
_DEFUN(fgetwc, (fp),
	FILE *fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT(reent, fp);
  return _fgetwc_r (reent, fp);
}
