/* sys/sem.h

   Copyright 2002 Red Hat Inc.
   Written by Conrad Scott <conrad.scott@dsl.pipex.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_SEM_H
#define _SYS_SEM_H

#include <cygwin/ipc.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Semaphore operation flags:
 */
#define SEM_UNDO		/* Set up adjust on exit entry. */

/* Command definitions for the semctl () function:
 */
#define GETNCNT    0x3000	/* Get semncnt. */
#define GETPID     0x3001	/* Get sempid. */
#define GETVAL     0x3002	/* Get semval. */
#define GETALL     0x3003	/* Get all cases of semval. */
#define GETZCNT    0x3004	/* Get semzcnt. */
#define SETVAL     0x3005	/* Set semval. */
#define SETALL     0x3006	/* Set all cases of semval. */

#define SEM_STAT   0x3010	/* For ipcs(8). */
#define SEM_INFO   0x3011	/* For ipcs(8). */

struct semid_ds
{
  struct ipc_perm  sem_perm;	/* Operation permission structure. */
  unsigned short   sem_nsems;	/* Number of semaphores in set. */
  timestruc_t      sem_otim;	/* Last semop () time. */
  timestruc_t      sem_ctim;	/* Last time changed by semctl (). */
  long             sem_spare4[2];
};

#define sem_otime sem_otim.tv_sec
#define sem_ctime sem_ctim.tv_sec

struct sembuf
{
  unsigned short  sem_num;	/* Semaphore number. */
  short           sem_op;	/* Semaphore operation. */
  short           sem_flg;	/* Operation flags. */
};

/* Buffer type for semctl (IPC_INFO, ...) as used by ipcs(8).
 */
struct seminfo
{
  unsigned long semmni;		/* Maximum number of unique semaphore
				   sets, system wide. */
  unsigned long semmns;		/* Maximum number of semaphores,
				   system wide. */
  unsigned long semmsl;		/* Maximum number of semaphores per
				   semaphore set. */
  unsigned long semopm;		/* Maximum number of operations per
				   semop call. */
  unsigned long semmnu;		/* Maximum number of undo structures,
				   system wide. */
  unsigned long semume;		/* Maximum number of undo entries per
				   undo structure. */
  unsigned long semvmx;		/* Maximum semaphore value. */
  unsigned long semaem;		/* Maximum adjust-on-exit value. */
  unsigned long sem_spare[4];
};

/* Buffer type for semctl (SEM_INFO, ...) as used by ipcs(8).
 */
struct sem_info
{
  unsigned long sem_ids;	/* Number of allocated semaphore sets. */
  unsigned long sem_num;	/* Number of allocated semaphores. */
};

int semctl (int semid, int semnum, int cmd, ...);
int semget (key_t key, int nsems, int semflg);
int semop (int semid, struct sembuf *sops, size_t nsops);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SEM_H */
