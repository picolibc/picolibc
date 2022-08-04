/* cygserver_sem.h: Single unix specification IPC interface for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_SEM_H__
#define __CYGSERVER_SEM_H__

#include <sys/types.h>
#include <sys/sysproto.h>
#ifndef _KERNEL
#define _KERNEL 1
#endif
#include <cygwin/sem.h>

#include "cygserver.h"
#include "cygserver_ipc.h"

#ifndef __INSIDE_CYGWIN__
class transport_layer_base;
class process_cache;
#endif

class client_request_sem : public client_request
{
  friend class client_request;

public:
  enum semop_t
    {
      SEMOP_semctl,
      SEMOP_semget,
      SEMOP_semop
    };

private:
  union
  {
    struct
    {
      semop_t semop;
      proc ipcblk;
      union
      {
	struct semctl_args ctlargs;
	struct semget_args getargs;
	struct semop_args  opargs;
      };
    } in;

    union {
      int ret;
    } out;
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_sem ();
  virtual void serve (transport_layer_base *, process_cache *);
#endif

public:

#ifdef __INSIDE_CYGWIN__
  client_request_sem (int, int, int, union semun *);	// semctl
  client_request_sem (key_t, int, int);			// semget
  client_request_sem (int, struct sembuf *, size_t);	// semop
#endif

  int retval () const { return msglen () ? _parameters.out.ret : -1; }
};

#ifndef __INSIDE_CYGWIN__
int seminit ();
int semunload ();
void semexit_myhook(void *arg, struct proc *p);

int semctl (struct thread *, struct semctl_args *);
int semget (struct thread *, struct semget_args *);
int semop (struct thread *, struct semop_args *);
#endif

#endif /* __CYGSERVER_SEM_H__ */
