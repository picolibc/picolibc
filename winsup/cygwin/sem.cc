/* sem.cc: Single unix specification IPC interface for Cygwin.

   Copyright 2002 Red Hat, Inc.

   Written by Conrad Scott <conrad.scott@dsl.pipex.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

#include <sys/types.h>
#include <cygwin/sem.h>

#include <errno.h>

#include "cygerrno.h"

extern "C" int
semctl (int semid, int semnum, int cmd, ...)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
semget (key_t key, int nsems, int semflg)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
semop (int semid, struct sembuf *sops, size_t nsops)
{
  set_errno (ENOSYS);
  return -1;
}
