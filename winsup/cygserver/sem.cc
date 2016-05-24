/* sem.cc: Single unix specification IPC interface for Cygwin.

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
#include "cygserver_sem.h"

client_request_sem::client_request_sem ()
  : client_request (CYGSERVER_REQUEST_SEM,
		    &_parameters, sizeof (_parameters))
{ 
}

void
client_request_sem::serve (transport_layer_base *const conn,
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
  if (support_semaphores == TUN_FALSE)
    {
      syscall_printf ("Semaphore support not started");
      error_code (ENOSYS);
      _parameters.out.ret = -1;
      msglen (sizeof (_parameters.out));
      return;
    }
  process *const client = cache->process (_parameters.in.ipcblk.cygpid,
					  _parameters.in.ipcblk.winpid);
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
  /* sysv_sem.cc takes care of itself. */
  client->release ();
  thread td (client, &_parameters.in.ipcblk, true);
  int res;
  switch (_parameters.in.semop)
    {
      case SEMOP_semctl:
	res = semctl (&td, &_parameters.in.ctlargs);
        break;
      case SEMOP_semget:
	res = semget (&td, &_parameters.in.getargs);
        break;
      case SEMOP_semop:
	res = semop (&td, &_parameters.in.opargs);
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
  _parameters.out.ret = td.td_retval[0];
  msglen (sizeof (_parameters.out));
}
#endif /* __OUTSIDE_CYGWIN__ */
