/* msg.cc: Single unix specification IPC interface for Cygwin.

   Copyright 2002 Red Hat, Inc.

   Written by Conrad Scott <conrad.scott@dsl.pipex.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

#include <sys/types.h>
#include <cygwin/msg.h>

#include <errno.h>

#include "cygerrno.h"

extern "C" int
msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
msgget (key_t key, int msgflg)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" ssize_t
msgrcv (int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
  set_errno (ENOSYS);
  return -1;
}
