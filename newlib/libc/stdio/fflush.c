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
<<fflush>>, <<fflush_unlocked>>---flush buffered file output

INDEX
	fflush
INDEX
	fflush_unlocked
INDEX
	_fflush_r
INDEX
	_fflush_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int fflush(FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int fflush_unlocked(FILE *<[fp]>);

	#include <stdio.h>
	int _fflush_r(struct _reent *<[reent]>, FILE *<[fp]>);

	#define _BSD_SOURCE
	#include <stdio.h>
	int _fflush_unlocked_r(struct _reent *<[reent]>, FILE *<[fp]>);

DESCRIPTION
The <<stdio>> output functions can buffer output before delivering it
to the host system, in order to minimize the overhead of system calls.

Use <<fflush>> to deliver any such pending output (for the file
or stream identified by <[fp]>) to the host system.

If <[fp]> is <<NULL>>, <<fflush>> delivers pending output from all
open files.

Additionally, if <[fp]> is a seekable input stream visiting a file
descriptor, set the position of the file descriptor to match next
unread byte, useful for obeying POSIX semantics when ending a process
without consuming all input from the stream.

<<fflush_unlocked>> is a non-thread-safe version of <<fflush>>.
<<fflush_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  This
function may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the (FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<fflush_unlocked>> is equivalent to <<fflush>>.

The alternate functions <<_fflush_r>> and <<_fflush_unlocked_r>> are
reentrant versions, where the extra argument <[reent]> is a pointer to
a reentrancy structure, and <[fp]> must not be NULL.

RETURNS
<<fflush>> returns <<0>> unless it encounters a write error; in that
situation, it returns <<EOF>>.

PORTABILITY
ANSI C requires <<fflush>>.  The behavior on input streams is only
specified by POSIX, and not all implementations follow POSIX rules.

<<fflush_unlocked>> is a BSD extension also provided by GNU libc.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

#ifdef __IMPL_UNLOCKED__
#define _fflush_r _fflush_unlocked_r
#define fflush fflush_unlocked
#endif

#ifndef __IMPL_UNLOCKED__
/* Flush a single file, or (if fp is NULL) all files.  */

/* Core function which does not lock file pointer.  This gets called
   directly from __srefill. */
int
__sflush_r (struct _reent *ptr,
       register FILE * fp)
{
  register unsigned char *p;
  register _READ_WRITE_BUFSIZE_TYPE n;
  register _READ_WRITE_RETURN_TYPE t;
  short flags;

  flags = fp->_flags;
  if ((flags & __SWR) == 0)
    {
#ifdef _FSEEK_OPTIMIZATION
      /* For a read stream, an fflush causes the next seek to be
         unoptimized (i.e. forces a system-level seek).  This conforms
         to the POSIX and SUSv3 standards.  */
      fp->_flags |= __SNPT;
#endif

      /* For a seekable stream with buffered read characters, we will attempt
         a seek to the current position now.  A subsequent read will then get
         the next byte from the file rather than the buffer.  This conforms
         to the POSIX and SUSv3 standards.  Note that the standards allow
         this seek to be deferred until necessary, but we choose to do it here
         to make the change simpler, more contained, and less likely
         to miss a code scenario.  */
      if ((fp->_r > 0 || fp->_ur > 0) && fp->_seek != NULL)
	{
	  int tmp_errno;
#ifdef __LARGE64_FILES
	  _fpos64_t curoff;
#else
	  _fpos_t curoff;
#endif

	  /* Save last errno and set errno to 0, so we can check if a device
	     returns with a valid position -1.  We restore the last errno if
	     no other error condition has been encountered. */
	  tmp_errno = ptr->_errno;
	  ptr->_errno = 0;
	  /* Get the physical position we are at in the file.  */
	  if (fp->_flags & __SOFF)
	    curoff = fp->_offset;
	  else
	    {
	      /* We don't know current physical offset, so ask for it.
		 Only ESPIPE and EINVAL are ignorable.  */
#ifdef __LARGE64_FILES
	      if (fp->_flags & __SL64)
		curoff = fp->_seek64 (ptr, fp->_cookie, 0, SEEK_CUR);
	      else
#endif
		curoff = fp->_seek (ptr, fp->_cookie, 0, SEEK_CUR);
	      if (curoff == -1L && ptr->_errno != 0)
		{
		  int result = EOF;
		  if (ptr->_errno == ESPIPE || ptr->_errno == EINVAL)
		    {
		      result = 0;
		      ptr->_errno = tmp_errno;
		    }
		  else
		    fp->_flags |= __SERR;
		  return result;
		}
            }
          if (fp->_flags & __SRD)
            {
              /* Current offset is at end of buffer.  Compensate for
                 characters not yet read.  */
              curoff -= fp->_r;
              if (HASUB (fp))
                curoff -= fp->_ur;
            }
	  /* Now physically seek to after byte last read.  */
#ifdef __LARGE64_FILES
	  if (fp->_flags & __SL64)
	    curoff = fp->_seek64 (ptr, fp->_cookie, curoff, SEEK_SET);
	  else
#endif
	    curoff = fp->_seek (ptr, fp->_cookie, curoff, SEEK_SET);
	  if (curoff != -1 || ptr->_errno == 0
	      || ptr->_errno == ESPIPE || ptr->_errno == EINVAL)
	    {
	      /* Seek successful or ignorable error condition.
		 We can clear read buffer now.  */
#ifdef _FSEEK_OPTIMIZATION
	      fp->_flags &= ~__SNPT;
#endif
	      fp->_r = 0;
	      fp->_p = fp->_bf._base;
	      if ((fp->_flags & __SOFF) && (curoff != -1 || ptr->_errno == 0))
		fp->_offset = curoff;
	      ptr->_errno = tmp_errno;
	      if (HASUB (fp))
		FREEUB (ptr, fp);
	    }
	  else
	    {
	      fp->_flags |= __SERR;
	      return EOF;
	    }
	}
      return 0;
    }
  if ((p = fp->_bf._base) == NULL)
    {
      /* Nothing to flush.  */
      return 0;
    }
  n = fp->_p - p;		/* write this much */

  /*
   * Set these immediately to avoid problems with longjmp
   * and to allow exchange buffering (via setvbuf) in user
   * write function.
   */
  fp->_p = p;
  fp->_w = flags & (__SLBF | __SNBF) ? 0 : fp->_bf._size;

  while (n > 0)
    {
      t = fp->_write (ptr, fp->_cookie, (char *) p, n);
      if (t <= 0)
	{
          fp->_flags |= __SERR;
          return EOF;
	}
      p += t;
      n -= t;
    }
  return 0;
}

#ifdef _STDIO_BSD_SEMANTICS
/* Called from _cleanup_r.  At exit time, we don't need file locking,
   and we don't want to move the underlying file pointer unless we're
   writing. */
int
__sflushw_r (struct _reent *ptr,
       register FILE *fp)
{
  return (fp->_flags & __SWR) ?  __sflush_r (ptr, fp) : 0;
}
#endif

#endif /* __IMPL_UNLOCKED__ */

int
_fflush_r (struct _reent *ptr,
       register FILE * fp)
{
  int ret;

#ifdef _REENT_SMALL
  /* For REENT_SMALL platforms, it is possible we are being
     called for the first time on a std stream.  This std
     stream can belong to a reentrant struct that is not
     _REENT.  If CHECK_INIT gets called below based on _REENT,
     we will end up changing said file pointers to the equivalent
     std stream off of _REENT.  This causes unexpected behavior if
     there is any data to flush on the _REENT std stream.  There
     are two alternatives to fix this:  1) make a reentrant fflush
     or 2) simply recognize that this file has nothing to flush
     and return immediately before performing a CHECK_INIT.  Choice
     2 is implemented here due to its simplicity.  */
  if (fp->_bf._base == NULL)
    return 0;
#endif /* _REENT_SMALL  */

  CHECK_INIT (ptr, fp);

  if (!fp->_flags)
    return 0;

  _newlib_flockfile_start (fp);
  ret = __sflush_r (ptr, fp);
  _newlib_flockfile_end (fp);
  return ret;
}

#ifndef _REENT_ONLY

int
fflush (register FILE * fp)
{
  if (fp == NULL)
    return _fwalk_reent (_GLOBAL_REENT, _fflush_r);

  return _fflush_r (_REENT, fp);
}

#endif /* _REENT_ONLY */
