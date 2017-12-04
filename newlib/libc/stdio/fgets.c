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
<<fgets>>, <<fgets_unlocked>>---get character string from a file or stream

INDEX
	fgets
INDEX
	fgets_unlocked
INDEX
	_fgets_r
INDEX
	_fgets_unlocked_r

SYNOPSIS
        #include <stdio.h>
	char *fgets(char *restrict <[buf]>, int <[n]>, FILE *restrict <[fp]>);

	#define _GNU_SOURCE
        #include <stdio.h>
	char *fgets_unlocked(char *restrict <[buf]>, int <[n]>, FILE *restrict <[fp]>);

        #include <stdio.h>
	char *_fgets_r(struct _reent *<[ptr]>, char *restrict <[buf]>, int <[n]>, FILE *restrict <[fp]>);

        #include <stdio.h>
	char *_fgets_unlocked_r(struct _reent *<[ptr]>, char *restrict <[buf]>, int <[n]>, FILE *restrict <[fp]>);

DESCRIPTION
	Reads at most <[n-1]> characters from <[fp]> until a newline
	is found. The characters including to the newline are stored
	in <[buf]>. The buffer is terminated with a 0.

	<<fgets_unlocked>> is a non-thread-safe version of <<fgets>>.
	<<fgets_unlocked>> may only safely be used within a scope
	protected by flockfile() (or ftrylockfile()) and funlockfile().  This
	function may safely be used in a multi-threaded program if and only
	if they are called while the invoking thread owns the (FILE *)
	object, as is the case after a successful call to the flockfile() or
	ftrylockfile() functions.  If threads are disabled, then
	<<fgets_unlocked>> is equivalent to <<fgets>>.

	The functions <<_fgets_r>> and <<_fgets_unlocked_r>> are simply
	reentrant versions that are passed the additional reentrant structure
	pointer argument: <[ptr]>.

RETURNS
	<<fgets>> returns the buffer passed to it, with the data
	filled in. If end of file occurs with some data already
	accumulated, the data is returned with no other indication. If
	no data are read, NULL is returned instead.

PORTABILITY
	<<fgets>> should replace all uses of <<gets>>. Note however
	that <<fgets>> returns all of the data, while <<gets>> removes
	the trailing newline (with no indication that it has done so.)

	<<fgets_unlocked>> is a GNU extension.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fgets_r _fgets_unlocked_r
#define fgets fgets_unlocked
#endif

/*
 * Read at most n-1 characters from the given file.
 * Stop when a newline has been read, or the count runs out.
 * Return first argument, or NULL if no characters were read.
 */

char *
_fgets_r (struct _reent * ptr,
       char *__restrict buf,
       int n,
       FILE *__restrict fp)
{
  size_t len;
  char *s;
  unsigned char *p, *t;

  if (n < 2)			/* sanity check */
    return 0;

  s = buf;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      int c = 0;
      /* Sorry, have to do it the slow way */
      while (--n > 0 && (c = __sgetc_r (ptr, fp)) != EOF)
	{
	  *s++ = c;
	  if (c == '\n')
	    break;
	}
      if (c == EOF && s == buf)
        {
          _newlib_flockfile_exit (fp);
          return NULL;
        }
      *s = 0;
      _newlib_flockfile_exit (fp);
      return buf;
    }
#endif

  n--;				/* leave space for NUL */
  do
    {
      /*
       * If the buffer is empty, refill it.
       */
      if ((len = fp->_r) <= 0)
	{
	  if (__srefill_r (ptr, fp))
	    {
	      /* EOF: stop with partial or no line */
	      if (s == buf)
                {
                  _newlib_flockfile_exit (fp);
                  return 0;
                }
	      break;
	    }
	  len = fp->_r;
	}
      p = fp->_p;

      /*
       * Scan through at most n bytes of the current buffer,
       * looking for '\n'.  If found, copy up to and including
       * newline, and stop.  Otherwise, copy entire chunk
       * and loop.
       */
      if (len > n)
	len = n;
      t = (unsigned char *) memchr ((void *) p, '\n', len);
      if (t != 0)
	{
	  len = ++t - p;
	  fp->_r -= len;
	  fp->_p = t;
	  (void) memcpy ((void *) s, (void *) p, len);
	  s[len] = 0;
          _newlib_flockfile_exit (fp);
	  return (buf);
	}
      fp->_r -= len;
      fp->_p += len;
      (void) memcpy ((void *) s, (void *) p, len);
      s += len;
    }
  while ((n -= len) != 0);
  *s = 0;
  _newlib_flockfile_end (fp);
  return buf;
}

#ifndef _REENT_ONLY

char *
fgets (char *__restrict buf,
       int n,
       FILE *__restrict fp)
{
  return _fgets_r (_REENT, buf, n, fp);
}

#endif /* !_REENT_ONLY */
