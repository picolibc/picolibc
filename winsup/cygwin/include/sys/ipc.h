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

/*
 * <sys/types.h> must be included before <sys/ipc.h>.  We aren't meant
 * to automatically include it however.
 */

struct ipc_perm {
  uid_t  uid;
  gid_t  gid;
  uid_t  cuid;
  gid_t  cgid;
  mode_t mode;
}; 

/*
 * The mode flags used with the _get functions use the low order 9
 * bits for a mode request.
 */
#define IPC_CREAT  0x0200
#define IPC_EXCL   0x0400
#define IPC_NOWAIT 0x0800

/* This is a value that will _never_ be a valid key from ftok(3). */
#define IPC_PRIVATE ((key_t) -2)

/*
 * Values for the cmd argument to shmctl(2).
 * Commands 1000-1fff are reserved for IPC_xxx.
 */
#define IPC_RMID 0x1000
#define IPC_SET  0x1001
#define IPC_STAT 0x1002
#define IPC_INFO 0x1003		/* For ipcs(8). */

key_t ftok(const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_IPC_H */
