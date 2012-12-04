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

#pragma pack (push, 4)
struct proc {
  pid_t cygpid;
  DWORD winpid;
  bool is_64bit;
  bool is_admin;
  uid_t uid;
  gid_t gid;
  int gidcnt;
  _TYPE64 (HANDLE, signal_arrived);
  /* Only used internally. */
  _TYPE64 (gid_t *, gidlist);
  _TYPE64 (struct vmspace *, p_vmspace);
};
#pragma pack (pop)

#ifdef __INSIDE_CYGWIN__
#include "sigproc.h"
inline void
ipc_set_proc_info (proc &blk)
{
  blk.cygpid = getpid ();
  blk.winpid = GetCurrentProcessId ();
#ifdef __x86_64__
  blk.is_64bit = true;
#else
  blk.is_64bit = false;
#endif
  blk.is_admin = false;
  blk.uid = geteuid32 ();
  blk.gid = getegid32 ();
  blk.__signal_arrived_align = 0;
  _my_tls.set_signal_arrived (true, blk.signal_arrived);
  blk.gidcnt = 0;
  blk.__gidlist_align = 0;
}
#endif /* __INSIDE_CYGWIN__ */

#ifndef __INSIDE_CYGWIN__
class ipc_retval {
private:
  union {
    int i;
    size_t sz;
    vm_offset_t off;
    vm_object_t obj;
  };

public:
  ipc_retval (int ni) { i = ni; }

  operator int () const { return i; }
  int operator = (int ni) { return i = ni; }

  operator size_t () const { return sz; }
  size_t operator = (size_t nsz) { return sz = nsz; }

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
