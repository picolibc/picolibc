/* sys/msg.h

   Copyright 2002 Red Hat Inc.
   Written by Conrad Scott <conrad.scott@dsl.pipex.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_MSG_H
#define _CYGWIN_MSG_H

#include <cygwin/ipc.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Message operation flags:
 */
#define MSG_NOERROR 0x01	/* No error if big message. */

#ifdef _KERNEL
/* Command definitions for the semctl () function:
 */
#define MSG_STAT   0x2000	/* For ipcs(8) */
#define MSG_INFO   0x2001	/* For ipcs(8) */
#endif /* _KERNEL */

/* Used for the number of messages in the message queue.
 */
typedef unsigned long msgqnum_t;

/* Used for the number of bytes allowed in a message queue.
 */
typedef unsigned long msglen_t;

struct msqid_ds
{
  struct ipc_perm msg_perm;	/* Operation permission structure. */
  msglen_t        msg_cbytes;	/* Number of bytes currently on queue. */
  msgqnum_t       msg_qnum;	/* Number of messages currently on queue. */
  msglen_t        msg_qbytes;	/* Maximum number of bytes allowed on queue. */
  pid_t           msg_lspid;	/* Process ID of last msgsnd (). */
  pid_t           msg_lrpid;	/* Process ID of last msgrcv (). */
  timestruc_t     msg_stim;	/* Time of last msgsnd (). */
  timestruc_t     msg_rtim;	/* Time of last msgrcv (). */
  timestruc_t     msg_ctim;	/* Time of last change. */
#ifdef _KERNEL
  struct msg     *msg_first;
  struct msg     *msg_last;
#else
  long            msg_spare4[2];
#endif /* _KERNEL */
};

#define msg_stime msg_stim.tv_sec
#define msg_rtime msg_rtim.tv_sec
#define msg_ctime msg_ctim.tv_sec

#ifdef _KERNEL
/* Buffer type for msgctl (IPC_INFO, ...) as used by ipcs(8).
 */
struct msginfo
{
  long msgmax;		/* Maximum number of bytes per
			   message. */
  long msgmnb;		/* Maximum number of bytes on any one
			   message queue. */
  long msgmni;		/* Maximum number of message queues,
			   system wide. */
  long msgtql;		/* Maximum number of messages, system
			   wide. */
  long msgssz;		/* Size of a message segment, must be
			   small power of 2 greater than 4. */
  long msgseg;		/* Number of message segments */
  long msg_spare[2];
};

/* Buffer type for msgctl (MSG_INFO, ...) as used by ipcs(8).
 */
struct msg_info
{
  long msg_ids;		/* Number of allocated queues. */
  long msg_num;		/* Number of messages, system wide. */
  long msg_tot;		/* Size in bytes of messages, system wide. */
};
#endif /* _KERNEL */

int     msgctl (int msqid, int cmd, struct msqid_ds *buf);
int     msgget (key_t key, int msgflg);
ssize_t msgrcv (int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
int     msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg);

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_MSG_H */
