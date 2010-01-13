/* fcntl.h

   Copyright 1996, 1998, 2001, 2005, 2006, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/fcntl.h>
#define O_NDELAY	_FNDELAY

/* sys/fcntl defines values up to 0x40000 (O_NOINHERIT). */
#define _FDIRECT	0x80000
#define _FNOFOLLOW	0x100000
#define _FDIRECTORY	0x200000
#define _FEXECSRCH	0x400000

/* POSIX-1.2008 requires this flag and allows to set it to 0 if its
   functionality is not required. */
#define O_TTY_INIT	0

#define O_DIRECT	_FDIRECT
#define O_NOFOLLOW	_FNOFOLLOW
#define O_DSYNC		_FSYNC
#define O_RSYNC		_FSYNC
#define O_DIRECTORY	_FDIRECTORY
#define O_EXEC		_FEXECSRCH
#define O_SEARCH	_FEXECSRCH

#define POSIX_FADV_NORMAL	0
#define POSIX_FADV_SEQUENTIAL	1
#define POSIX_FADV_RANDOM	2
#define POSIX_FADV_WILLNEED	3
#define POSIX_FADV_DONTNEED	4
#define POSIX_FADV_NOREUSE	5

#ifdef __cplusplus
extern "C" {
#endif
extern int posix_fadvise _PARAMS ((int, off_t, off_t, int));
extern int posix_fallocate _PARAMS ((int, off_t, off_t));
#ifdef __cplusplus
}
#endif

#endif /* _FCNTL_H */
