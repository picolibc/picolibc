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
<<fputws>>, <<fputws_unlocked>>---write a wide character string in a file or stream

INDEX
	fputws   
INDEX
	fputws_unlocked 
INDEX
	_fputws_r
INDEX
	_fputws_unlocked_r

SYNOPSIS
	#include <wchar.h>
	int fputws(const wchar_t *__restrict <[ws]>, FILE *__restrict <[fp]>);

	#define _GNU_SOURCE
	#include <wchar.h>
	int fputws_unlocked(const wchar_t *__restrict <[ws]>, FILE *__restrict <[fp]>);

	#include <wchar.h>
	int fputws( const wchar_t *<[ws]>,
                      FILE *<[fp]>);

	#include <wchar.h>
	int fputws_unlocked( const wchar_t *<[ws]>,
                               FILE *<[fp]>);

DESCRIPTION
<<fputws>> writes the wide character string at <[ws]> (but without the
trailing null) to the file or stream identified by <[fp]>.

<<fputws_unlocked>> is a non-thread-safe version of <<fputws>>.
<<fputws_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fputws_unlocked>> is equivalent to <<fputws>>.

<<_fputws_r>> and <<_fputws_unlocked_r>> are simply reentrant versions of the
above that take an additional reentrant struct pointer argument: <[ptr]>.

RETURNS
If successful, the result is a non-negative integer; otherwise, the result
is <<-1>> to indicate an error.

PORTABILITY
<<fputws>> is required by C99 and POSIX.1-2001.

<<fputws_unlocked>> is a GNU extension.
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <wchar.h>
#include "fvwrite.h"
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fputws_r _fputws_unlocked_r
#define fputws fputws_unlocked
#endif

int
fputws (
	const wchar_t *ws,
	FILE *fp)
{
  size_t nbytes;
  char buf[BUFSIZ];
#ifdef _FVWRITE_IN_STREAMIO
  struct __suio uio;
  struct __siov iov;

  _newlib_flockfile_start (fp);
  if (ORIENT (fp, 1) != 1)
    goto error;
  if (cantwrite (ptr, fp) != 0)
    goto error;
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;
  iov.iov_base = buf;
  do
    {
      nbytes = wcsrtombs(buf, &ws, sizeof (buf), &fp->_mbstate);
      if (nbytes == (size_t) -1)
	goto error;
      iov.iov_len = uio.uio_resid = nbytes;
      if (_sfvwrite( fp, &uio) != 0)
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
  if (cantwrite (ptr, fp) != 0)
    goto error;

  do
    {
      size_t i = 0;
      nbytes = wcsrtombs (buf, &ws, sizeof (buf), &fp->_mbstate);
      if (nbytes == (size_t) -1)
	goto error;
      while (i < nbytes)
        {
	  if (__swputc(buf[i], fp) == EOF)
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
