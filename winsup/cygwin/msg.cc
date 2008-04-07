/* msg.cc: XSI IPC interface for Cygwin.

   Copyright 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#ifdef USE_SERVER
#include <unistd.h>

#include "sigproc.h"
#include "cygtls.h"

#include "cygserver_msg.h"

/*
 * client_request_msg Constructors
 */

client_request_msg::client_request_msg (int msqid,
					int cmd,
					struct msqid_ds *buf)
  : client_request (CYGSERVER_REQUEST_MSG, &_parameters, sizeof (_parameters))
{
  _parameters.in.msgop = MSGOP_msgctl;
  ipc_set_proc_info (_parameters.in.ipcblk);

   _parameters.in.ctlargs.msqid = msqid;
   _parameters.in.ctlargs.cmd = cmd;
   _parameters.in.ctlargs.buf = buf;

  msglen (sizeof (_parameters.in));
}

client_request_msg::client_request_msg (key_t key,
					int msgflg)
  : client_request (CYGSERVER_REQUEST_MSG, &_parameters, sizeof (_parameters))
{
  _parameters.in.msgop = MSGOP_msgget;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.getargs.key = key;
  _parameters.in.getargs.msgflg = msgflg;

  msglen (sizeof (_parameters.in));
}

client_request_msg::client_request_msg (int msqid,
					void *msgp,
					size_t msgsz,
					long msgtyp,
					int msgflg)
  : client_request (CYGSERVER_REQUEST_MSG, &_parameters, sizeof (_parameters))
{
  _parameters.in.msgop = MSGOP_msgrcv;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.rcvargs.msqid = msqid;
  _parameters.in.rcvargs.msgp = msgp;
  _parameters.in.rcvargs.msgsz = msgsz;
  _parameters.in.rcvargs.msgtyp = msgtyp;
  _parameters.in.rcvargs.msgflg = msgflg;

  msglen (sizeof (_parameters.in));
}

client_request_msg::client_request_msg (int msqid,
					const void *msgp,
					size_t msgsz,
					int msgflg)
  : client_request (CYGSERVER_REQUEST_MSG, &_parameters, sizeof (_parameters))
{
  _parameters.in.msgop = MSGOP_msgsnd;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.sndargs.msqid = msqid;
  _parameters.in.sndargs.msgp = msgp;
  _parameters.in.sndargs.msgsz = msgsz;
  _parameters.in.sndargs.msgflg = msgflg;

  msglen (sizeof (_parameters.in));
}
#endif /* USE_SERVER */

/*
 * XSI message queue API.  These are exported by the DLL.
 */

extern "C" int
msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
#ifdef USE_SERVER
  syscall_printf ("msgctl (msqid = %d, cmd = 0x%x, buf = %p)",
		  msqid, cmd, buf);
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  switch (cmd)
    {
      case IPC_STAT:
	break;
      case IPC_SET:
	break;
      case IPC_RMID:
	break;
      case IPC_INFO:
	break;
      case MSG_INFO:
	break;
      default:
	syscall_printf ("-1 [%d] = msgctl ()", EINVAL);
	set_errno (EINVAL);
	return -1;
    }
  client_request_msg request (msqid, cmd, buf);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = msgctl ()", request.error_code ());
      set_errno (request.error_code ());
      if (request.error_code () == ENOSYS)
	raise (SIGSYS);
      return -1;
    }
  return request.retval ();
#else
  set_errno (ENOSYS);
  raise (SIGSYS);
  return -1;
#endif
}

extern "C" int
msgget (key_t key, int msgflg)
{
#ifdef USE_SERVER
  syscall_printf ("msgget (key = %U, msgflg = 0x%x)", key, msgflg);
  client_request_msg request (key, msgflg);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = msgget ()", request.error_code ());
      set_errno (request.error_code ());
      if (request.error_code () == ENOSYS)
	raise (SIGSYS);
      return -1;
    }
  return request.retval ();
#else
  set_errno (ENOSYS);
  raise (SIGSYS);
  return -1;
#endif
}

extern "C" ssize_t
msgrcv (int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
#ifdef USE_SERVER
  syscall_printf ("msgrcv (msqid = %d, msgp = %p, msgsz = %d, "
		  "msgtyp = %d, msgflg = 0x%x)",
		  msqid, msgp, msgsz, msgtyp, msgflg);
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  client_request_msg request (msqid, msgp, msgsz, msgtyp, msgflg);
  if (request.make_request () == -1 || request.rcvval () == -1)
    {
      syscall_printf ("-1 [%d] = msgrcv ()", request.error_code ());
      set_errno (request.error_code ());
      if (request.error_code () == ENOSYS)
	raise (SIGSYS);
      return -1;
    }
  return request.rcvval ();
#else
  set_errno (ENOSYS);
  raise (SIGSYS);
  return -1;
#endif
}

extern "C" int
msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
#ifdef USE_SERVER
  syscall_printf ("msgsnd (msqid = %d, msgp = %p, msgsz = %d, msgflg = 0x%x)",
		  msqid, msgp, msgsz, msgflg);
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  client_request_msg request (msqid, msgp, msgsz, msgflg);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = msgsnd ()", request.error_code ());
      set_errno (request.error_code ());
      if (request.error_code () == ENOSYS)
	raise (SIGSYS);
      return -1;
    }
  return request.retval ();
#else
  set_errno (ENOSYS);
  raise (SIGSYS);
  return -1;
#endif
}
