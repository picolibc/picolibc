/* sem.cc: XSI IPC interface for Cygwin.

   Copyright 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#ifdef USE_SERVER
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "sigproc.h"

#include "cygserver_ipc.h"
#include "cygserver_sem.h"

/*
 * client_request_sem Constructors
 */

client_request_sem::client_request_sem (int semid,
					int semnum,
					int cmd,
					union semun *arg)
  : client_request (CYGSERVER_REQUEST_SEM, &_parameters, sizeof (_parameters))
{
  _parameters.in.semop = SEMOP_semctl;
  ipc_set_proc_info (_parameters.in.ipcblk);

   _parameters.in.ctlargs.semid = semid;
   _parameters.in.ctlargs.semnum = semnum;
   _parameters.in.ctlargs.cmd = cmd;
   _parameters.in.ctlargs.arg = arg;

  msglen (sizeof (_parameters.in));
}

client_request_sem::client_request_sem (key_t key,
					int nsems,
					int semflg)
  : client_request (CYGSERVER_REQUEST_SEM, &_parameters, sizeof (_parameters))
{
  _parameters.in.semop = SEMOP_semget;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.getargs.key = key;
  _parameters.in.getargs.nsems = nsems;
  _parameters.in.getargs.semflg = semflg;

  msglen (sizeof (_parameters.in));
}

client_request_sem::client_request_sem (int semid,
					struct sembuf *sops,
					size_t nsops)
  : client_request (CYGSERVER_REQUEST_SEM, &_parameters, sizeof (_parameters))
{
  _parameters.in.semop = SEMOP_semop;
  ipc_set_proc_info (_parameters.in.ipcblk);

  _parameters.in.opargs.semid = semid;
  _parameters.in.opargs.sops = sops;
  _parameters.in.opargs.nsops = nsops;

  msglen (sizeof (_parameters.in));
}
#endif /* USE_SERVER */

/*
 * XSI semaphore API.  These are exported by the DLL.
 */

extern "C" int
semctl (int semid, int semnum, int cmd, ...)
{
#ifdef USE_SERVER
  sigframe thisframe (mainthread);
  union semun arg = {0};
  if (cmd == IPC_STAT || cmd == IPC_SET || cmd == IPC_INFO || cmd == SEM_INFO
      || cmd == GETALL || cmd == SETALL || cmd == SETVAL)
    {
      va_list ap;
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
    }
  syscall_printf ("semctl (semid = %d, semnum = %d, cmd = %d, arg.val = 0x%x)",
		  semid, semnum, cmd, arg.val);
  if ((cmd == IPC_STAT || cmd == IPC_SET)
      && __check_null_invalid_struct_errno (arg.buf, sizeof (struct semid_ds)))
    return -1;
  if (cmd == IPC_INFO)
    {
      /* semid == 0: Request for seminfo struct. */
      if (!semid
	  && __check_null_invalid_struct_errno (arg.buf, sizeof (struct seminfo)))
	  return -1;
      /* Otherwise, request semid entries from internal semid_ds array. */
      if (semid)
	if (__check_null_invalid_struct_errno (arg.buf, semid * sizeof (struct semid_ds)))
	  return -1;
    }
  if (cmd == SEM_INFO
      && __check_null_invalid_struct_errno (arg.buf, sizeof (struct sem_info)))
    return -1;
  client_request_sem request (semid, semnum, cmd, &arg);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = semctl ()", request.error_code ());
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
semget (key_t key, int nsems, int semflg)
{
#ifdef USE_SERVER
  sigframe thisframe (mainthread);
  syscall_printf ("semget (key = %U, nsems = %d, semflg = 0x%x)",
		  key, nsems, semflg);
  client_request_sem request (key, nsems, semflg);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = semctl ()", request.error_code ());
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
semop (int semid, struct sembuf *sops, size_t nsops)
{
#ifdef USE_SERVER
  sigframe thisframe (mainthread);
  syscall_printf ("semop (semid = %d, sops = %p, nsops = %d)",
		  semid, sops, nsops);
  if (__check_null_invalid_struct_errno (sops, nsops * sizeof (struct sembuf)))
    return -1;
  client_request_sem request (semid, sops, nsops);
  if (request.make_request () == -1 || request.retval () == -1)
    {
      syscall_printf ("-1 [%d] = semctl ()", request.error_code ());
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
