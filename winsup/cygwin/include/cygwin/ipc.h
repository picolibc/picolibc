/* sys/ipc.h

   Copyright 2001, 2002 Red Hat Inc.
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#ifdef __cplusplus
extern "C"
{
#endif

struct ipc_perm
{
  uid_t  uid;			/* Owner's user ID. */
  gid_t  gid;			/* Owner's group ID. */
  uid_t  cuid;			/* Creator's user ID. */
  gid_t  cgid;			/* Creator's group ID. */
  mode_t mode;			/* Read/write permission. */
  key_t  key;
};

/* Mode bits:
 */
#define IPC_CREAT  0x0200	/* Create entry if key does not exist. */
#define IPC_EXCL   0x0400	/* Fail if key exists. */
#define IPC_NOWAIT 0x0800	/* Error if request must wait. */

/* Keys:
 */
#define IPC_PRIVATE ((key_t) 0)	/* Private key. */

/* Control commands:
 */
#define IPC_RMID 0x1000		/* Remove identifier. */
#define IPC_SET  0x1001		/* Set options. */
#define IPC_STAT 0x1002		/* Get options. */
#define IPC_INFO 0x1003		/* For ipcs(8). */

key_t ftok (const char *path, int id);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_IPC_H */
