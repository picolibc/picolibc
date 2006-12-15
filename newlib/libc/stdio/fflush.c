/*
 * Copyright (c) 1990, 2006 The Regents of the University of California.
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
<<fflush>>---flush buffered file output

INDEX
	fflush

ANSI_SYNOPSIS
	#include <stdio.h>
	int fflush(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fflush(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
The <<stdio>> output functions can buffer output before delivering it
to the host system, in order to minimize the overhead of system calls.

Use <<fflush>> to deliver any such pending output (for the file
or stream identified by <[fp]>) to the host system.

If <[fp]> is <<NULL>>, <<fflush>> delivers pending output from all
open files.

RETURNS
<<fflush>> returns <<0>> unless it encounters a write error; in that
situation, it returns <<EOF>>.

PORTABILITY
ANSI C requires <<fflush>>.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <stdio.h>
#include "local.h"

/* Flush a single file, or (if fp is NULL) all files.  */

int
_DEFUN(fflush, (fp),
       register FILE * fp)
{
  register unsigned char *p;
  register int n, t;

  if (fp == NULL)
    return _fwalk (_GLOBAL_REENT, fflush);

  CHECK_INIT (_REENT);

  _flockfile (fp);

  t = fp->_flags;
  if ((t & __SWR) == 0)
    {
      _fpos_t _EXFUN((*seekfn), (_PTR, _fpos_t, int));

      /* For a read stream, an fflush causes the next seek to be
         unoptimized (i.e. forces a system-level seek).  This conforms
         to the POSIX and SUSv3 standards.  */
      fp->_flags |= __SNPT;

      /* For a seekable stream with buffered read characters, we will attempt
         a seek to the current position now.  A subsequent read will then get
         the next byte from the file rather than the buffer.  This conforms
         to the POSIX and SUSv3 standards.  Note that the standards allow
         this seek to be deferred until necessary, but we choose to do it here
         to make the change simpler, more contained, and less likely
         to miss a code scenario.  */
      if ((fp->_r > 0 || fp->_ur > 0) && (seekfn = fp->_seek) != NULL)
        {
          _fpos_t curoff;

          /* Get the physical position we are at in the file.  */
          if (fp->_flags & __SOFF)
            curoff = fp->_offset;
          else
            {
              /* We don't know current physical offset, so ask for it.  */
              curoff = (*seekfn) (fp->_cookie, (_fpos_t) 0, SEEK_CUR);
              if (curoff == -1L)
                {
                  _funlockfile (fp);
                  return 0;
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
          if ((*seekfn)(fp->_cookie, curoff, SEEK_SET) != -1)
            {
              /* Seek successful.  We can clear read buffer now.  */
              fp->_flags &= ~__SNPT;
              fp->_r = 0;
              fp->_p = fp->_bf._base;
              if (fp->_flags & __SOFF)
                fp->_offset = curoff;
            }
        } 
      _funlockfile (fp);
      return 0;
    }
  if ((p = fp->_bf._base) == NULL)
    {
      /* Nothing to flush.  */
      _funlockfile (fp);
      return 0;
    }
  n = fp->_p - p;		/* write this much */

  /*
   * Set these immediately to avoid problems with longjmp
   * and to allow exchange buffering (via setvbuf) in user
   * write function.
   */
  fp->_p = p;
  fp->_w = t & (__SLBF | __SNBF) ? 0 : fp->_bf._size;

  while (n > 0)
    {
      t = (*fp->_write) (fp->_cookie, (char *) p, n);
      if (t <= 0)
	{
          fp->_flags |= __SERR;
          _funlockfile (fp);
          return EOF;
	}
      p += t;
      n -= t;
    }
  _funlockfile (fp);
  return 0;
}
