/* sys/shm.h

   Copyright 2001, 2002 Red Hat Inc.
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_SHM_H
#define _CYGWIN_SHM_H

#include <cygwin/ipc.h>
#include <sys/cygwin.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Segment low boundary address multiple.
 *
 * 64 Kb was hardcoded for x86. MS states this may change so the constant
 * expression is replaced by a function call returning the correct value. */
#define SHMLBA	(cygwin_internal (CW_GET_SHMLBA))

/* Shared memory operation flags:
 */
#define SHM_RDONLY 0x01		/* Attach read-only (else read-write). */
#define SHM_RND    0x02		/* Round attach address to SHMLBA. */

#ifdef _KERNEL
/* Command definitions for the semctl () function:
 */
#define SHM_STAT   0x4000	/* For ipcs(8) */
#define SHM_INFO   0x4001	/* For ipcs(8) */
#endif /* _KERNEL */

/* Unsigned integer used for the number of current attaches.
 */
typedef unsigned int shmatt_t;

struct shmid_ds
{
  struct ipc_perm    shm_perm;	/* Operation permission structure. */
  size_t             shm_segsz;	/* Size of segment in bytes. */
  pid_t              shm_lpid;	/* Process ID of last operation. */
  pid_t              shm_cpid;	/* Process ID of creator. */
  shmatt_t           shm_nattch;/* Number of current attaches. */
  timestruc_t        shm_atim;	/* Time of last shmat (). */
  timestruc_t        shm_dtim;	/* Time of last shmdt (). */
  timestruc_t        shm_ctim;	/* Time of last change by shmctl (). */
#ifdef _KERNEL
  struct shm_handle *shm_internal;
  long               shm_spare4[1];
#else
  long               shm_spare4[2];
#endif /* _KERNEL */
};

#define shm_atime shm_atim.tv_sec
#define shm_dtime shm_dtim.tv_sec
#define shm_ctime shm_ctim.tv_sec

#ifdef _KERNEL
/* Buffer type for shmctl (IPC_INFO, ...) as used by ipcs(8).
 */
struct shminfo
{
  long shmmax;		/* Maximum size in bytes of a shared
			   memory segment. */
  long shmmin;		/* Minimum size in bytes of a shared
			   memory segment. */
  long shmmni;		/* Maximum number of shared memory
			   segments, system wide. */
  long shmseg;		/* Maximum number of shared memory
			   segments attached per process. */
  long shmall;		/* Maximum number of bytes of shared
			   memory, system wide. */
  long shm_spare[4];
};

/* Buffer type for shmctl (SHM_INFO, ...) as used by ipcs(8).
 */
struct shm_info
{
#define shm_ids used_ids
  long used_ids;	/* Number of allocated segments. */
  long shm_tot;		/* Size in bytes of allocated segments. */
  long shm_atts;	/* Number of attached segments, system
			   wide. */
};
#endif /* _KERNEL */

void *shmat (int shmid, const void *shmaddr, int shmflg);
int   shmctl (int shmid, int cmd, struct shmid_ds *buf);
int   shmdt (const void *shmaddr);
int   shmget (key_t key, size_t size, int shmflg);

#ifdef __cplusplus
}
#endif

#endif /* _CYGWIN_SHM_H */
