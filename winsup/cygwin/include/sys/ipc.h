/* sys/ipc.h

   Copyright 2001 Red Hat Inc. 
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _SYS_IPC_H
#define _SYS_IPC_H

/* sys/types must be included before sys/ipc.h. We aren't meant to automatically 
 * include it however 
 */

struct ipc_perm {
  uid_t  uid;
  gid_t  gid;
  uid_t  cuid;
  gid_t  cgid;
  mode_t mode;
}; 

/* the mode flags used with the _get functions use the low order 9 bits for a mode 
 * request
 */
#define IPC_CREAT  0x0200
#define IPC_EXCL   0x0400
#define IPC_NOWAIT 0x0800

/* this is a value that will _never_ be a valid key from ftok */
#define IPC_PRIVATE -2

#define IPC_RMID 0x0003
#define IPC_SET  0x0002
#define IPC_STAT 0x0001

key_t ftok(const char *, int);

#endif /* _SYS_IPC_H */

#ifdef __cplusplus
}
#endif
