/* cygserver.cc

   Copyright 2001 Red Hat Inc.

   Written by Egor Duda <deo@logos-m.ru>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include "wincap.h"
#include "cygwin_version.h"

#include "getopt.h"

#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_pipes.h"
#include "cygwin/cygserver_transport_sockets.h"
#include "threaded_queue.h"
#include "cygwin/cygserver_process.h"
#include "cygwin/cygserver.h"
#include "cygserver_shm.h"

/* for quieter operation, set to 0 */
#define DEBUG 0
#define debug_printf if (DEBUG) printf

GENERIC_MAPPING access_mapping;
class transport_layer_base *transport;

DWORD request_count = 0;

BOOL
setup_privileges ()
{
  BOOL rc, ret_val;
  HANDLE hToken = NULL;
  TOKEN_PRIVILEGES sPrivileges;

  rc = OpenProcessToken ( GetCurrentProcess() , TOKEN_ALL_ACCESS , &hToken ) ;
  if ( !rc )
    {
      printf ( "error opening process token (%lu)\n", GetLastError () );
      ret_val = FALSE;
      goto out;
    }
  rc = LookupPrivilegeValue ( NULL, SE_DEBUG_NAME, &sPrivileges.Privileges[0].Luid );
  if ( !rc )
    {
      printf ( "error getting prigilege luid (%lu)\n", GetLastError () );
      ret_val = FALSE;
      goto out;
    }
  sPrivileges.PrivilegeCount = 1 ;
  sPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
  rc = AdjustTokenPrivileges ( hToken, FALSE, &sPrivileges, 0, NULL, NULL ) ;
  if ( !rc )
    {
      printf ( "error adjusting prigilege level. (%lu)\n", GetLastError () );
      ret_val = FALSE;
      goto out;
    }

  access_mapping.GenericRead = FILE_READ_DATA;
  access_mapping.GenericWrite = FILE_WRITE_DATA;
  access_mapping.GenericExecute = 0;
  access_mapping.GenericAll = FILE_READ_DATA | FILE_WRITE_DATA;

  ret_val = TRUE;

out:
  CloseHandle ( hToken );
  return ret_val;
}

int
check_and_dup_handle (HANDLE from_process, HANDLE to_process,
		      HANDLE from_process_token,
                      DWORD access,
                      HANDLE from_handle,
                      HANDLE* to_handle_ptr, BOOL bInheritHandle)
{
  HANDLE local_handle = NULL;
  int ret_val = EACCES;
  char sd_buf [1024];
  PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR) &sd_buf;
  DWORD bytes_needed;
  PRIVILEGE_SET ps;
  DWORD ps_len = sizeof (ps);
  BOOL status;

  if (from_process != GetCurrentProcess ())
{ 

  if (!DuplicateHandle (from_process, from_handle,
	                GetCurrentProcess (), &local_handle,
                        0, bInheritHandle,
                        DUPLICATE_SAME_ACCESS))
    {
      printf ( "error getting handle(%u) to server (%lu)\n", (unsigned int)from_handle, GetLastError ());
      goto out;
    }
} else
 local_handle = from_handle;

  if (!GetKernelObjectSecurity (local_handle,
  			        OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION	| DACL_SECURITY_INFORMATION,
				sd, sizeof (sd_buf), &bytes_needed))
    {
      printf ( "error getting handle SD (%lu)\n", GetLastError ());
      goto out;
    }

  MapGenericMask (&access, &access_mapping);

  if (!AccessCheck (sd, from_process_token, access, &access_mapping,
  		    &ps, &ps_len, &access, &status))
    {
      printf ( "error checking access rights (%lu)\n", GetLastError ());
      goto out;
    }

  if (!status)
    {
      printf ( "access to object denied\n");
      goto out;
    }

  if (!DuplicateHandle (from_process, from_handle,
	                to_process, to_handle_ptr,
                        access, bInheritHandle, 0))
    {
      printf ( "error getting handle to client (%lu)\n", GetLastError ());
      goto out;
    }

  ret_val = 0;
                    
