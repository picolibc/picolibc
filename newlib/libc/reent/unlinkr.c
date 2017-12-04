/* Reentrant versions of file system calls.  These implementations
   just call the usual system calls.  */

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
	<<_unlink_r>>---Reentrant version of unlink
	
INDEX
	_unlink_r

SYNOPSIS
	#include <reent.h>
	int _unlink_r(struct _reent *<[ptr]>, const char *<[file]>);

DESCRIPTION
	This is a reentrant version of <<unlink>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_unlink_r (struct _reent *ptr,
     const char *file)
{
  int ret;

  errno = 0;
  if ((ret = _unlink (file)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
