/* cygserver_ipc.h

   Copyright 2002 Red Hat, Inc.

   Originally written by Conrad Scott <conrad.scott@dsl.pipex.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_IPC_H__
#define __CYGSERVER_IPC_H__

#include <limits.h>		/* For OPEN_MAX. */

/*
 * The sysv ipc id's (msgid, semid, shmid) are small integers arranged
 * such that they no subsystem will generate the same id as some other
 * subsystem, and nor do these ids overlap file descriptors (the other
 * common small integer ids).  Since Cygwin can allocate more than
 * OPEN_MAX file descriptor, it can't be guarenteed not to overlap,
 * but it should help catch some errors.
 *
 * msgid's: OPEN_MAX,     OPEN_MAX + 3, OPEN_MAX + 6, . . .
 * semid's: OPEN_MAX + 1, OPEN_MAX + 4, OPEN_MAX + 7, . . . 
 * shmid's: OPEN_MAX + 2, OPEN_MAX + 5, OPEN_MAX + 8, . . . 
 *
 * Internal ipc id's, which are 0, 1, ... within each subsystem, are
 * used solely by the ipcs(8) interface.
 */

enum ipc_subsys_t {
  IPC_MSGOP = 0,
  IPC_SEMOP = 1,
  IPC_SHMOP = 2,
  IPC_SUBSYS_COUNT
};

inline int
ipc_int2ext (const int id, const ipc_subsys_t subsys)
{
  return OPEN_MAX + (id * IPC_SUBSYS_COUNT) + subsys;
}

inline int
ipc_ext2int (const int id)
{
  return (id - OPEN_MAX) / IPC_SUBSYS_COUNT;
}

inline int
ipc_ext2int_subsys (const int id)
{
  return (id - OPEN_MAX) % IPC_SUBSYS_COUNT;
}

inline int
ipc_inc_id (const int id, const ipc_subsys_t subsys)
{
  return id + IPC_SUBSYS_COUNT;
}

inline int
ipc_dec_id (const int id, const ipc_subsys_t subsys)
{
  return id - IPC_SUBSYS_COUNT;
}

#endif /* __CYGSERVER_IPC_H__ */
