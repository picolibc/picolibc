/* cygserver_ipc.h

   Copyright 2002 Red Hat, Inc.

   Originally written by Conrad Scott <conrad.scott@dsl.pipex.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_IPC_H__
#define __CYGSERVER_IPC_H__

#include <assert.h>
#include <limits.h>		/* For OPEN_MAX. */

/*
 * The sysv ipc id's (msgid, semid, shmid) are integers arranged such
 * that they no subsystem will generate the same id as some other
 * subsystem; nor do these ids overlap file descriptors (the other
 * common integer ids).  Since Cygwin can allocate more than OPEN_MAX
 * file descriptors, it can't be guaranteed not to overlap, but it
 * should help catch some errors.
 *
 * msgid's: OPEN_MAX,     OPEN_MAX + 3, OPEN_MAX + 6, . . .
 * semid's: OPEN_MAX + 1, OPEN_MAX + 4, OPEN_MAX + 7, . . .
 * shmid's: OPEN_MAX + 2, OPEN_MAX + 5, OPEN_MAX + 8, . . .
 *
 * To further ensure that ids are unique, if ipc objects are created
 * and destroyed and then re-created, they are given new ids by
 * munging the basic id (as above) with a sequence number.
 *
 * Internal ipc id's, which are 0, 1, ... within each subsystem (and
 * not munged with a sequence number), are used solely by the ipcs(8)
 * interface.
 */

enum ipc_subsys_t
  {
    IPC_MSGOP = 0,
    IPC_SEMOP = 1,
    IPC_SHMOP = 2,
    IPC_SUBSYS_COUNT
  };

/*
 * IPCMNI - The absolute maximum number of simultaneous ipc ids for
 * any one subsystem.
 */

enum
  {
    IPCMNI = 0x10000		// Must be a power of two.
  };

inline int
ipc_int2ext (const int intid, const ipc_subsys_t subsys, long & sequence)
{
  assert (0 <= intid && intid < IPCMNI);

  const long tmp = InterlockedIncrement (&sequence);

  return (((tmp & 0x7fff) << 16)
	  | (OPEN_MAX + (intid * IPC_SUBSYS_COUNT) + subsys));
}

inline int
ipc_ext2int_subsys (const int extid)
{
  return ((extid & (IPCMNI - 1)) - OPEN_MAX) % IPC_SUBSYS_COUNT;
}

inline int
ipc_ext2int (const int extid, const ipc_subsys_t subsys)
{
  if (ipc_ext2int_subsys (extid) != subsys)
    return -1;
  else
    return ((extid & (IPCMNI - 1)) - OPEN_MAX) / IPC_SUBSYS_COUNT;
}

#endif /* __CYGSERVER_IPC_H__ */
