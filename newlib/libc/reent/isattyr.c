/* Reentrant versions of isatty system call.  */

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

#ifdef REENTRANT_SYSCALLS_PROVIDED

int _dummy_isatty_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_isatty_r>>---Reentrant version of isatty
	
INDEX
	_isatty_r

SYNOPSIS
	#include <reent.h>
	int _isatty_r(struct _reent *<[ptr]>,
		     int <[fd]>);

DESCRIPTION
	This is a reentrant version of <<isatty>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_isatty_r (ptr, fd)
     struct _reent *ptr;
     int fd;
{
  int ret;

  errno = 0;
  if ((ret = _isatty (fd)) == -1 && errno != 0)
    _REENT_ERRNO(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
