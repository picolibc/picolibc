/* cygserver.cc

   Copyright 2001, 2002 Red Hat Inc.

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "woutsup.h"

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cygerrno.h"
#include "cygwin_version.h"

#include "cygwin/cygserver.h"
#include "cygwin/cygserver_process.h"
#include "cygwin/cygserver_transport.h"

// Version string.
static const char version[] = "$Revision$";

/*
 * Support function for the XXX_printf() macros in "woutsup.h".
 * Copied verbatim from "strace.cc".
 */
static int
getfunc (char *in_dst, const char *func)
{
  const char *p;
  const char *pe;
  char *dst = in_dst;
  for (p = func; (pe = strchr (p, '(')); p = pe + 1)
    if (isalnum ((int)pe[-1]) || pe[-1] == '_')
      break;
    else if (isspace((int)pe[-1]))
      {
	pe--;
	break;
      }
  if (!pe)
    pe = strchr (func, '\0');
  for (p = pe; p > func; p--)
    if (p != pe && *p == ' ')
      {
	p++;
	break;
      }
  if (*p == '*')
    p++;
  while (p < pe)
    *dst++ = *p++;

  *dst++ = ':';
  *dst++ = ' ';
  *dst = '\0';

  return dst - in_dst;
}

/*
 * Support function for the XXX_printf() macros in "woutsup.h".
 */
extern "C" void
__cygserver__printf (const char *const function, const char *const fmt, ...)
{
  const DWORD lasterror = GetLastError ();
  const int lasterrno = errno;

  va_list ap;

  char *const buf = (char *) alloca(BUFSIZ);

  assert (buf);

  int len = 0;

  if (function)
    len += getfunc(buf, function);

  va_start (ap, fmt);
  len += vsnprintf (buf + len, BUFSIZ - len, fmt, ap);
  va_end (ap);

  len += snprintf (buf + len, BUFSIZ - len, "\n");

  const int actual = (len > BUFSIZ ? BUFSIZ : len);

  write (2, buf, actual);

  errno = lasterrno;
  SetLastError (lasterror);

  return;
}

#ifdef DEBUGGING

int __stdcall
__set_errno (const char *func, int ln, int val)
{
  debug_printf ("%s:%d val %d", func, ln, val);
  return _impure_ptr->_errno = val;
}

#endif /* DEBUGGING */

GENERIC_MAPPING access_mapping;

static BOOL
setup_privileges ()
{
  BOOL rc, ret_val;
  HANDLE hToken = NULL;
  TOKEN_PRIVILEGES sPrivileges;

  rc = OpenProcessToken (GetCurrentProcess() , TOKEN_ALL_ACCESS , &hToken) ;
  if (!rc)
    {
      system_printf ("error opening process token (%lu)", GetLastError ());
      ret_val = FALSE;
      goto out;
    }
  rc = LookupPrivilegeValue (NULL, SE_DEBUG_NAME, &sPrivileges.Privileges[0].Luid);
  if (!rc)
    {
      system_printf ("error getting privilege luid (%lu)", GetLastError ());
      ret_val = FALSE;
      goto out;
    }
  sPrivileges.PrivilegeCount = 1 ;
  sPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
  rc = AdjustTokenPrivileges (hToken, FALSE, &sPrivileges, 0, NULL, NULL) ;
  if (!rc)
    {
      system_printf ("error adjusting privilege level. (%lu)",
		     GetLastError ());
      ret_val = FALSE;
      goto out;
    }

  access_mapping.GenericRead = FILE_READ_DATA;
  access_mapping.GenericWrite = FILE_WRITE_DATA;
  access_mapping.GenericExecute = 0;
  access_mapping.GenericAll = FILE_READ_DATA | FILE_WRITE_DATA;

  ret_val = TRUE;

out:
  CloseHandle (hToken);
  return ret_val;
}

