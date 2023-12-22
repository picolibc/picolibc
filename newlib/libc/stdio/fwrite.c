/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
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
<<fwrite>>, <<fwrite_unlocked>>---write array elements

INDEX
	fwrite
INDEX
	fwrite_unlocked
INDEX
	_fwrite_r
INDEX
	_fwrite_unlocked_r

SYNOPSIS
	#include <stdio.h>
	size_t fwrite(const void *restrict <[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *restrict <[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	size_t fwrite_unlocked(const void *restrict <[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *restrict <[fp]>);

	#include <stdio.h>
	size_t fwrite( const void *restrict <[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *restrict <[fp]>);

	#include <stdio.h>
	size_t fwrite_unlocked( const void *restrict <[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *restrict <[fp]>);

DESCRIPTION
<<fwrite>> attempts to copy, starting from the memory location
<[buf]>, <[count]> elements (each of size <[size]>) into the file or
stream identified by <[fp]>.  <<fwrite>> may copy fewer elements than
<[count]> if an error intervenes.

<<fwrite>> also advances the file position indicator (if any) for
<[fp]> by the number of @emph{characters} actually written.

<<fwrite_unlocked>> is a non-thread-safe version of <<fwrite>>.
<<fwrite_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fwrite_unlocked>> is equivalent to <<fwrite>>.

<<_fwrite_r>> and <<_fwrite_unlocked_r>> are simply reentrant versions of the
above that take an additional reentrant structure argument: <[ptr]>.

RETURNS
If <<fwrite>> succeeds in writing all the elements you specify, the
result is the same as the argument <[count]>.  In any event, the
result is the number of complete elements that <<fwrite>> copied to
the file.

PORTABILITY
ANSI C requires <<fwrite>>.

<<fwrite_unlocked>> is a BSD extension also provided by GNU libc.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "%W% (Berkeley) %G%";
#endif /* LIBC_SCCS and not lint */

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#if 0
#include <sys/stdc.h>
#endif
#include "local.h"
#if 1
#include "fvwrite.h"
#endif

#ifdef __IMPL_UNLOCKED__
#define _fwrite_r _fwrite_unlocked_r
#define fwrite fwrite_unlocked
#endif

/*
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */

size_t
fwrite (
       const void *__restrict buf,
       size_t size,
       size_t count,
       FILE * __restrict fp)
{
  size_t n;
#ifdef _FVWRITE_IN_STREAMIO
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = buf;
  if (!(uio.uio_resid = iov.iov_len = n = count * size))
      return 0;
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  /*
   * The usual case is success (__sfvwrite_r returns 0);
   * skip the divide if this happens, since divides are
   * generally slow and since this occurs whenever size==0.
   */

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  if (ORIENT (fp, -1) != -1)
    {
      _newlib_flockfile_exit (fp);
      return 0;
    }
  if (_sfvwrite (fp, &uio) == 0)
    {
      _newlib_flockfile_exit (fp);
      return count;
    }
  _newlib_flockfile_end (fp);
  return (n - uio.uio_resid) / size;
#else
  size_t i = 0;
  const char *p = buf;
  if (!(n = count * size))
      return 0;
  CHECK_INIT (ptr, fp);

  _newlib_flockfile_start (fp);
  /* Make sure we can write.  */
  if (cantwrite (ptr, fp))
    goto ret;

  while (i < n)
    {
      if (_sputc ( p[i], fp) == EOF)
	break;

      i++;
    }

ret:
  _newlib_flockfile_end (fp);
  return i / size;
#endif
}
