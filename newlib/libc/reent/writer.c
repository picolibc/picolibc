/* Reentrant versions of write system call. */

#include <reent.h>
#include <unistd.h>
#include <_syslist.h>

/* Some targets provides their own versions of this functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifndef REENTRANT_SYSCALLS_PROVIDED

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_write_r>>---Reentrant version of write
	
INDEX
	_write_r

SYNOPSIS
	#include <reent.h>
	_ssize_t _write_r(struct _reent *<[ptr]>,
		          int <[fd]>, const void *<[buf]>, size_t <[cnt]>);

DESCRIPTION
	This is a reentrant version of <<write>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

_ssize_t
_write_r (struct _reent *ptr,
     int fd,
     const void *buf,
     size_t cnt)
{
  _ssize_t ret;

  errno = 0;
  if ((ret = (_ssize_t)_write (fd, buf, cnt)) == -1 && errno != 0)
    _REENT_ERRNO(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
