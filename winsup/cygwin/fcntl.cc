/* fcntl.cc: fcntl syscall

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

extern "C"
int
_fcntl (int fd, int cmd,...)
{
  va_list args;
  int arg = 0;
  int res;
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK, "_fcntl");

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      res = -1;
      goto done;
    }

  switch (cmd)
    {
    case F_DUPFD:
      va_start (args, cmd);
      arg = va_arg (args,int);
      va_end (args);
      res = dup2 (fd, fdtab.find_unused_handle (arg));
      goto done;

    case F_GETFD:
      res = fdtab[fd]->get_close_on_exec () ? FD_CLOEXEC : 0;
      goto done;

    case F_SETFD:
      va_start (args, cmd);
      arg = va_arg (args, int);
      va_end (args);
      fdtab[fd]->set_close_on_exec (arg);
      res = 0;
      goto done;

    case F_GETFL:
      {
	res = fdtab[fd]->get_flags ();
	goto done;
      }
    case F_SETFL:
      {
	int temp = 0;

	va_start (args, cmd);
	arg = va_arg (args, int);
	va_end (args);

	if (arg & O_RDONLY)
	  temp |= GENERIC_READ;
	if (arg & O_WRONLY)
	  temp |= GENERIC_WRITE;

	syscall_printf ("fcntl (%d, F_SETFL, %d)", arg);

	fdtab[fd]->set_access (temp);
	fdtab[fd]->set_flags (arg);

	res = 0;
	goto done;
      }

    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      {
	struct flock *fl;
	va_start (args, cmd);
	fl = va_arg (args,struct flock *);
	va_end (args);
	res = fdtab[fd]->lock (cmd, fl);
	goto done;
      }
    default:
      set_errno (EINVAL);
      res = -1;
      goto done;
    }

  set_errno (ENOSYS);
  res = -1;

 done:
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"_fcntl");

  syscall_printf ("%d = fcntl (%d, %d, %d)", res, fd, cmd, arg);
  return res;
}
