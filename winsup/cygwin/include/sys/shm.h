/* sys/shm.h

   Copyright 2001, 2002 Red Hat Inc. 
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <sys/ipc.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 64 Kb was hardcoded for x86. MS states this may change, but we need
 * it in the header file.
 */
#define SHMLBA 65536

/*
 * Values for the shmflg argument to shmat(2).
 */
#define SHM_RDONLY 0x01		/* Attach read-only, not read/write. */
#define SHM_RND    0x02		/* Round shmaddr down to multiple of SHMLBA. */

/*
 * Values for the cmd argument to shmctl(2).
 * Commands 4000-4fff are reserved for SHM_xxx.
 */
#define SHM_STAT   0x4000	/* For ipcs(8) */

typedef long int shmatt_t;

struct shmid_ds {
  struct ipc_perm shm_perm;
  size_t          shm_segsz;
  pid_t           shm_lpid;
  pid_t           shm_cpid;
  shmatt_t        shm_nattch;
  timestruc_t     shm_atim;
  timestruc_t     shm_dtim;
  timestruc_t     shm_ctim;
  long            shm_spare4[2];
};

#define shm_atime shm_atim.tv_sec
#define shm_dtime shm_dtim.tv_sec
#define shm_ctime shm_ctim.tv_sec

/* Buffer type for shmctl(IPC_INFO, ...) as used by ipcs(8). */
struct shminfo {
  unsigned long shmmax;
  unsigned long shmmin;
  unsigned long shmmni;
  unsigned long shmseg;
  unsigned long shmall;
  unsigned long shm_spare[4];
};

void *shmat(int shmid, const void *shmaddr, int shmflg);
int   shmctl(int shmid, int cmd, struct shmid_ds *buf);
int   shmdt(const void *shmaddr);
int   shmget(key_t key, size_t size, int shmflg);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SHM_H */