out:
  if (local_handle && from_process != GetCurrentProcess ())
    CloseHandle (local_handle);

  return (ret_val);
}

int
check_and_dup_handle (HANDLE from_process, HANDLE to_process,
                      HANDLE from_process_token,
                      DWORD access,
                      HANDLE from_handle,
                      HANDLE* to_handle_ptr)
{
  return check_and_dup_handle(from_process,to_process,from_process_token,access,from_handle,to_handle_ptr,FALSE);
}

void
client_request::serve (transport_layer_base *conn, class process_cache *cache)
{
  printf ("*****************************************\n"
	  "A call to the base client_request class has occured\n"
	  "This indicates a mismatch in a virtual function definition somewhere\n");
  exit (1);
}

void
client_request_attach_tty::serve(transport_layer_base *conn, class process_cache *cache)
{
  HANDLE from_process_handle = NULL;
  HANDLE to_process_handle = NULL;
  HANDLE token_handle = NULL;
  DWORD rc;

  if (header.cb != sizeof (req))
    {
      header.error_code = EINVAL;
      return;
    }

#if DEBUG
  printf ("%ld:(%p,%p) -> %ld\n", req.master_pid,
  	  			req.from_master, req.to_master,
				req.pid);
#endif

  from_process_handle = OpenProcess (PROCESS_DUP_HANDLE, FALSE, req.master_pid);
  to_process_handle = OpenProcess (PROCESS_DUP_HANDLE, FALSE, req.pid);
  if (!from_process_handle || !to_process_handle)
    {
      printf ("error opening process (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      goto out;
    }

  transport->impersonate_client ();
  
  rc = OpenThreadToken (GetCurrentThread (),
  			TOKEN_QUERY,
                        TRUE,
			&token_handle);

  transport->revert_to_self ();

  if (!rc)
    {
      printf ("error opening thread token (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      goto out;
    }

  if (check_and_dup_handle (from_process_handle, to_process_handle,
                            token_handle,
                            GENERIC_READ,
                            req.from_master,
                            &req.from_master) != 0)
    {
      printf ("error duplicating from_master handle (%lu)\n", GetLastError ());
      header.error_code = EACCES;
      goto out;
    }

  if (req.to_master)
    {
      if (check_and_dup_handle (from_process_handle, to_process_handle,
				token_handle,
				GENERIC_WRITE,
				req.to_master,
				&req.to_master) != 0)
	{
	  printf ("error duplicating to_master handle (%lu)\n", GetLastError ());
	  header.error_code = EACCES;
	  goto out;
	}
    }

#if DEBUG
  printf ("%ld -> %ld(%p,%p)\n", req.master_pid, req.pid,
  	  			req.from_master, req.to_master);
#endif

  header.error_code = 0;

out:
  if (from_process_handle)
    CloseHandle (from_process_handle);
  if (to_process_handle)
    CloseHandle (to_process_handle);
  if (token_handle)
    CloseHandle (token_handle);
}

void
client_request_get_version::serve(transport_layer_base *conn, class process_cache *cache)
{
  if (header.cb != sizeof (version))
    {
      header.error_code = EINVAL;
      return;
    }
  header.error_code = 0;
  version.major = CYGWIN_SERVER_VERSION_MAJOR;
  version.api   = CYGWIN_SERVER_VERSION_API;
  version.minor = CYGWIN_SERVER_VERSION_MINOR;
  version.patch = CYGWIN_SERVER_VERSION_PATCH;
}

class server_request : public queue_request
{
  public:
  server_request (transport_layer_base *newconn, class process_cache *newcache);
  virtual void process ();
  private:
    char request_buffer [MAX_REQUEST_SIZE];
    transport_layer_base *conn;
    class process_cache *cache;
};

class server_process_param : public queue_process_param
{
  public:
    transport_layer_base *transport;
    server_process_param () : queue_process_param (false) {};
};

class server_request_queue : public threaded_queue
{
  public:
    class process_cache *cache;
    void process_requests (transport_layer_base *transport);
    virtual void add (transport_layer_base *conn);
};
class server_request_queue request_queue;

static DWORD WINAPI
request_loop (LPVOID LpParam)
{
  class server_process_param *params = (server_process_param *) LpParam;
  class server_request_queue *queue = (server_request_queue *) params->queue;
  class transport_layer_base * transport = params->transport;
  while (queue->active)
  {
    transport_layer_base * new_conn = transport->accept ();
    /* FIXME: this is a little ugly. What we really want is to wait on two objects:
     * one for the pipe/socket, and one for being told to shutdown. Otherwise
     * this will stay a problem (we won't actually shutdown until the request
     * _AFTER_ the shutdown request. And sending ourselves a request is ugly
     */
     if (new_conn && queue->active)
         queue->add (new_conn);
  }
  return 0;
}

/* TODO: check we are not being asked to service a already serviced transport */
void
server_request_queue::process_requests (transport_layer_base *transport)
{
  class server_process_param *params = new server_process_param;
  params->transport = transport;
  threaded_queue::process_requests (params, request_loop);
}

void
client_request_shutdown::serve (transport_layer_base *conn, class process_cache *cache)
{
  /* FIXME: link upwards, and then this becomes a trivial method call to
   * only shutdown _this queue_
   */
  /* tell the main thread to shutdown */
  request_queue.active=false;
}

server_request::server_request (transport_layer_base *newconn, class process_cache *newcache)
{
  conn = newconn;
  cache = newcache;
}

void
server_request::process ()
{
  ssize_t bytes_read, bytes_written;
  struct request_header* req_ptr = (struct request_header*) &request_buffer;
  client_request *req = NULL;
  debug_printf ("about to read\n");

  bytes_read = conn->read (request_buffer, sizeof (struct request_header));
  if (bytes_read != sizeof (struct request_header))
    {
      printf ("error reading from connection (%lu)\n", GetLastError ());
      goto out;
    }
  debug_printf ("got header (%ld)\n", bytes_read);

  switch (req_ptr->req_id)
    {
    case CYGSERVER_REQUEST_GET_VERSION:
      req = new client_request_get_version (); break;
    case CYGSERVER_REQUEST_ATTACH_TTY:
      req = new client_request_attach_tty (); break;
    case CYGSERVER_REQUEST_SHUTDOWN:
      req = new client_request_shutdown (); break;
    case CYGSERVER_REQUEST_SHM_GET:
     req = new client_request_shm_get (); break;
    default:
      req = new client_request (CYGSERVER_REQUEST_INVALID, 0);
      req->header.error_code = ENOSYS;
      debug_printf ("Bad client request - returning ENOSYS\n");
    }

  if (req->header.cb != req_ptr->cb)
    {
      debug_printf ("Mismatch in request buffer sizes\n");
      goto out;
    }

  if (req->header.cb)
    {

      bytes_read = conn->read (req->buffer, req->header.cb);
      if (bytes_read != req->header.cb)
        {
          debug_printf ("error reading from connection (%lu)\n", GetLastError ());
          goto out;
        }
      debug_printf ("got body (%ld)\n",bytes_read);
    }

  /* this is not allowed to fail. We must return ENOSYS at a minimum to the client */
  req->serve (conn, cache);

  if ((bytes_written = conn->write ((char *)&req->header, sizeof (req->header)))
    != sizeof(req->header) || (req->header.cb &&
    (bytes_written = conn->write (req->buffer, req->header.cb)) != req->header.cb))
    {
      req->header.error_code = -1;
      printf ("error writing to connection (%lu)\n", GetLastError ());
      goto out;
    }

  debug_printf("Sent reply, size (%ld)\n",bytes_written);
  printf (".");

out:
  conn->close ();
  delete conn;
  if (req)
    delete (req);
}

void
server_request_queue::add (transport_layer_base *conn)
{
  /* safe to not "Try" because workers don't hog this, they wait on the event
   */
  /* every derived ::add must enter the section! */
  EnterCriticalSection (&queuelock);
  if (!running)
    {
      conn->close ();
      delete conn;
      LeaveCriticalSection (&queuelock);
      return;
    }
  queue_request * listrequest = new server_request (conn, cache);
  threaded_queue::add (listrequest);
  LeaveCriticalSection (&queuelock);
}

void
handle_signal (int signal)
{
  /* any signal makes us die :} */
  /* FIXME: link upwards, and then this becomes a trivial method call to
   * only shutdown _this queue_
   */
  /* tell the main thread to shutdown */
  request_queue.active=false;
}

struct option longopts[] = {
  {"shutdown", no_argument, NULL, 's'},
  {0, no_argument, NULL, 0}
};

char opts[] = "s";

int
main (int argc, char **argv)
{
  int shutdown=0;
  char i;

  while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (i)
      {
      case 's':
        shutdown = 1;
        break;
      default:
        break;
       /*NOTREACHED*/
      }

  wincap.init();
  if (wincap.has_security ())
    setup_privileges ();
  transport = create_server_transport ();

  if (shutdown)
    {
      if (!transport->connect())
	{
	  printf ("couldn't establish connection with server\n");
	  exit (1);
	}
      client_request_shutdown *request =
	new client_request_shutdown ();
      request->send (transport);
      transport->close();
      delete transport;
      delete request;
      exit(0);
    }

  char version[200];
  /* Cygwin dll release */
  snprintf (version, 200, "%d.%d.%d(%d.%d/%d/%d)-(%d.%d.%d.%d) %s",
                 cygwin_version.dll_major / 1000,
                 cygwin_version.dll_major % 1000,
                 cygwin_version.dll_minor,
                 cygwin_version.api_major,
                 cygwin_version.api_minor,
                 cygwin_version.shared_data,
		 CYGWIN_SERVER_VERSION_MAJOR,
		 CYGWIN_SERVER_VERSION_API,
		 CYGWIN_SERVER_VERSION_MINOR,
		 CYGWIN_SERVER_VERSION_PATCH,
                 cygwin_version.mount_registry,
		 cygwin_version.dll_build_date);
  setbuf (stdout, NULL);
  printf ("daemon version %s starting up", version);
  if (signal (SIGQUIT, handle_signal) == SIG_ERR)
    {
      printf ("\ncould not install signal handler (%d)- aborting startup\n", errno);
      exit (1);
    }
  printf (".");
  transport->listen ();
  printf (".");
  class process_cache cache (2);
  request_queue.initial_workers = 10;
  request_queue.cache = &cache;
  request_queue.create_workers ();
  printf (".");
  request_queue.process_requests (transport);
  printf (".");
  cache.create_workers ();
  printf (".");
  cache.process_requests ();
  printf (".complete\n");
  /* TODO: wait on multiple objects - the thread handle for each request loop +
   * all the process handles. This should be done by querying the request_queue and
   * the process cache for all their handles, and then waiting for (say) 30 seconds.
   * after that we recreate the list of handles to wait on, and wait again.
   * the point of all this abstraction is that we can trivially server both sockets
   * and pipes simply by making a new transport, and then calling
   * request_queue.process_requests (transport2);
   */
  /* WaitForMultipleObjects abort && request_queue && process_queue && signal
     -- if signal event then retrigger it
   */
  while (1 && request_queue.active) 
    {
      sleep (1);
    }
  printf ("\nShutdown request recieved - new requests will be denied\n");
  request_queue.cleanup ();
  printf ("All pending requests processed\n");
  transport->close ();
  printf ("No longer accepting requests - cygwin will operate in daemonless mode\n");
  cache.cleanup ();
  printf ("All outstanding process-cache activities completed\n");
  printf ("daemon shutdown\n");
}
