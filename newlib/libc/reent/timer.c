/* Reentrant versions of times and gettimeofday system calls for the
   clock and time ANSI C routines.
   This implementation just calls the times/gettimeofday system calls.
   Gettimeofday may not be available on all targets.  It's presence
   here is dubious.  Consider it for internal use only.  */

#include <reent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <_syslist.h>

/* Some targets provides their own versions of these functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifdef REENTRANT_SYSCALLS_PROVIDED

int _dummy_time_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_times_r>>---Reentrant version of times
	
INDEX
	_times_r

ANSI_SYNOPSIS
	#include <reent.h>
	#include <sys/times.h>
	clock_t _times_r(struct _reent *<[ptr]>, struct tms *<[ptms]>);

TRAD_SYNOPSIS
	#include <reent.h>
	#include <sys/times.h>
	clock_t _times_r(<[ptr]>, <[ptms]>)
	struct _reent *<[ptr]>;
	struct tms *<[ptms]>;

DESCRIPTION
	This is a reentrant version of <<times>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

clock_t
_DEFUN (_times_r, (ptr, ptms),
     struct _reent *ptr _AND
     struct tms *ptms)
{
  clock_t ret;

  ret = _times (ptms);
  return ret;
}

/*
FUNCTION
	<<_gettimeofday_r>>---Reentrant version of gettimeofday
	
INDEX
	_gettimeofday_r

ANSI_SYNOPSIS
	#include <reent.h>
	#include <time.h>
	int _gettimeofday_r(struct _reent *<[ptr]>,
		struct timeval *<[ptimeval]>,
		struct timezone *<[ptimezone]>);

TRAD_SYNOPSIS
	#include <reent.h>
	#include <time.h>
	int _gettimeofday_r(<[ptr]>, <[ptimeval]>, <[ptimezone]>)
	struct _reent *<[ptr]>;
	struct timeval *<[ptimeval]>;
	struct timezone *<[ptimezone]>;

DESCRIPTION
	This is a reentrant version of <<gettimeofday>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.

	This function is only available for a few targets.
	Check libc.a to see if its available on yours.
*/

int
_DEFUN (_gettimeofday_r, (ptr, ptimeval, ptimezone),
     struct _reent *ptr _AND
     struct timeval *ptimeval _AND
     struct timezone *ptimezone)
{
  int ret;

  errno = 0;
  if ((ret = _gettimeofday (ptimeval, ptimezone)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
