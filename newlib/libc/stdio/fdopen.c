/*
FUNCTION
<<fdopen>>---turn open file into a stream

INDEX
	fdopen
INDEX
	_fdopen_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *fdopen(int <[fd]>, const char *<[mode]>);
	FILE *_fdopen_r(void *<[reent]>,
                     int <[fd]>, const char *<[mode]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *fdopen(<[fd]>, <[mode]>)
	int <[fd]>;
	char *<[mode]>;

	FILE *_fdopen_r(<[reent]>, <[fd]>, <[mode]>)
	char *<[reent]>;
        int <[fd]>;
	char *<[mode]>);

DESCRIPTION
<<fdopen>> produces a file descriptor of type <<FILE *>>, from a
descriptor for an already-open file (returned, for example, by the
system subroutine <<open>> rather than by <<fopen>>).
The <[mode]> argument has the same meanings as in <<fopen>>.

RETURNS
File pointer or <<NULL>>, as for <<fopen>>.

PORTABILITY
<<fdopen>> is ANSI.
*/

#include <sys/types.h>
#include <sys/fcntl.h>

#include <stdio.h>
#include <errno.h>
#include "local.h"
#include <_syslist.h>

extern int __sflags ();

FILE *
_DEFUN (_fdopen_r, (ptr, fd, mode),
	struct _reent *ptr _AND
	int fd _AND
	_CONST char *mode)
{
  register FILE *fp;
  int flags, oflags;
#ifdef F_GETFL
  int fdflags, fdmode;
#endif

  if ((flags = __sflags (ptr, mode, &oflags)) == 0)
    return 0;

  /* make sure the mode the user wants is a subset of the actual mode */
#ifdef F_GETFL
  if ((fdflags = _fcntl (fd, F_GETFL, 0)) < 0)
    return 0;
  fdmode = fdflags & O_ACCMODE;
  if (fdmode != O_RDWR && (fdmode != (oflags & O_ACCMODE)))
    {
      ptr->_errno = EBADF;
      return 0;
    }
#endif

  if ((fp = __sfp (ptr)) == 0)
    return 0;
  fp->_flags = flags;
  /*
   * If opened for appending, but underlying descriptor
   * does not have O_APPEND bit set, assert __SAPP so that
   * __swrite() will lseek to end before each write.
   */
  if ((oflags & O_APPEND)
#ifdef F_GETFL
       && !(fdflags & O_APPEND)
#endif
      )
    fp->_flags |= __SAPP;
  fp->_file = fd;
  fp->_cookie = (_PTR) fp;

#undef _read
#undef _write
#undef _seek
#undef _close

  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

#ifdef __SCLE
  if (setmode(fp->_file, O_BINARY) == O_TEXT)
    fp->_flags |= __SCLE;
#endif

  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN (fdopen, (fd, mode),
	int fd _AND
	_CONST char *mode)
{
  return _fdopen_r (_REENT, fd, mode);
}

#endif
