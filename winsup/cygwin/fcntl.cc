/* fcntl.cc: fcntl syscall

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygerrno.h"
#include "cygheap.h"
#include "thread.h"

extern "C"
int
_fcntl (int fd, int cmd,...)
{
  void *arg = NULL;
  va_list args;
  int res;

  cygheap_fdget cfd (fd, true);
  if (cfd < 0)
    {
      res = -1;
      goto done;
    }

  va_start (args, cmd);
  arg = va_arg (args, void *);
  if (cmd != F_DUPFD)
    res = cfd->fcntl(cmd, arg);
  else
    res = dup2 (fd, cygheap_fdnew (((int) arg) - 1));
  va_end (args);

done:
  syscall_printf ("%d = fcntl (%d, %d, %p)", res, fd, cmd, arg);
  return res;
}