int
check_and_dup_handle (HANDLE from_process, HANDLE to_process,
		      HANDLE from_process_token,
		      DWORD access,
		      HANDLE from_handle,
		      HANDLE *to_handle_ptr, BOOL bInheritHandle = FALSE)
{
  HANDLE local_handle = NULL;
  int ret_val = EACCES;

  if (from_process != GetCurrentProcess ())
    {
      if (!DuplicateHandle (from_process, from_handle,
			    GetCurrentProcess (), &local_handle,
			    0, bInheritHandle,
			    DUPLICATE_SAME_ACCESS))
	{
	  system_printf ("error getting handle(%u) to server (%lu)",
			 (unsigned int)from_handle, GetLastError ());
	  goto out;
	}
    } else
      local_handle = from_handle;

  if (!wincap.has_security ())
    assert (!from_process_token);
  else
    {
      char sd_buf [1024];
      PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR) &sd_buf;
      DWORD bytes_needed;
      PRIVILEGE_SET ps;
      DWORD ps_len = sizeof (ps);
      BOOL status;

      if (!GetKernelObjectSecurity (local_handle,
				    (OWNER_SECURITY_INFORMATION
				     | GROUP_SECURITY_INFORMATION
				     | DACL_SECURITY_INFORMATION),
				    sd, sizeof (sd_buf), &bytes_needed))
	{
	  system_printf ("error getting handle SD (%lu)", GetLastError ());
	  goto out;
	}

      MapGenericMask (&access, &access_mapping);

      if (!AccessCheck (sd, from_process_token, access, &access_mapping,
			&ps, &ps_len, &access, &status))
	{
	  system_printf ("error checking access rights (%lu)",
			 GetLastError ());
	  goto out;
	}

      if (!status)
	{
	  system_printf ("access to object denied");
	  goto out;
	}
    }

  if (!DuplicateHandle (from_process, from_handle,
			to_process, to_handle_ptr,
			access, bInheritHandle, 0))
    {
      system_printf ("error getting handle to client (%lu)", GetLastError ());
      goto out;
    }

  // verbose: debug_printf ("Duplicated %p to %p", from_handle, *to_handle_ptr);

  ret_val = 0;

 out:
  if (local_handle && from_process != GetCurrentProcess ())
    CloseHandle (local_handle);

  return (ret_val);
}

/*
 * client_request_attach_tty::serve ()
 */

void
client_request_attach_tty::serve (transport_layer_base *const conn,
				  process_cache *)
{
  assert (conn);

  assert (!error_code ());

  if (!wincap.has_security ())
    {
      syscall_printf ("operation only supported on systems with security");
      error_code (EINVAL);
      msglen (0);
      return;
    }

  if (msglen () != sizeof (req))
    {
      syscall_printf ("bad request body length: expecting %lu bytes, got %lu",
		      sizeof (req), msglen ());
      error_code (EINVAL);
      msglen (0);
      return;
    }

  msglen (0);			// Until we fill in some fields.

  // verbose: debug_printf ("pid %ld:(%p,%p) -> pid %ld",
  //			    req.master_pid, req.from_master, req.to_master,
  //			    req.pid);

  // verbose: debug_printf ("opening process %ld", req.master_pid);

  const HANDLE from_process_handle =
    OpenProcess (PROCESS_DUP_HANDLE, FALSE, req.master_pid);

  if (!from_process_handle)
    {
      system_printf ("error opening `from' process, error = %lu",
		     GetLastError ());
      error_code (EACCES);
      return;
    }

  // verbose: debug_printf ("opening process %ld", req.pid);

  const HANDLE to_process_handle =
    OpenProcess (PROCESS_DUP_HANDLE, FALSE, req.pid);

  if (!to_process_handle)
    {
      system_printf ("error opening `to' process, error = %lu",
		     GetLastError ());
      CloseHandle (from_process_handle);
      error_code (EACCES);
      return;
    }

  // verbose: debug_printf ("Impersonating client");
  conn->impersonate_client ();

  HANDLE token_handle = NULL;

  // verbose: debug_printf ("about to open thread token");
  const DWORD rc = OpenThreadToken (GetCurrentThread (),
				    TOKEN_QUERY,
				    TRUE,
				    &token_handle);

  // verbose: debug_printf ("opened thread token, rc=%lu", rc);
  conn->revert_to_self ();

  if (!rc)
    {
      system_printf ("error opening thread token, error = %lu",
		     GetLastError ());
      CloseHandle (from_process_handle);
      CloseHandle (to_process_handle);
      error_code (EACCES);
      return;
    }

  // From this point on, a reply body is returned to the client.

  const HANDLE from_master = req.from_master;
  const HANDLE to_master = req.to_master;

  req.from_master = NULL;
  req.to_master = NULL;

  msglen (sizeof (req));

  if (from_master)
    if (check_and_dup_handle (from_process_handle, to_process_handle,
			      token_handle,
			      GENERIC_READ,
			      from_master,
			      &req.from_master, TRUE) != 0)
      {
	system_printf ("error duplicating from_master handle, error = %lu",
		       GetLastError ());
	error_code (EACCES);
      }

  if (to_master)
    if (check_and_dup_handle (from_process_handle, to_process_handle,
			      token_handle,
			      GENERIC_WRITE,
			      to_master,
			      &req.to_master, TRUE) != 0)
      {
	system_printf ("error duplicating to_master handle, error = %lu",
		       GetLastError ());
	error_code (EACCES);
      }

  CloseHandle (from_process_handle);
  CloseHandle (to_process_handle);
  CloseHandle (token_handle);

  debug_printf ("%lu(%lu, %lu) -> %lu(%lu,%lu)",
		req.master_pid, from_master, to_master,
		req.pid, req.from_master, req.to_master);

  return;
}

