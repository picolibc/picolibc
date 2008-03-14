/* fcntl.cc: fcntl syscall

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdarg.h>
#include <unistd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "thread.h"
#include "cygtls.h"

int
fcntl_worker (int fd, int cmd, void *arg)
{
  int res;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  cygheap_fdget cfd (fd, true);
  if (cfd < 0)
    {
      res = -1;
      goto done;
    }
  if (cmd != F_DUPFD)
    res = cfd->fcntl (cmd, arg);
  else
    res = dup2 (fd, cygheap_fdnew (((int) arg) - 1));
done:
  syscall_printf ("%d = fcntl (%d, %d, %p)", res, fd, cmd, arg);
  return res;
}

extern "C" int
fcntl64 (int fd, int cmd, ...)
{
  void *arg = NULL;
  va_list args;

  va_start (args, cmd);
  arg = va_arg (args, void *);
  va_end (args);
  return fcntl_worker (fd, cmd, arg);
}

extern "C" int
_fcntl (int fd, int cmd, ...)
{
  void *arg = NULL;
  va_list args;
  struct __flock32 *src = NULL;
  struct __flock64 dst;

  va_start (args, cmd);
  arg = va_arg (args, void *);
  va_end (args);
  if (cmd == F_GETLK || cmd == F_SETLK || cmd == F_SETLKW)
    {
      src = (struct __flock32 *) arg;
      dst.l_type = src->l_type;
      dst.l_whence = src->l_whence;
      dst.l_start = src->l_start;
      dst.l_len = src->l_len;
      dst.l_pid = src->l_pid;
      arg = &dst;
    }
  int res = fcntl_worker (fd, cmd, arg);
  if (cmd == F_GETLK)
    {
      src->l_type = dst.l_type;
      src->l_whence = dst.l_whence;
      src->l_start = dst.l_start;
      src->l_len = dst.l_len;
      src->l_pid = (short) dst.l_pid;
    }
  return res;
}
