/* fcntl.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/fcntl.h>
#define O_NDELAY	_FNDELAY

/* F_LCK_MANDATORY: Request mandatory locks for this file descriptor.

   Cygwin extension to fcntl file locking mechanism.  By default, fcntl file
   locks are advisory locks.  This works nicely as long as only Cygwin
   processes interact.  If you have the requirement to interact with native
   Windows applications which use Windows mandatory file locking, your have
   to use mandatory locking as well.  The command
   
   fcntl (fd, F_LCK_MANDATORY, 1)

   switches subsequent F_GETLK, F_SETLK, F_SETLKW calls to mandatory locking
   for this file descriptor and subsequently duplicated ones WITHIN THE SAME
   PROCESS.  Note that mandatory locks are NOT inherited by child processes,
   nor do they survive an execve call.  This fully corresponds to Windows
   mandatory locking semantics. */
#define F_LCK_MANDATORY	0x99

/* POSIX-1.2008 requires this flag and allows to set it to 0 if its
   functionality is not required. */
#define O_TTY_INIT	0

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