void
client_request_get_version::serve(transport_layer_base *, process_cache *)
{
  assert (!error_code ());

  if (msglen ())
    syscall_printf ("unexpected request body ignored: %lu bytes", msglen ());

  msglen (sizeof (version));

  version.major = CYGWIN_SERVER_VERSION_MAJOR;
  version.api   = CYGWIN_SERVER_VERSION_API;
  version.minor = CYGWIN_SERVER_VERSION_MINOR;
  version.patch = CYGWIN_SERVER_VERSION_PATCH;
}

class server_request : public queue_request
{
public:
  server_request (transport_layer_base *const conn, process_cache *const cache)
    : _conn (conn), _cache (cache)
  {}

  virtual ~server_request()
  {
    safe_delete (transport_layer_base, _conn);
  }

  virtual void process ()
  {
    client_request::handle_request (_conn, _cache);
  }

private:
  transport_layer_base *const _conn;
  process_cache *const _cache;
};

class server_submission_loop : public queue_submission_loop
{
public:
  server_submission_loop (threaded_queue *const queue,
			  transport_layer_base *const transport,
			  process_cache *const cache)
    : queue_submission_loop (queue, false),
      _transport (transport),
      _cache (cache)
  {
    assert (_transport);
    assert (_cache);
  }

private:
  transport_layer_base *const _transport;
  process_cache *const _cache;

  virtual void request_loop ();
};

/* FIXME: this is a little ugly.  What we really want is to wait on
 * two objects: one for the pipe/socket, and one for being told to
 * shutdown.  Otherwise this will stay a problem (we won't actually
 * shutdown until the request _AFTER_ the shutdown request.  And
 * sending ourselves a request is ugly
 */
void
server_submission_loop::request_loop ()
{
  while (_running)
    {
      bool recoverable = false;
      transport_layer_base *const conn = _transport->accept (&recoverable);
      if (!conn && !recoverable)
	{
	  system_printf ("fatal error on IPC transport: closing down");
	  return;
	}
      if (conn)
	_queue->add (safe_new (server_request, conn, _cache));
    }
}

client_request_shutdown::client_request_shutdown ()
  : client_request (CYGSERVER_REQUEST_SHUTDOWN)
{
  syscall_printf ("created");
}

static volatile sig_atomic_t shutdown_server = false;

void
client_request_shutdown::serve (transport_layer_base *, process_cache *)
{
  assert (!error_code ());

  if (msglen ())
    syscall_printf ("unexpected request body ignored: %lu bytes", msglen ());

  /* FIXME: link upwards, and then this becomes a trivial method call to
   * only shutdown _this queue_
   */

  shutdown_server = true;

  msglen (0);
}

static void
handle_signal (const int signum)
{
  /* any signal makes us die :} */

  shutdown_server = true;
}

/*
 * print_usage()
 */

static void
print_usage (const char *const pgm)
{
  printf ("Usage: %s [OPTIONS]\n", pgm);
  printf ("  -h, --help       output usage information and exit\n");
  printf ("  -s, --shutdown   shutdown the current instance of the daemon\n");
  printf ("  -v, --version    output version information and exit\n");
}

