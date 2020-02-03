/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* Reentrant version of rename system call.  */

#include <reent.h>
#include <stdio.h>
#include <unistd.h>
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
	<<_rename_r>>---Reentrant version of rename
	
INDEX
	_rename_r

SYNOPSIS
	#include <reent.h>
	int _rename_r(struct _reent *<[ptr]>,
		const char *<[old]>, const char *<[new]>);

DESCRIPTION
	This is a reentrant version of <<rename>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/

int
_rename_r (struct _reent *ptr,
     const char *old,
     const char *new)
{
  int ret = 0;

#ifdef HAVE_RENAME
  errno = 0;
  if ((ret = _rename (old, new)) == -1 && errno != 0)
    __errno_r(ptr) = errno;
#else
  if (_link_r (ptr, old, new) == -1)
    return -1;

  if (_unlink_r (ptr, old) == -1)
    {
      /* ??? Should we unlink new? (rhetorical question) */
      return -1;
    }
#endif
  return ret;
}

#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
