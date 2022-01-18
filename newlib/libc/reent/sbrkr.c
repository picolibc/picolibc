/* Reentrant version of sbrk system call. */

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

#if defined (REENTRANT_SYSCALLS_PROVIDED)

int _dummy_sbrk_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_sbrk_r>>---Reentrant version of sbrk
	
INDEX
	_sbrk_r

SYNOPSIS
	#include <reent.h>
	void *_sbrk_r(struct _reent *<[ptr]>, ptrdiff_t <[incr]>);

DESCRIPTION
	This is a reentrant version of <<sbrk>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

void *
_sbrk_r (struct _reent *ptr,
     ptrdiff_t incr)
{
  char *ret;
  void *_sbrk(ptrdiff_t);

  errno = 0;
  if ((ret = (char *)(_sbrk (incr))) == (void *) -1 && errno != 0)
    _REENT_ERRNO(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
