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
<<fputws>>---write a wide character string in a file or stream

INDEX
	fputws   
INDEX
	_fputws_r

ANSI_SYNOPSIS
	#include <wchar.h>
	int fputws(const wchar_t *__restrict <[ws]>, FILE *__restrict <[fp]>);

	#include <wchar.h>
	int _fputws_r(struct _reent *<[ptr]>, const wchar_t *<[ws]>,
                      FILE *<[fp]>);

TRAD_SYNOPSIS   
	#include <wchar.h>
	int fputws(<[ws]>, <[fp]>)
	wchar_t *__restrict <[ws]>;
	FILE *__restrict <[fp]>;

	#include <wchar.h>
	int _fputws_r(<[ptr]>, <[ws]>, <[fp]>)
	struct _reent *<[ptr]>;
	wchar_t *<[ws]>;
	FILE *<[fp]>;

DESCRIPTION
<<fputws>> writes the wide character string at <[ws]> (but without the
trailing null) to the file or stream identified by <[fp]>.

<<_fputws_r>> is simply the reentrant version of <<fputws>> that takes
an additional reentrant struct pointer argument: <[ptr]>.

RETURNS
If successful, the result is a non-negative integer; otherwise, the result
is <<-1>> to indicate an error.

PORTABILITY
C99, POSIX.1-2001
*/

#include <_ansi.h>
#include <reent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <wchar.h>
#include "fvwrite.h"
#include "local.h"

int
_DEFUN(_fputws_r, (ptr, ws, fp),
	struct _reent *ptr _AND
	const wchar_t *ws _AND
	FILE *fp)
{
  size_t nbytes;
  char buf[BUFSIZ];
#ifdef _FVWRITE_IN_STREAMIO
  struct __suio uio;
  struct __siov iov;

  _newlib_flockfile_start (fp);
  ORIENT (fp, 1);
  if (cantwrite (ptr, fp) != 0)
    goto error;
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;
  iov.iov_base = buf;
  do
    {
      nbytes = _wcsrtombs_r(ptr, buf, &ws, sizeof (buf), &fp->_mbstate);
      if (nbytes == (size_t) -1)
	goto error;
      iov.iov_len = uio.uio_resid = nbytes;
      if (__sfvwrite_r(ptr, fp, &uio) != 0)
	goto error;
    }
  while (ws != NULL);
  _newlib_flockfile_exit (fp);
  return (0);

error:
  _newlib_flockfile_end (fp);
  return (-1);
#else
  _newlib_flockfile_start (fp);
  ORIENT (fp, 1);
  if (cantwrite (ptr, fp) != 0)
    goto error;

  do
    {
      size_t i = 0;
      nbytes = _wcsrtombs_r (ptr, buf, &ws, sizeof (buf), &fp->_mbstate);
      if (nbytes == (size_t) -1)
	goto error;
      while (i < nbytes)
        {
	  if (__sputc_r (ptr, buf[i], fp) == EOF)
	    goto error;
	  i++;
        }
    }
  while (ws != NULL);
  _newlib_flockfile_exit (fp);
  return (0);

error:
  _newlib_flockfile_end (fp);
  return (-1);
#endif
}

int
_DEFUN(fputws, (ws, fp),
	const wchar_t *__restrict ws _AND
	FILE *__restrict fp)
{
  struct _reent *reent = _REENT;

  CHECK_INIT (reent, fp);
  return _fputws_r (reent, ws, fp);
}
