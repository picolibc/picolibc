/* sys/shm.h

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

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <sys/ipc.h>

#define SHM_RDONLY 1
/* 64 Kb was hardcoded for x86. MS states this may change, but we need it in the header
 * file.
 */
#define SHMLBA     65536
#define SHM_RND	   1

typedef long int shmatt_t;

#if defined(__INSIDE_CYGWIN__) && defined(__cplusplus)

class _shmattach {
public:
  void *data;
  int shmflg;
  class _shmattach *next;
};

class shmid_ds {
public:
  struct   ipc_perm shm_perm;
  size_t   shm_segsz;
  pid_t    shm_lpid;
  pid_t    shm_cpid;
  shmatt_t shm_nattch;
  time_t   shm_atime;
  time_t   shm_dtime;
  time_t   shm_ctime;
  void *mapptr;
};

class shmnode {
public:
  class shmid_ds * shmds;
  int shm_id;
  class shmnode *next;
  key_t key;
  HANDLE filemap;
  HANDLE attachmap;
  class _shmattach *attachhead;
};

#else
/* this is what we return when queried. It has no bitwise correspondence
 * the internal structures 
 */
struct shmid_ds {
  struct   ipc_perm shm_perm;
  size_t   shm_segsz;
  pid_t    shm_lpid;
  pid_t    shm_cpid;
  shmatt_t shm_nattch;
  time_t   shm_atime;
  time_t   shm_dtime;
  time_t   shm_ctime;
};
#endif /* __INSIDE_CYGWIN__ */

void *shmat(int, const void *, int);
int   shmctl(int, int, struct shmid_ds *);
int   shmdt(const void *);
int   shmget(key_t, size_t, int);

#endif /* _SYS_SHM_H */

#ifdef __cplusplus
}
#endif
