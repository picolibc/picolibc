/* shm.cc: Single unix specification IPC interface for Cygwin.

   Copyright 2003, 2004, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cygserver.h"
#include "process.h"
#include "transport.h"

#include "cygserver_ipc.h"
#include "cygserver_shm.h"

client_request_shm::client_request_shm ()
  : client_request (CYGSERVER_REQUEST_SHM,
		    &_parameters, sizeof (_parameters))
{ 
}

void
client_request_shm::serve (transport_layer_base *const conn,
                           process_cache *const cache)
{
  if (msglen () != sizeof (_parameters.in))
    {
      syscall_printf ("bad request body length: expecting %lu bytes, got %lu",
		      sizeof (_parameters), msglen ());
      error_code (EINVAL);
      msglen (0);
      return;
    }
  if (support_sharedmem == TUN_FALSE)
    {
      syscall_printf ("Shared memory support not started");
      error_code (ENOSYS);
      if (_parameters.in.shmop == SHMOP_shmat)
	_parameters.out.ptr = (vm_offset_t)0;
      else
	_parameters.out.ret = -1;
      msglen (sizeof (_parameters.out));
      return;
    }
  process *const client = cache->process (_parameters.in.ipcblk.cygpid,
					  _parameters.in.ipcblk.winpid,
					  _parameters.in.ipcblk.signal_arrived);
  if (!client)
    {
      error_code (EAGAIN);
      msglen (0);
      return;
    }
  if (!conn->impersonate_client ())
    {
      client->release ();
      error_code (EACCES);
      msglen (0);
      return;
    }
  if (!adjust_identity_info (&_parameters.in.ipcblk))
    {
      client->release ();
      conn->revert_to_self ();
      error_code (EACCES);
      msglen (0);
      return;
    }
  /* Early revert_to_self since IPC code runs in kernel mode. */
  conn->revert_to_self ();
  /* sysv_shm.cc takes care of itself. */
  client->release ();
  thread td = { client, &_parameters.in.ipcblk, {0, 0} };
  int res;
  shmop_t shmop = _parameters.in.shmop; /* Get's overwritten otherwise. */
  switch (shmop)
    {
      case SHMOP_shmat:
	ipc_p_vmspace (td.ipcblk);
	res = shmat (&td, &_parameters.in.atargs);
        break;
      case SHMOP_shmctl:
	res = shmctl (&td, &_parameters.in.ctlargs);
        break;
      case SHMOP_shmdt:
	ipc_p_vmspace (td.ipcblk);
	res = shmdt (&td, &_parameters.in.dtargs);
        break;
      case SHMOP_shmget:
	res = shmget (&td, &_parameters.in.getargs);
        break;
      case SHMOP_shmfork:
        res = cygwin_shmfork_myhook (&td, &_parameters.in.forkargs);
	break;
      default:
	res = ENOSYS;
        td.td_retval[0] = -1;
	break;
    }
  /* Allocated by the call to adjust_identity_info(). */
  if (_parameters.in.ipcblk.gidlist)
    free (_parameters.in.ipcblk.gidlist);
  error_code (res);
  if (shmop == SHMOP_shmat)
    _parameters.out.ptr = td.td_retval[0];
  else
    _parameters.out.ret = td.td_retval[0];
  if (shmop == SHMOP_shmget)
    _parameters.out.obj = td.td_retval[1];
  msglen (sizeof (_parameters.out));
}
#endif /* __OUTSIDE_CYGWIN__ */
