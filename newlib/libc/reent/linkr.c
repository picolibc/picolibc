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

#ifdef REENTRANT_SYSCALLS_PROVIDED

int _dummy_link_syscalls = 1;

#else

/* We use the errno variable used by the system dependent layer.  */
#undef errno
extern int errno;

/*
FUNCTION
	<<_link_r>>---Reentrant version of link
	
INDEX
	_link_r

SYNOPSIS
	#include <reent.h>
	int _link_r(struct _reent *<[ptr]>,
		    const char *<[old]>, const char *<[new]>);

DESCRIPTION
	This is a reentrant version of <<link>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_link_r (struct _reent *ptr,
     const char *old,
     const char *new)
{
  int ret;

  errno = 0;
  if ((ret = _link (old, new)) == -1 && errno != 0)
    __errno_r(ptr) = errno;
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
