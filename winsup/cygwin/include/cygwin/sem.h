/* sys/sem.h
   Written by Conrad Scott <conrad.scott@dsl.pipex.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_SEM_H
#define _CYGWIN_SEM_H

#include <cygwin/ipc.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Semaphore operation flags:
 */
#define SEM_UNDO   010000	/* Set up adjust on exit entry. */

/* Command definitions for the semctl () function:
 */
#define GETNCNT    0x3000	/* Get semncnt. */
#define GETPID     0x3001	/* Get sempid. */
#define GETVAL     0x3002	/* Get semval. */
#define GETALL     0x3003	/* Get all cases of semval. */
#define GETZCNT    0x3004	/* Get semzcnt. */
#define SETVAL     0x3005	/* Set semval. */
#define SETALL     0x3006	/* Set all cases of semval. */

#ifdef _KERNEL
#define SEM_STAT   0x3010	/* For ipcs(8). */
#define SEM_INFO   0x3011	/* For ipcs(8). */
#endif /* _KERNEL */

struct semid_ds
{
  struct ipc_perm  sem_perm;	/* Operation permission structure. */
  unsigned short   sem_nsems;	/* Number of semaphores in set. */
  timestruc_t      sem_otim;	/* Last semop () time. */
  timestruc_t      sem_ctim;	/* Last time changed by semctl (). */
#ifdef _KERNEL
  struct sem	  *sem_base;	/* pointer to first semaphore in set */
  long             sem_spare4[1];
#else
  long             sem_spare4[2];
#endif /* _KERNEL */
};

#define sem_otime sem_otim.tv_sec
#define sem_ctime sem_ctim.tv_sec

struct sembuf
{
  unsigned short  sem_num;	/* Semaphore number. */
  short           sem_op;	/* Semaphore operation. */
  short           sem_flg;	/* Operation flags. */
};

#ifdef _KERNEL
/* Buffer type for semctl (IPC_INFO, ...) as used by ipcs(8).
 */
struct seminfo
{
  int32_t semmni;	/* Maximum number of unique semaphore
			   sets, system wide. */
  int32_t semmns;	/* Maximum number of semaphores,
			   system wide. */
  int32_t semmsl;	/* Maximum number of semaphores per
			   semaphore set. */
  int32_t semopm;	/* Maximum number of operations per
			   semop call. */
  int32_t semmnu;	/* Maximum number of undo structures,
			   system wide. */
  int32_t semume;	/* Maximum number of undo entries per
			   undo structure. */
  int32_t semvmx;	/* Maximum semaphore value. */
  int32_t semaem;	/* Maximum adjust-on-exit value. */
  int32_t semmap;	/* # of entries in semaphore map */
  int32_t semusz;	/* size in bytes of undo structure */
  int32_t sem_spare[2];
};

/* Buffer type for semctl (SEM_INFO, ...) as used by ipcs(8).
 */
struct sem_info
{
  long sem_ids;		/* Number of allocated semaphore sets. */
  long sem_num;		/* Number of allocated semaphores. */
};

/* Permission flags */
#define SEM_A		IPC_W	/* alter permission */
#define SEM_R		IPC_R	/* read permission */

/* Internally used mode bits. */
#define	SEM_ALLOC	01000	/* semaphore is allocated */

#endif /* _KERNEL */

#ifdef _KERNEL
/* According to SUSv3, "the fourth argument [to semctl()] is optional and
   depends upon the operation requested. If required, it is of type
   union semun, which the application shall explicitly declare:" */
union semun {
    int val;			/* value for SETVAL */
    struct semid_ds *buf;	/* buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;	/* array for GETALL, SETALL */
};
/* Therefore this union is only declared here if building internal code.
   _KERNEL must not be defined in exernal applications!  Declare union
   semun explicitely as required by SUSv3, please. */
#endif /* _KERNEL */

int semctl (int semid, int semnum, int cmd, ...);
int semget (key_t key, int nsems, int semflg);
int semop (int semid, struct sembuf *sops, size_t nsops);

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_SEM_H */
