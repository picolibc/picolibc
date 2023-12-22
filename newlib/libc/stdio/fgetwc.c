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
<<fgetwc>>, <<getwc>>, <<fgetwc_unlocked>>, <<getwc_unlocked>>---get a wide character from a file or stream

INDEX
	fgetwc
INDEX
	fgetwc_unlocked
INDEX
	_fgetwc_r
INDEX
	_fgetwc_unlocked_r
INDEX
	getwc
INDEX
	getwc_unlocked
INDEX
	_getwc_r
INDEX
	_getwc_unlocked_r

SYNOPSIS
	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc(FILE *<[fp]>);

	#define _GNU_SOURCE
	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc_unlocked(FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc( FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t fgetwc_unlocked( FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc(FILE *<[fp]>);

	#define _GNU_SOURCE
	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc_unlocked(FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc( FILE *<[fp]>);

	#include <stdio.h>
	#include <wchar.h>
	wint_t getwc_unlocked( FILE *<[fp]>);

DESCRIPTION
Use <<fgetwc>> to get the next wide character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetwc>> advances the file's
current position indicator.

<<fgetwc_unlocked>> is a non-thread-safe version of <<fgetwc>>.
<<fgetwc_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fgetwc_unlocked>> is equivalent to <<fgetwc>>.

The <<getwc>> and <<getwc_unlocked>> functions or macros functions identically
to <<fgetwc>> and <<fgetwc_unlocked>>.  It may be implemented as a macro, and
may evaluate its argument more than once. There is no reason ever to use it.

<<_fgetwc_r>>, <<_getwc_r>>, <<_fgetwc_unlocked_r>>, and <<_getwc_unlocked_r>>
are simply reentrant versions of the above functions that are passed the
additional reentrant structure pointer argument: <[ptr]>.

RETURNS
The next wide character cast to <<wint_t>>, unless there is no more data,
or the host system reports a read error; in either of these situations,
<<fgetwc>> and <<getwc>> return <<WEOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
<<fgetwc>> and <<getwc>> are required by C99 and POSIX.1-2001.

<<fgetwc_unlocked>> and <<getwc_unlocked>> are GNU extensions.
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "local.h"

wint_t
__fgetwc (
	register FILE *fp)
{
  wchar_t wc;
  size_t nconv;

  if (fp->_r <= 0 && _srefill ( fp))
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
      nconv = mbrtowc (&wc, (char *) fp->_p, fp->_r, &fp->_mbstate);
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
  while (_srefill( fp) == 0);
  fp->_flags |= __SERR;
  errno = EILSEQ;
  return (WEOF);
}

wint_t
fgetwc (
	register FILE *fp)
{
  wint_t r;

  _newlib_flockfile_start (fp);
  if (ORIENT(fp, 1) != 1)
    r = WEOF;
  else
    r = __fgetwc (fp);
  _newlib_flockfile_end (fp);
  return r;
}
