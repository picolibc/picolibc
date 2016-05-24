/* client.cc

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#else
#include "winsup.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "cygserver_msg.h"
#include "cygserver_sem.h"
#include "cygserver_shm.h"
#include "cygserver_setpwd.h"
#include "cygserver_pwdgrp.h"

#include "cygserver.h"
#include "transport.h"

int cygserver_running = CYGSERVER_UNKNOWN; // Nb: inherited by children.

client_request_get_version::client_request_get_version ()
  : client_request (CYGSERVER_REQUEST_GET_VERSION, &version, sizeof (version))
{
  msglen (0);			// No parameters for request.

  // verbose: syscall_printf ("created");
}

/*
 * client_request_get_version::check_version ()
 *
 * The major version and API version numbers must match exactly.  An
 * older than expected minor version number is accepted (as long as
 * the first numbers match, that is).
 */

#ifdef __INSIDE_CYGWIN__

bool
client_request_get_version::check_version () const
{
  const bool ok = (version.major == CYGWIN_SERVER_VERSION_MAJOR
		   && version.api == CYGWIN_SERVER_VERSION_API
		   && version.minor <= CYGWIN_SERVER_VERSION_MINOR);

  if (!ok)
    syscall_printf (("incompatible version of cygwin server: "
		     "client version %d.%d.%d.%d, "
		     "server version %d.%d.%d.%d"),
		    CYGWIN_SERVER_VERSION_MAJOR,
		    CYGWIN_SERVER_VERSION_API,
		    CYGWIN_SERVER_VERSION_MINOR,
		    CYGWIN_SERVER_VERSION_PATCH,
		    version.major,
		    version.api,
		    version.minor,
		    version.patch);

  return ok;
}

client_request_attach_tty::client_request_attach_tty (DWORD nmaster_pid,
						      HANDLE nfrom_master,
						      HANDLE nto_master)
  : client_request (CYGSERVER_REQUEST_ATTACH_TTY, &req, sizeof (req))
{
  req.pid = GetCurrentProcessId ();
  req.master_pid = nmaster_pid;
  req.from_master = nfrom_master;
  req.to_master = nto_master;

  syscall_printf (("created: pid = %u, master_pid = %u, "
		   "from_master = %p, to_master = %p"),
		  req.pid, req.master_pid, req.from_master, req.to_master);
}
#endif /* __INSIDE_CYGWIN__ */

/*
 * client_request_attach_tty::send ()
 *
 * Wraps the base method to provide error handling support.  If the
 * reply contains a body but is flagged as an error, close any handles
 * that have been returned by cygserver and then discard the message
 * body, i.e. the client either sees a successful result with handles
 * or an unsuccessful result with no handles.
 */

void
client_request_attach_tty::send (transport_layer_base * const conn)
{
  client_request::send (conn);

  if (msglen () && error_code ())
    {
      if (from_master ())
	CloseHandle (from_master ());
      if (to_master ())
	CloseHandle (to_master ());
      msglen (0);
    }
}

client_request::header_t::header_t (const request_code_t request_code,
				    const size_t len)
  : request_code (request_code)
{
  assert (request_code >= 0 && request_code < CYGSERVER_REQUEST_LAST);
  msglen = len;
}

// FIXME: also check write and read result for -1.

void
client_request::send (transport_layer_base * const conn)
{
  assert (conn);
  assert (!(msglen () && !_buf)); // i.e., msglen () implies _buf
  assert (msglen () <= _buflen);

  {
    const ssize_t count = conn->write (&_header, sizeof (_header));

    if (count != sizeof (_header))
      {
	assert (errno);
	error_code (errno);
	syscall_printf (("request header write failure: "
			 "only %ld bytes sent of %lu, "
			 "error = %d(%u)"),
			count, sizeof (_header),
			errno, GetLastError ());
	return;
      }
  }

  if (msglen ())
    {
      const ssize_t count = conn->write (_buf, msglen ());

      if (count == -1 || (size_t) count != msglen ())
	{
	  assert (errno);
	  error_code (errno);
	  syscall_printf (("request body write failure: "
			   "only %ld bytes sent of %lu, "
			   "error = %d(%u)"),
			  count, msglen (),
			  errno, GetLastError ());
	  return;
	}
    }

  {
    const ssize_t count = conn->read (&_header, sizeof (_header));

    if (count != sizeof (_header))
      {
	assert (errno);
	error_code (errno);
	syscall_printf (("reply header read failure: "
			 "only %ld bytes received of %lu, "
			 "error = %d(%u)"),
			count, sizeof (_header),
			errno, GetLastError ());
	return;
      }
  }

  if (msglen () && !_buf)
    {
      system_printf ("no client buffer for reply body: %lu bytes needed",
		     msglen ());
      error_code (EINVAL);
      return;
    }

  if (msglen () > _buflen)
    {
      system_printf (("client buffer too small for reply body: "
		      "have %lu bytes and need %lu"),
		     _buflen, msglen ());
      error_code (EINVAL);
      return;
    }

  if (msglen ())
    {
      const ssize_t count = conn->read (_buf, msglen ());

      if (count == -1 || (size_t) count != msglen ())
	{
	  assert (errno);
	  error_code (errno);
	  syscall_printf (("reply body read failure: "
			   "only %ld bytes received of %lu, "
			   "error = %d(%u)"),
			  count, msglen (),
			  errno, GetLastError ());
	  return;
	}
    }
}

