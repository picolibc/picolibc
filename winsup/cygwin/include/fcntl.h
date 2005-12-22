/* fcntl.h

   Copyright 1996, 1998, 2001, 2005 Red Hat, Inc.

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

#define O_DIRECT	_FDIRECT
#define O_NOFOLLOW	_FNOFOLLOW
#define O_DSYNC		_FSYNC
#define O_RSYNC		_FSYNC

#endif /* _FCNTL_H */
