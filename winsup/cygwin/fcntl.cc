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
#include "fhandler.h"
#include "dtable.h"
#include "cygerrno.h"
#include "thread.h"

extern "C"
int
_fcntl (int fd, int cmd,...)
{
  void *arg = NULL;
  va_list args;
  int res;

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      res = -1;
      goto done;
    }

  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK, "_fcntl");
  va_start (args, cmd);
  arg = va_arg (args, void *);
  if (cmd == F_DUPFD)
    res = dup2 (fd, fdtab.find_unused_handle ((int) arg));
  else
    res = fdtab[fd]->fcntl(cmd, arg);
  va_end (args);
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"_fcntl");

done:
  syscall_printf ("%d = fcntl (%d, %d, %p)", res, fd, cmd, arg);
  return res;
}
