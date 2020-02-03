/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* Reentrant versions of execution system calls.  These
   implementations just call the usual system calls.  */

#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <_syslist.h>

/* Some targets provides their own versions of these functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

/* If NO_EXEC is defined, we don't need these functions.  */

#if defined (REENTRANT_SYSCALLS_PROVIDED) || defined (NO_EXEC)

int _dummy_exec_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_execve_r>>---Reentrant version of execve	
INDEX
	_execve_r

SYNOPSIS
	#include <reent.h>
	int _execve_r(struct _reent *<[ptr]>, const char *<[name]>,
                      char *const <[argv]>[], char *const <[env]>[]);

DESCRIPTION
	This is a reentrant version of <<execve>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_execve_r (struct _reent *ptr,
     const char *name,
     char *const argv[],
     char *const env[])
{
  int ret;

  errno = 0;
  if ((ret = _execve (name, argv, env)) == -1 && errno != 0)
    __errno_r(ptr) = errno;
  return ret;
}


/*
NEWPAGE
FUNCTION
	<<_fork_r>>---Reentrant version of fork
	
INDEX
	_fork_r

SYNOPSIS
	#include <reent.h>
	int _fork_r(struct _reent *<[ptr]>);

DESCRIPTION
	This is a reentrant version of <<fork>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

#ifndef NO_FORK

int
_fork_r (struct _reent *ptr)
{
  int ret;

  errno = 0;
  if ((ret = _fork ()) == -1 && errno != 0)
    __errno_r(ptr) = errno;
  return ret;
}

#endif

/*
NEWPAGE
FUNCTION
	<<_wait_r>>---Reentrant version of wait
	
INDEX
	_wait_r

SYNOPSIS
	#include <reent.h>
	int _wait_r(struct _reent *<[ptr]>, int *<[status]>);

DESCRIPTION
	This is a reentrant version of <<wait>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_wait_r (struct _reent *ptr,
     int *status)
{
  int ret;

  errno = 0;
  if ((ret = _wait (status)) == -1 && errno != 0)
    __errno_r(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