#ifdef __OUTSIDE_CYGWIN__

client_request_attach_tty::client_request_attach_tty ()
  : client_request (CYGSERVER_REQUEST_ATTACH_TTY, &req, sizeof (req))
{
}

/*
 * client_request::handle_request ()
 *
 * A server-side method.
 *
 * This is a factory method for the client_request subclasses.  It
 * reads the incoming request header and, based on its request code,
 * creates an instance of the appropriate class.
 *
 * FIXME: If the incoming packet is malformed, the server drops it on
 * the floor.  Should it try and generate some sort of reply for the
 * client?  As it is, the client will simply get a broken connection.
 *
 * FIXME: also check write and read result for -1.
 */

/* static */ void
client_request::handle_request (transport_layer_base *const conn,
				process_cache *const cache)
{
  // verbose: debug_printf ("about to read");

  header_t header;

  {
    const ssize_t count = conn->read (&header, sizeof (header));

    if (count != sizeof (header))
      {
	syscall_printf (("request header read failure: "
			 "only %ld bytes received of %lu, "
			 "error = %d(%u)"),
			count, sizeof (header),
			errno, GetLastError ());
	return;
      }
  }

  client_request *req = NULL;

  switch (header.request_code)
    {
    case CYGSERVER_REQUEST_GET_VERSION:
      req = new client_request_get_version;
      break;
    case CYGSERVER_REQUEST_SHUTDOWN:
      req = new client_request_shutdown;
      break;
    case CYGSERVER_REQUEST_ATTACH_TTY:
      req = new client_request_attach_tty;
      break;
    case CYGSERVER_REQUEST_MSG:
      req = new client_request_msg;
      break;
    case CYGSERVER_REQUEST_SEM:
      req = new client_request_sem;
      break;
    case CYGSERVER_REQUEST_SHM:
      req = new client_request_shm;
      break;
    case CYGSERVER_REQUEST_SETPWD:
      req = new client_request_setpwd;
      break;
    case CYGSERVER_REQUEST_PWDGRP:
      req = new client_request_pwdgrp;
      break;
    default:
      syscall_printf ("unknown request code %d received: request ignored",
		      header.request_code);
      return;
    }

  assert (req);

  req->msglen (header.msglen);
  req->handle (conn, cache);

  delete req;
}

/*
 * client_request::handle ()
 *
 * A server-side method.
 *
 * At this point, the header of an incoming request has been read and
 * an appropriate client_request object constructed.  This method has
 * to read the request body into its buffer, if there is such a body,
 * then perform the request and send back the results to the client.
 *
 * FIXME: If the incoming packet is malformed, the server drops it on
 * the floor.  Should it try and generate some sort of reply for the
 * client?  As it is, the client will simply get a broken connection.
 *
 * FIXME: also check write and read result for -1.
 */

