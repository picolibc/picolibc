/* cygwin/sysproto.h

   Copyright 2003, 2005, 2012, 2013 Red Hat, Inc.

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

struct msgctl_args {
  int     msqid;
  int     cmd;
  struct  msqid_ds *buf;
};

struct msgget_args {
  key_t   key;
  int     msgflg;
};

struct msgrcv_args {
  int     msqid;
  void    *msgp;
  size_t  msgsz;
  long    msgtyp;
  int     msgflg;
};

struct msgsnd_args {
  int     msqid;
  const void *msgp;
  size_t  msgsz;
  int     msgflg;
};

struct semctl_args {
  int     semid;
  int     semnum;
  int     cmd;
  union   semun *arg;
};

struct semget_args {
  key_t   key;
  int     nsems;
  int     semflg;
};

struct semop_args {
  int     semid;
  struct  sembuf *sops;
  size_t  nsops;
};

struct shmat_args {
  int     shmid;
  const void *shmaddr;
  int     shmflg;
};

struct shmctl_args {
  int     shmid;
  int     cmd;
  struct shmid_ds *buf;
};

struct shmdt_args {
  const void *shmaddr;
};

struct shmget_args {
  key_t   key;
  size_t  size;
  int     shmflg;
};

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_SYSPROTO_H */
