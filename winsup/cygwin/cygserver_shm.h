/* cygserver_shm.h: Single unix specification IPC interface for Cygwin.

   Copyright 2001, 2002, 2003, 2005, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_SHM_H__
#define __CYGSERVER_SHM_H__

#include <sys/types.h>
#include <sys/sysproto.h>
#ifndef _KERNEL
#define _KERNEL 1
#endif
#include <cygwin/shm.h>

#include "cygserver.h"
#include "cygserver_ipc.h"

#ifndef __INSIDE_CYGWIN__
class transport_layer_base;
class process_cache;
#endif

class client_request_shm : public client_request
{
  friend class client_request;

public:
  enum shmop_t
    {
      SHMOP_shmat,
      SHMOP_shmctl,
      SHMOP_shmdt,
      SHMOP_shmget,
      SHMOP_shmfork	/* Called on fixup_after_fork */
    };

private:
  union
  {
    struct
    {
      shmop_t shmop;
      proc ipcblk;
      struct shmat_args  atargs;
      struct shmctl_args ctlargs;
      struct shmdt_args  dtargs;
      struct shmget_args getargs;
      struct proc        forkargs;
    } in;

    struct {
      union {
	int ret;
	vm_offset_t ptr;
      };
      vm_object_t obj;
    } out;
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_shm ();
  virtual void serve (transport_layer_base *, process_cache *);
#endif

public:

#ifdef __INSIDE_CYGWIN__
  client_request_shm (int, const void *, int);		// shmat
  client_request_shm (int, int, struct shmid_ds *);	// shmctl
  client_request_shm (const void *);			// shmdt
  client_request_shm (key_t, size_t, int);		// shmget
  client_request_shm (proc *);				// shmfork
#endif

  int retval () const { return msglen () ? _parameters.out.ret : -1; }
  void *ptrval () const { return (void *)_parameters.out.ptr; }
  vm_object_t objval () const { return _parameters.out.obj; }
};

#ifndef __INSIDE_CYGWIN__
void shminit ();
int shmunload ();
void shmexit_myhook (struct vmspace *vm);
int cygwin_shmfork_myhook (struct thread *, struct proc *);

int shmat (struct thread *, struct shmat_args *);
int shmctl (struct thread *, struct shmctl_args *);
int shmdt (struct thread *, struct shmdt_args *);
int shmget (struct thread *, struct shmget_args *);
#endif

#endif /* __CYGSERVER_SHM_H__ */