void
client_request::handle (transport_layer_base *const conn,
			process_cache *const cache)
{
  if (msglen () && !_buf)
    {
      system_printf ("no buffer for request body: %lu bytes needed",
		     msglen ());
      error_code (EINVAL);
      return;
    }

  if (msglen () > _buflen)
    {
      system_printf (("buffer too small for request body: "
		      "have %lu bytes and need %lu"),
		     _buflen, msglen ());
      error_code (EINVAL);
      return;
    }

  if (msglen ())
    {
      const ssize_t count = conn->read (_buf, msglen ());

      if (count == -1 || (size_t) count != msglen ())
	{
	  assert (errno);
	  error_code (errno);
	  syscall_printf (("request body read failure: "
			   "only %ld bytes received of %lu, "
			   "error = %d(%u)"),
			  count, msglen (),
			  errno, GetLastError ());
	  return;
	}
    }

  error_code (0);		// Overwrites the _header.request_code field.

  /*
   * This is not allowed to fail. We must return ENOSYS at a minimum
   * to the client.
   */
  serve (conn, cache);

  {
    const ssize_t count = conn->write (&_header, sizeof (_header));

    if (count != sizeof (_header))
      {
	assert (errno);
	error_code (errno);
	syscall_printf (("reply header write failure: "
			 "only %ld bytes sent of %lu, "
			 "error = %d(%u)"),
			count, sizeof (_header),
			errno, GetLastError ());
	return;
      }
  }

  if (msglen ())
    {
      const ssize_t count = conn->write (_buf, msglen ());

      if (count == -1 || (size_t) count != msglen ())
	{
	  assert (errno);
	  error_code (errno);
	  syscall_printf (("reply body write failure: "
			   "only %ld bytes sent of %lu, "
			   "error = %d(%u)"),
			  count, msglen (),
			  errno, GetLastError ());
	  return;
	}
    }
}

/* The server side implementation of make_request.  Very simple. */
int
client_request::make_request ()
{
  transport_layer_base *const transport = create_server_transport ();
  assert (transport);
  if (transport->connect () == -1)
    {
      if (errno)
	error_code (errno);
      else
	error_code (ENOSYS);
      delete transport;
      return -1;
    }
  send (transport);
  delete transport;
  return 0;
}
#endif /* __OUTSIDE_CYGWIN__ */

client_request::client_request (request_code_t const id,
				void * const buf,
				size_t const buflen)
  : _header (id, buflen),
    _buf (buf),
    _buflen (buflen)
{
  assert ((!_buf && !_buflen) || (_buf && _buflen));
}

client_request::~client_request ()
{}

#ifdef __INSIDE_CYGWIN__
int
client_request::make_request ()
{
  assert (cygserver_running == CYGSERVER_UNKNOWN	\
	  || cygserver_running == CYGSERVER_OK		\
	  || cygserver_running == CYGSERVER_UNAVAIL);

  if (cygserver_running == CYGSERVER_UNKNOWN)
    cygserver_init ();

  assert (cygserver_running == CYGSERVER_OK		\
	  || cygserver_running == CYGSERVER_UNAVAIL);

  /* Don't retry every request if the server's not there */
  if (cygserver_running == CYGSERVER_UNAVAIL)
    {
      syscall_printf ("cygserver un-available");
      error_code (ENOSYS);
      return -1;
    }

  transport_layer_base *const transport = create_server_transport ();

  assert (transport);

  if (transport->connect () == -1)
    {
      if (errno)
	error_code (errno);
      else
	error_code (ENOSYS);
      delete transport;
      return -1;
    }

  // verbose: debug_printf ("connected to server %p", transport);

  send (transport);

  delete transport;

  return 0;
}

bool
check_cygserver_available ()
{
  assert (cygserver_running == CYGSERVER_UNKNOWN	\
	  || cygserver_running == CYGSERVER_UNAVAIL);

  cygserver_running = CYGSERVER_OK; // For make_request ().

  client_request_get_version req;

  /* This indicates that we failed to connect to cygserver at all but
   * that's fine as cygwin doesn't need it to be running.
   */
  if (req.make_request () == -1)
    return false;

  /* We connected to the server but something went wrong after that
   * (in sending the message, in cygserver itself, or in receiving the
   * reply).
   */
  if (req.error_code ())
    {
      syscall_printf ("failure in cygserver version request: %d",
		      req.error_code ());
      syscall_printf ("process will continue without cygserver support");
      return false;
    }

  return req.check_version ();
}

void
cygserver_init ()
{
  assert (cygserver_running == CYGSERVER_UNKNOWN	\
	  || cygserver_running == CYGSERVER_OK		\
	  || cygserver_running == CYGSERVER_UNAVAIL);

  if (cygserver_running == CYGSERVER_OK)
    return;

  if (!check_cygserver_available ())
    cygserver_running = CYGSERVER_UNAVAIL;
}
#endif /* __INSIDE_CYGWIN__ */
