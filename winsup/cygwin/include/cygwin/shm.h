/* sys/shm.h

   Copyright 2001, 2002 Red Hat Inc.
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <cygwin/ipc.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* 64 Kb was hardcoded for x86. MS states this may change, but we need
 * it in the header file.
 */
#define SHMLBA 65536		/* Segment low boundary address multiple. */

/* Shared memory operation flags:
 */
#define SHM_RDONLY 0x01		/* Attach read-only (else read-write). */
#define SHM_RND    0x02		/* Round attach address to SHMLBA. */

/* Command definitions for the semctl () function:
 */
#define SHM_STAT   0x4000	/* For ipcs(8) */
#define SHM_INFO   0x4001	/* For ipcs(8) */

/* Unsigned integer used for the number of current attaches.
 */
typedef unsigned int shmatt_t;

struct shmid_ds
{
  struct ipc_perm shm_perm;	/* Operation permission structure. */
  size_t          shm_segsz;	/* Size of segment in bytes. */
  pid_t           shm_lpid;	/* Process ID of last operation. */
  pid_t           shm_cpid;	/* Process ID of creator. */
  shmatt_t        shm_nattch;	/* Number of current attaches. */
  timestruc_t     shm_atim;	/* Time of last shmat (). */
  timestruc_t     shm_dtim;	/* Time of last shmdt (). */
  timestruc_t     shm_ctim;	/* Time of last change by shmctl (). */
  long            shm_spare4[2];
};

#define shm_atime shm_atim.tv_sec
#define shm_dtime shm_dtim.tv_sec
#define shm_ctime shm_ctim.tv_sec

/* Buffer type for shmctl (IPC_INFO, ...) as used by ipcs(8).
 */
struct shminfo
{
  unsigned long shmmax;		/* Maximum size in bytes of a shared
				   memory segment. */
  unsigned long shmmin;		/* Minimum size in bytes of a shared
				   memory segment. */
  unsigned long shmmni;		/* Maximum number of shared memory
				   segments, system wide. */
  unsigned long shmseg;		/* Maximum number of shared memory
				   segments attached per process. */
  unsigned long shmall;		/* Maximum number of bytes of shared
				   memory, system wide. */
  unsigned long shm_spare[4];
};

/* Buffer type for shmctl (SHM_INFO, ...) as used by ipcs(8).
 */
struct shm_info
{
  unsigned long shm_ids;	/* Number of allocated segments. */
  unsigned long shm_tot;	/* Size in bytes of allocated segments. */
  unsigned long shm_atts;	/* Number of attached segments, system
				   wide. */
};

void *shmat (int shmid, const void *shmaddr, int shmflg);
int   shmctl (int shmid, int cmd, struct shmid_ds *buf);
int   shmdt (const void *shmaddr);
int   shmget (key_t key, size_t size, int shmflg);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SHM_H */
