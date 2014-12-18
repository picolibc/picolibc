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
<<fgetws>>, <<fgetws_unlocked>>---get wide character string from a file or stream

INDEX
	fgetws
INDEX
	fgetws_unlocked
INDEX
	_fgetws_r
INDEX
	_fgetws_unlocked_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wchar_t *fgetws(wchar_t *__restrict <[ws]>, int <[n]>,
                        FILE *__restrict <[fp]>);

	#define _GNU_SOURCE
	#include <wchar.h>
	wchar_t *fgetws_unlocked(wchar_t *__restrict <[ws]>, int <[n]>,
                        FILE *__restrict <[fp]>);

	#include <wchar.h>
	wchar_t *_fgetws_r(struct _reent *<[ptr]>, wchar_t *<[ws]>,
                           int <[n]>, FILE *<[fp]>);

	#include <wchar.h>
	wchar_t *_fgetws_unlocked_r(struct _reent *<[ptr]>, wchar_t *<[ws]>,
                           int <[n]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wchar_t *fgetws(<[ws]>,<[n]>,<[fp]>)
	wchar_t *__restrict <[ws]>;
	int <[n]>;
	FILE *__restrict <[fp]>;

	#define _GNU_SOURCE
	#include <wchar.h>
	wchar_t *fgetws_unlocked(<[ws]>,<[n]>,<[fp]>)
	wchar_t *__restrict <[ws]>;
	int <[n]>;
	FILE *__restrict <[fp]>;

	#include <wchar.h>
	wchar_t *_fgetws_r(<[ptr]>, <[ws]>,<[n]>,<[fp]>)
	struct _reent *<[ptr]>;
	wchar_t *<[ws]>;
	int <[n]>;
	FILE *<[fp]>;

	#include <wchar.h>
	wchar_t *_fgetws_unlocked_r(<[ptr]>, <[ws]>,<[n]>,<[fp]>)
	struct _reent *<[ptr]>;
	wchar_t *<[ws]>;
	int <[n]>;
	FILE *<[fp]>;

DESCRIPTION
Reads at most <[n-1]> wide characters from <[fp]> until a newline
is found. The wide characters including to the newline are stored
in <[ws]>. The buffer is terminated with a 0.

<<fgetws_unlocked>> is a non-thread-safe version of <<fgetws>>.
<<fgetws_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fgetws_unlocked>> is equivalent to <<fgetws>>.

The <<_fgetws_r>> and <<_fgetws_unlocked_r>> functions are simply reentrant
version of the above and are passed an additional reentrancy structure
pointer: <[ptr]>.

RETURNS
<<fgetws>> returns the buffer passed to it, with the data
filled in. If end of file occurs with some data already
accumulated, the data is returned with no other indication. If
no data are read, NULL is returned instead.

PORTABILITY
<<fgetws>> is required by C99 and POSIX.1-2001.

<<fgetws_unlocked>> is a GNU extension.
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fgetws_r _fgetws_unlocked_r
#define fgetws fgetws_unlocked
#endif

wchar_t *
_DEFUN(_fgetws_r, (ptr, ws, n, fp),
	struct _reent *ptr _AND
	wchar_t * ws _AND
	int n _AND
	FILE * fp)
{
  wchar_t *wsp;
  size_t nconv;
  const char *src;
  unsigned char *nl;

  _newlib_flockfile_start (fp);
  ORIENT (fp, 1);

  if (n <= 0)
    {
      errno = EINVAL;
      goto error;
    }

  if (fp->_r <= 0 && __srefill_r (ptr, fp))
    /* EOF */
    goto error;
  wsp = ws;
  do
    {
      src = (char *) fp->_p;
      nl = memchr (fp->_p, '\n', fp->_r);
      nconv = _mbsnrtowcs_r (ptr, wsp, &src,
			     /* Read all bytes up to the next NL, or up to the
				end of the buffer if there is no NL. */
			     nl != NULL ? (nl - fp->_p + 1) : fp->_r,
			     /* But never more than n - 1 wide chars. */
			     n - 1,
			     &fp->_mbstate);
      if (nconv == (size_t) -1)
	/* Conversion error */
	goto error;
      if (src == NULL)
	{
	  /*
	   * We hit a null byte. Increment the character count,
	   * since mbsnrtowcs()'s return value doesn't include
	   * the terminating null, then resume conversion
	   * after the null.
	   */
	  nconv++;
	  src = memchr (fp->_p, '\0', fp->_r);
	  src++;
	}
      fp->_r -= (unsigned char *) src - fp->_p;
      fp->_p = (unsigned char *) src;
      n -= nconv;
      wsp += nconv;
    }
  while (wsp[-1] != L'\n' && n > 1 && (fp->_r > 0
	 || __srefill_r (ptr, fp) == 0));
  if (wsp == ws)
    /* EOF */
    goto error;
  if (!mbsinit (&fp->_mbstate))
    /* Incomplete character */
    goto error;
  *wsp++ = L'\0';
  _newlib_flockfile_exit (fp);
  return ws;

error:
  _newlib_flockfile_end (fp);
  return NULL;
}

wchar_t *
_DEFUN(fgetws, (ws, n, fp),
	wchar_t *__restrict ws _AND
	int n _AND
	FILE *__restrict fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT (reent, fp);
  return _fgetws_r (reent, ws, n, fp);
}
