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
<<freopen64>>---open a large file using an existing file descriptor

INDEX
	freopen64
INDEX
	_freopen64_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *freopen64(const char *<[file]>, const char *<[mode]>,
		        FILE *<[fp]>);
	FILE *_freopen64_r(struct _reent *<[ptr]>, const char *<[file]>, 
		        const char *<[mode]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *freopen64(<[file]>, <[mode]>, <[fp]>)
	char *<[file]>;
	char *<[mode]>;
	FILE *<[fp]>;

	FILE *_freopen64_r(<[ptr]>, <[file]>, <[mode]>, <[fp]>)
	struct _reent *<[ptr]>;
	char *<[file]>;
	char *<[mode]>;
	FILE *<[fp]>;

DESCRIPTION
Use this variant of <<fopen64>> if you wish to specify a particular file
descriptor <[fp]> (notably <<stdin>>, <<stdout>>, or <<stderr>>) for
the file.

If <[fp]> was associated with another file or stream, <<freopen64>>
closes that other file or stream (but ignores any errors while closing
it).

<[file]> and <[mode]> are used just as in <<fopen>>.

RETURNS
If successful, the result is the same as the argument <[fp]>.  If the
file cannot be opened as specified, the result is <<NULL>>.

PORTABILITY
<<freopen>> is a glibc extension.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek64>>, <<open64>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/lock.h>
#include "local64.h"

/*
 * Re-direct an existing, open (probably) file to some other file.
 */

#ifdef __LARGE64_FILES

FILE *
_DEFUN (_freopen64_r, (ptr, file, mode, fp),
	struct _reent *ptr _AND
	_CONST char *file _AND
	_CONST char *mode _AND
	register FILE *fp)
{
  register int f;
  int flags, oflags, e;

  __sfp_lock_acquire ();

  _flockfile(fp);

  CHECK_INIT (fp);

  if ((flags = __sflags (ptr, mode, &oflags)) == 0)
    {
      _funlockfile(fp);
      (void) _fclose_r (ptr, fp);
      __sfp_lock_release ();
      return NULL;
    }

  /*
   * Remember whether the stream was open to begin with, and
   * which file descriptor (if any) was associated with it.
   * If it was attached to a descriptor, defer closing it,
   * so that, e.g., freopen("/dev/stdin", "r", stdin) works.
   * This is unnecessary if it was not a Unix file.
   */

  if (fp->_flags == 0)
    fp->_flags = __SEOF;	/* hold on to it */
  else
    {
      if (fp->_flags & __SWR)
	(void) fflush (fp);
      /* if close is NULL, closing is a no-op, hence pointless */
      if (fp->_close != NULL)
	(void) (*fp->_close) (fp->_cookie);
    }

  /*
   * Now get a new descriptor to refer to the new file.
   */

  f = _open64_r (ptr, (char *) file, oflags, 0666);
  e = ptr->_errno;

  /*
   * Finish closing fp.  Even if the open succeeded above,
   * we cannot keep fp->_base: it may be the wrong size.
   * This loses the effect of any setbuffer calls,
   * but stdio has always done this before.
   */

  if (fp->_flags & __SMBF)
    _free_r (ptr, (char *) fp->_bf._base);
  fp->_w = 0;
  fp->_r = 0;
  fp->_p = NULL;
  fp->_bf._base = NULL;
  fp->_bf._size = 0;
  fp->_lbfsize = 0;
  if (HASUB (fp))
    FREEUB (fp);
  fp->_ub._size = 0;
  if (HASLB (fp))
    FREELB (fp);
  fp->_lb._size = 0;

  if (f < 0)
    {				/* did not get it after all */
      fp->_flags = 0;		/* set it free */
      ptr->_errno = e;		/* restore in case _close clobbered */
      _funlockfile(fp);
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }

  fp->_flags = flags;
  fp->_file = f;
  fp->_cookie = (_PTR) fp;
  fp->_read = __sread;
  fp->_write = __swrite64;
  fp->_seek = __sseek;
  fp->_seek64 = __sseek64;
  fp->_close = __sclose;

#ifdef __SCLE
  if (__stextmode(fp->_file))
    fp->_flags |= __SCLE;
#endif

  fp->_flags |= __SL64;

  _funlockfile(fp);
  __sfp_lock_release ();
  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN (freopen64, (file, mode, fp),
	_CONST char *file _AND
	_CONST char *mode _AND
	register FILE *fp)
{
  return _freopen64_r (_REENT, file, mode, fp);
}

#endif /* !_REENT_ONLY */

#endif /* __LARGE64_FILES */
