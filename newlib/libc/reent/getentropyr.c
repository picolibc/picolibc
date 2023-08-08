/* Reentrant version of getentropy system call. */

#include <reent.h>
#include <unistd.h>
#include <_syslist.h>

/* Some targets provides their own versions of these functions.  Those
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
	<<_getentropy_r>>---Reentrant version of getentropy

INDEX
	_getentropy_r

SYNOPSIS
	#include <reent.h>
	int _getentropy_r(struct _reent *<[ptr]>,
		          void *<[buf]>, size_t <[buflen]>);

DESCRIPTION
	This is a reentrant version of <<getentropy>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_getentropy_r (struct _reent *ptr,
     void *buf,
     size_t buflen)
{
  int ret;

  errno = 0;
  if ((ret = _getentropy (buf, buflen)) == -1 && errno != 0)
    _REENT_ERRNO(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
