/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
<<fread>>, <<fread_unlocked>>---read array elements from a file

INDEX
	fread
INDEX
	fread_unlocked
INDEX
	_fread_r
INDEX
	_fread_unlocked_r

SYNOPSIS
	#include <stdio.h>
	size_t fread(void *restrict <[buf]>, size_t <[size]>, size_t <[count]>,
		     FILE *restrict <[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	size_t fread_unlocked(void *restrict <[buf]>, size_t <[size]>, size_t <[count]>,
		     FILE *restrict <[fp]>);

	#include <stdio.h>
	size_t _fread_r(struct _reent *<[ptr]>, void *restrict <[buf]>,
	                size_t <[size]>, size_t <[count]>, FILE *restrict <[fp]>);

	#include <stdio.h>
	size_t _fread_unlocked_r(struct _reent *<[ptr]>, void *restrict <[buf]>,
	                size_t <[size]>, size_t <[count]>, FILE *restrict <[fp]>);

DESCRIPTION
<<fread>> attempts to copy, from the file or stream identified by
<[fp]>, <[count]> elements (each of size <[size]>) into memory,
starting at <[buf]>.   <<fread>> may copy fewer elements than
<[count]> if an error, or end of file, intervenes.

<<fread>> also advances the file position indicator (if any) for
<[fp]> by the number of @emph{characters} actually read.

<<fread_unlocked>> is a non-thread-safe version of <<fread>>.
<<fread_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fread_unlocked>> is equivalent to <<fread>>.

<<_fread_r>> and <<_fread_unlocked_r>> are simply reentrant versions of the
above that take an additional reentrant structure pointer argument: <[ptr]>.

RETURNS
The result of <<fread>> is the number of elements it succeeded in
reading.

PORTABILITY
ANSI C requires <<fread>>.

<<fread_unlocked>> is a BSD extension also provided by GNU libc.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fread_r _fread_unlocked_r
#define fread fread_unlocked
#endif

#ifdef __SCLE
static size_t
crlf_r (struct _reent * ptr,
       FILE * fp,
       char * buf,
       size_t count,
       int eof)
{
  int r;
  char *s, *d, *e;

  if (count == 0)
    return 0;

  e = buf + count;
  for (s=d=buf; s<e-1; s++)
    {
      if (*s == '\r' && s[1] == '\n')
        s++;
      *d++ = *s;
    }
  if (s < e)
    {
      if (*s == '\r')
        {
          int c = __sgetc_raw_r (ptr, fp);
          if (c == '\n')
            *s = '\n';
          else
            ungetc (c, fp);
        }
      *d++ = *s++;
    }


  while (d < e)
    {
      r = _getc_r (ptr, fp);
      if (r == EOF)
        return count - (e-d);
      *d++ = r;
    }

  return count;
  
}

#endif

size_t
_fread_r (struct _reent * ptr,
       void *__restrict buf,
       size_t size,
       size_t count,
       FILE * __restrict fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;

  CHECK_INIT(ptr, fp);

  _newlib_flockfile_start (fp);
  ORIENT (fp, -1);
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)

  /* Optimize unbuffered I/O.  */
  if (fp->_flags & __SNBF)
    {
      /* First copy any available characters from ungetc buffer.  */
      int copy_size = resid > fp->_r ? fp->_r : resid;
      (void) memcpy ((void *) p, (void *) fp->_p, (size_t) copy_size);
      fp->_p += copy_size;
      fp->_r -= copy_size;
      p += copy_size;
      resid -= copy_size;

      /* If still more data needed, free any allocated ungetc buffer.  */
      if (HASUB (fp) && resid > 0)
	FREEUB (ptr, fp);

      /* Finally read directly into user's buffer if needed.  */
      while (resid > 0)
	{
	  int rc = 0;
	  /* save fp buffering state */
	  void *old_base = fp->_bf._base;
	  void * old_p = fp->_p;
	  int old_size = fp->_bf._size;
	  /* allow __refill to use user's buffer */
	  fp->_bf._base = (unsigned char *) p;
	  fp->_bf._size = resid;
	  fp->_p = (unsigned char *) p;
	  rc = __srefill_r (ptr, fp);
	  /* restore fp buffering back to original state */
	  fp->_bf._base = old_base;
	  fp->_bf._size = old_size;
	  fp->_p = old_p;
	  resid -= fp->_r;
	  p += fp->_r;
	  fp->_r = 0;
	  if (rc)
	    {
#ifdef __SCLE
              if (fp->_flags & __SCLE)
	        {
	          _newlib_flockfile_exit (fp);
	          return crlf_r (ptr, fp, buf, total-resid, 1) / size;
	        }
#endif
	      _newlib_flockfile_exit (fp);
	      return (total - resid) / size;
	    }
	}
    }
  else
#endif /* !PREFER_SIZE_OVER_SPEED && !__OPTIMIZE_SIZE__ */
    {
      while (resid > (r = fp->_r))
	{
	  (void) memcpy ((void *) p, (void *) fp->_p, (size_t) r);
	  fp->_p += r;
	  /* fp->_r = 0 ... done in __srefill */
	  p += r;
	  resid -= r;
	  if (__srefill_r (ptr, fp))
	    {
	      /* no more input: return partial result */
#ifdef __SCLE
	      if (fp->_flags & __SCLE)
		{
		  _newlib_flockfile_exit (fp);
		  return crlf_r (ptr, fp, buf, total-resid, 1) / size;
		}
#endif
	      _newlib_flockfile_exit (fp);
	      return (total - resid) / size;
	    }
	}
      (void) memcpy ((void *) p, (void *) fp->_p, resid);
      fp->_r -= resid;
      fp->_p += resid;
    }

  /* Perform any CR/LF clean-up if necessary.  */
#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      _newlib_flockfile_exit (fp);
      return crlf_r(ptr, fp, buf, total, 0) / size;
    }
#endif
  _newlib_flockfile_end (fp);
  return count;
}

#ifndef _REENT_ONLY
size_t
fread (void *__restrict  buf,
       size_t size,
       size_t count,
       FILE *__restrict fp)
{
   return _fread_r (_REENT, buf, size, count, fp);
}
#endif
