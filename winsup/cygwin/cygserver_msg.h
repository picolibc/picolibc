/* cygserver_msg.h: Single unix specification IPC interface for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGSERVER_MSG_H__
#define __CYGSERVER_MSG_H__

#include <sys/types.h>
#include <sys/sysproto.h>
#ifndef _KERNEL
#define _KERNEL 1
#endif
#include <cygwin/msg.h>

#include "cygserver.h"
#include "cygserver_ipc.h"

#ifndef __INSIDE_CYGWIN__
class transport_layer_base;
class process_cache;
#endif

class client_request_msg : public client_request
{
  friend class client_request;

public:
  enum msgop_t
    {
      MSGOP_msgctl,
      MSGOP_msgget,
      MSGOP_msgrcv,
      MSGOP_msgsnd
    };

private:
  union
  {
    struct
    {
      msgop_t msgop;
      proc ipcblk;
      union
      {
	struct msgctl_args ctlargs;
	struct msgget_args getargs;
	struct msgrcv_args rcvargs;
	struct msgsnd_args sndargs;
      };
    } in;

    union {
      int ret;
      ssize_t rcv;
    } out;
  } _parameters;

#ifndef __INSIDE_CYGWIN__
  client_request_msg ();
  virtual void serve (transport_layer_base *, process_cache *);
#endif

public:

#ifdef __INSIDE_CYGWIN__
  client_request_msg (int, int, struct msqid_ds *);	// msgctl
  client_request_msg (key_t, int);			// msgget
  client_request_msg (int, void *, size_t, long, int);	// msgrcv
  client_request_msg (int, const void *, size_t, int);	// msgsnd
#endif

  int retval () const { return msglen () ? _parameters.out.ret : -1; }
  ssize_t rcvval () const { return _parameters.out.rcv; }
};

#ifndef __INSIDE_CYGWIN__
int msginit ();
int msgunload ();
int msgctl (struct thread *, struct msgctl_args *);
int msgget (struct thread *, struct msgget_args *);
int msgsnd (struct thread *, struct msgsnd_args *);
int msgrcv (struct thread *, struct msgrcv_args *);
#endif

#endif /* __CYGSERVER_MSG_H__ */