/*
 * print_version()
 */

static void
print_version (const char *const pgm)
{
  char *vn = NULL;

  const char *const colon = strchr (version, ':');

  if (!colon)
    {
      vn = strdup ("?");
    }
  else
    {
      vn = strdup (colon + 2);	// Skip ": "

      char *const spc = strchr (vn, ' ');

      if (spc)
	*spc = '\0';
    }

  char buf[200];
  snprintf (buf, sizeof (buf), "%d.%d.%d(%d.%d/%d/%d)-(%d.%d.%d.%d) %s",
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

  printf ("%s (cygwin) %s\n", pgm, vn);
  printf ("API version %s\n", buf);
  printf ("Copyright 2001, 2002 Red Hat, Inc.\n");
  printf ("Compiled on %s\n", __DATE__);

  free (vn);
}

/*
 * main()
 */

int
main (const int argc, char *argv[])
{
  const struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"shutdown", no_argument, NULL, 's'},
    {"version", no_argument, NULL, 'v'},
    {0, no_argument, NULL, 0}
  };

  const char opts[] = "hsv";

  bool shutdown = false;

  const char *pgm = NULL;

  if (!(pgm = strrchr (*argv, '\\')) && !(pgm = strrchr (*argv, '/')))
    pgm = *argv;
  else
    pgm++;

  wincap.init();
  if (wincap.has_security ())
    setup_privileges ();

  int opt;

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
	print_usage (pgm);
	return 0;

      case 's':
	shutdown = true;
	break;

      case 'v':
	print_version (pgm);
	return 0;

      case '?':
	fprintf (stderr, "Try `%s --help' for more information.\n", pgm);
	exit (1);
      }

  if (optind != argc)
    {
      fprintf (stderr, "%s: too many arguments\n", pgm);
      exit (1);
    }

  if (shutdown)
    {
      /* Setting `cygserver_running' stops the request code making a
       * version request, which is not much to the point.
       */
      cygserver_running = CYGSERVER_OK;

      client_request_shutdown req;

      if (req.make_request () == -1 || req.error_code ())
	{
	  fprintf (stderr, "%s: shutdown request failed: %s\n",
		   pgm, strerror (errno));
	  exit (1);
	}

      // FIXME: It would be nice to wait here for the daemon to exit.

      return 0;
    }

  if (signal (SIGINT, handle_signal) == SIG_ERR)
    {
      system_printf ("could not install signal handler (%d)- aborting startup",
		     errno);
      exit (1);
    }

  print_version (pgm);
  setbuf (stdout, NULL);
  printf ("daemon starting up");

  threaded_queue request_queue (10);
  printf (".");

  transport_layer_base *const transport = create_server_transport ();
  assert (transport);
  printf (".");

  process_cache cache (2);
  printf (".");

  server_submission_loop submission_loop (&request_queue, transport, &cache);
  printf (".");

  request_queue.add_submission_loop (&submission_loop);
  printf (".");

  transport->listen ();
  printf (".");

  cache.start ();
  printf (".");

  request_queue.start ();
  printf (".");

  printf ("complete\n");

  /* TODO: wait on multiple objects - the thread handle for each
   * request loop + all the process handles. This should be done by
   * querying the request_queue and the process cache for all their
   * handles, and then waiting for (say) 30 seconds.  after that we
   * recreate the list of handles to wait on, and wait again.  the
   * point of all this abstraction is that we can trivially server
   * both sockets and pipes simply by making a new transport, and then
   * calling request_queue.process_requests (transport2);
   */
  /* WaitForMultipleObjects abort && request_queue && process_queue && signal
     -- if signal event then retrigger it
  */
  while (!shutdown_server && request_queue.running () && cache.running ())
    Sleep (1000);

  printf ("\nShutdown request received - new requests will be denied\n");
  request_queue.stop ();
  printf ("All pending requests processed\n");
  safe_delete (transport_layer_base, transport);
  printf ("No longer accepting requests - cygwin will operate in daemonless mode\n");
  cache.stop ();
  printf ("All outstanding process-cache activities completed\n");
  printf ("daemon shutdown\n");

  return 0;
}
