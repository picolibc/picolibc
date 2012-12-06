/* cygwin/sysproto.h

   Copyright 2003, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* cygwin/sysproto.h header file for Cygwin.  */

#ifndef _CYGWIN_SYSPROTO_H
#define _CYGWIN_SYSPROTO_H
#define _SYS_SYSPROTO_H_ /* Keep it, used by BSD files */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#pragma pack (push, 4)
struct msgctl_args {
  int     msqid;
  int     cmd;
  _TYPE64 (struct msqid_ds *, buf);
};

struct msgget_args {
  key_t   key;
  int     msgflg;
};

struct msgrcv_args {
  int     msqid;
  int     msgflg;
  _TYPE64 (void *, msgp);
  _TYPE64 (size_t, msgsz);
  _TYPE64 (long, msgtyp);
};

struct msgsnd_args {
  int     msqid;
  int     msgflg;
  _TYPE64 (const void *, msgp);
  _TYPE64 (size_t, msgsz);
};

struct semctl_args {
  int     semid;
  int     semnum;
  int     cmd;
  _TYPE64 (union semun *, arg);
};

struct semget_args {
  key_t   key;
  int     nsems;
  int     semflg;
};

struct semop_args {
  int     semid;
  _TYPE64 (struct sembuf *, sops);
  _TYPE64 (size_t, nsops);
};

struct shmat_args {
  int     shmid;
  _TYPE64 (const void *, shmaddr);
  int     shmflg;
};

struct shmctl_args {
  int     shmid;
  int     cmd;
  _TYPE64 (struct shmid_ds *, buf);
};

struct shmdt_args {
  _TYPE64 (const void *, shmaddr);
};

struct shmget_args {
  key_t   key;
  _TYPE64 (size_t, size);
  int     shmflg;
};
#pragma pack (pop)

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_SYSPROTO_H */
