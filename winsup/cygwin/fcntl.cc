/* fcntl.cc: fcntl syscall

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2008,
   2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"

extern "C" int
fcntl64 (int fd, int cmd, ...)
{
  int res = -1;
  void *arg = NULL;
  va_list args;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  cygheap_fdget cfd (fd, true);
  if (cfd < 0)
    goto done;

  va_start (args, cmd);
  arg = va_arg (args, void *);
  va_end (args);

  switch (cmd)
    {
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
      if ((int) arg >= 0 && (int) arg < OPEN_MAX_MAX)
	res = dup3 (fd, cygheap_fdnew (((int) arg) - 1),
		    cmd == F_DUPFD_CLOEXEC ? O_CLOEXEC : 0);
      else
	{
	  set_errno (EINVAL);
	  res = -1;
	}
      break;
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      {
	struct __flock64 *fl = (struct __flock64 *) arg;
	fl->l_type &= F_RDLCK | F_WRLCK | F_UNLCK;
	res = cfd->lock (cmd, fl);
      }
      break;
    default:
      res = cfd->fcntl (cmd, arg);
      break;
    }
done:
  syscall_printf ("%d = fcntl (%d, %d, %p)", res, fd, cmd, arg);
  return res;
}

extern "C" int
_fcntl (int fd, int cmd, ...)
{
  void *arg = NULL;
  va_list args;
  struct __flock32 *src = NULL;
  struct __flock64 dst;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

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
  int res = fcntl64 (fd, cmd, arg);
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
