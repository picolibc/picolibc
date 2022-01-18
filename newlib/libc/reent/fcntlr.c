/* Reentrant versions of fcntl system call.  This implementation just
   calls the fcntl system call.  */

#include <reent.h>
#include <fcntl.h>
#include <sys/stat.h>
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
	<<_fcntl_r>>---Reentrant version of fcntl
	
INDEX
	_fcntl_r

SYNOPSIS
	#include <reent.h>
	int _fcntl_r(struct _reent *<[ptr]>,
		     int <[fd]>, int <[cmd]>, <[arg]>);

DESCRIPTION
	This is a reentrant version of <<fcntl>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_fcntl_r (struct _reent *ptr,
     int fd,
     int cmd,
     int arg)
{
  int ret;

  errno = 0;
  if ((ret = _fcntl (fd, cmd, arg)) == -1 && errno != 0)
    _REENT_ERRNO(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
