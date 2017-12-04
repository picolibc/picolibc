/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
FUNCTION
<<fputs>>, <<fputs_unlocked>>---write a character string in a file or stream

INDEX
	fputs
INDEX
	fputs_unlocked
INDEX
	_fputs_r
INDEX
	_fputs_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int fputs(const char *restrict <[s]>, FILE *restrict <[fp]>);

	#define _GNU_SOURCE
	#include <stdio.h>
	int fputs_unlocked(const char *restrict <[s]>, FILE *restrict <[fp]>);

	#include <stdio.h>
	int _fputs_r(struct _reent *<[ptr]>, const char *restrict <[s]>, FILE *restrict <[fp]>);

	#include <stdio.h>
	int _fputs_unlocked_r(struct _reent *<[ptr]>, const char *restrict <[s]>, FILE *restrict <[fp]>);

DESCRIPTION
<<fputs>> writes the string at <[s]> (but without the trailing null)
to the file or stream identified by <[fp]>.

<<fputs_unlocked>> is a non-thread-safe version of <<fputs>>.
<<fputs_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fputs_unlocked>> is equivalent to <<fputs>>.

<<_fputs_r>> and <<_fputs_unlocked_r>> are simply reentrant versions of the
above that take an additional reentrant struct pointer argument: <[ptr]>.

RETURNS
If successful, the result is <<0>>; otherwise, the result is <<EOF>>.

PORTABILITY
ANSI C requires <<fputs>>, but does not specify that the result on
success must be <<0>>; any non-negative value is permitted.

<<fputs_unlocked>> is a GNU extension.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include "fvwrite.h"
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fputs_r _fputs_unlocked_r
#define fputs fputs_unlocked
#endif

/*
 * Write the given string to the given file.
 */
int
_fputs_r (struct _reent * ptr,
       char const *__restrict s,
       FILE *__restrict fp)
{
#ifdef _FVWRITE_IN_STREAMIO
  int result;
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = s;
  iov.iov_len = uio.uio_resid = strlen (s);
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  ORIENT (fp, -1);
  result = __sfvwrite_r (ptr, fp, &uio);
  _newlib_flockfile_end (fp);
  return result;
#else
  const char *p = s;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  ORIENT (fp, -1);
  /* Make sure we can write.  */
  if (cantwrite (ptr, fp))
    goto error;

  while (*p)
    {
      if (__sputc_r (ptr, *p++, fp) == EOF)
	goto error;
    }
  _newlib_flockfile_exit (fp);
  return 0;

error:
  _newlib_flockfile_end (fp);
  return EOF;
#endif
}

#ifndef _REENT_ONLY
int
fputs (char const *__restrict s,
       FILE *__restrict fp)
{
  return _fputs_r (_REENT, s, fp);
}
#endif /* !_REENT_ONLY */
