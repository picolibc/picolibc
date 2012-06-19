/* cygserver_ipc.h

   Copyright 2002, 2003, 2004, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_IPC_H__
#define __CYGSERVER_IPC_H__

/*
 * Datastructure which is part of any IPC input parameter block.
 */
struct vmspace {
  void *vm_map;			/* UNUSED */
  struct shmmap_state *vm_shm;
};

struct proc {
  pid_t cygpid;
  DWORD winpid;
  __uid32_t uid;
  __gid32_t gid;
  int gidcnt;
  __gid32_t *gidlist;
  bool is_admin;
  struct vmspace *p_vmspace;
  HANDLE signal_arrived;
};

#ifdef __INSIDE_CYGWIN__
#include "sigproc.h"
inline void
ipc_set_proc_info (proc &blk)
{
  blk.cygpid = getpid ();
  blk.winpid = GetCurrentProcessId ();
  blk.uid = geteuid32 ();
  blk.gid = getegid32 ();
  blk.gidcnt = 0;
  blk.gidlist = NULL;
  blk.is_admin = false;
  blk.signal_arrived = signal_arrived;
}
#endif /* __INSIDE_CYGWIN__ */

#ifndef __INSIDE_CYGWIN__
class ipc_retval {
private:
  union {
    int i;
    unsigned int u;
    vm_offset_t off;
    vm_object_t obj;
  };

public:
  ipc_retval (int ni) { i = ni; }

  operator int () const { return i; }
  int operator = (int ni) { return i = ni; }

  operator unsigned int () const { return u; }
  unsigned int operator = (unsigned int nu) { return u = nu; }

  operator vm_offset_t () const { return off; }
  vm_offset_t operator = (vm_offset_t noff) { return off = noff; }

  operator vm_object_t () const { return obj; }
  vm_object_t operator = (vm_object_t nobj) { return obj = nobj; }
};

struct thread {
  class process *client;
  proc *ipcblk;
  ipc_retval td_retval[2];
};
#define td_proc ipcblk
#define p_pid cygpid
#endif

#endif /* __CYGSERVER_IPC_H__ */
