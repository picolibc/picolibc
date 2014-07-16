/* transport_pipes.cc

   Copyright 2001, 2002, 2003, 2004, 2009, 2012, 2014 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#include <ntdef.h>
#else
#include "winsup.h"
#endif

#include <sys/types.h>

#include <assert.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <wchar.h>
#include <sys/cygwin.h>

#include "cygerrno.h"
#include "transport.h"
#include "transport_pipes.h"

#ifndef __INSIDE_CYGWIN__
#include "cygserver.h"
#include "cygserver_ipc.h"
#else
#include "security.h"
#endif

#ifdef __INSIDE_CYGWIN__
#define SET_ERRNO(err)	set_errno (err)
#else
#define SET_ERRNO(err)	errno = (err)
#endif

enum
  {
    MAX_WAIT_NAMED_PIPE_RETRY = 64,
    WAIT_NAMED_PIPE_TIMEOUT = 10 // milliseconds
  };

#ifndef __INSIDE_CYGWIN__

transport_layer_pipes::transport_layer_pipes (const HANDLE hPipe)
  : _hPipe (hPipe),
    _is_accepted_endpoint (true),
    _is_listening_endpoint (false)
{
  assert (_hPipe);
  assert (_hPipe != INVALID_HANDLE_VALUE);
  _pipe_name[0] = L'\0';
}

#endif /* !__INSIDE_CYGWIN__ */

transport_layer_pipes::transport_layer_pipes ()
  : _hPipe (NULL),
    _is_accepted_endpoint (false),
    _is_listening_endpoint (false)
{
  wchar_t cyg_instkey[18];

  wchar_t *p = wcpcpy (_pipe_name, PIPE_NAME_PREFIX);
  if (!cygwin_internal (CW_GET_INSTKEY, cyg_instkey))
    wcpcpy (wcpcpy (p, cyg_instkey), PIPE_NAME_SUFFIX);
}

transport_layer_pipes::~transport_layer_pipes ()
{
  close ();
}

#ifndef __INSIDE_CYGWIN__

static HANDLE listen_pipe;
static HANDLE connect_pipe;

int
transport_layer_pipes::listen ()
{
  assert (!_hPipe);
  assert (!_is_accepted_endpoint);
  assert (!_is_listening_endpoint);

  _is_listening_endpoint = true;

  debug ("Try to create named pipe: %ls", _pipe_name);

  /* We have to create the first instance of the listening pipe here, and
     we also have to create at least one instance of the client side to avoid
     a race condition.
     See https://cygwin.com/ml/cygwin/2012-11/threads.html#00144 */
  listen_pipe =
    CreateNamedPipeW (_pipe_name,
		      PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
		      PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
		      0, 0, 1000, &sec_all_nih);
  if (listen_pipe != INVALID_HANDLE_VALUE)
    {
      connect_pipe = CreateFileW (_pipe_name, GENERIC_READ | GENERIC_WRITE, 0,
				  &sec_all_nih, OPEN_EXISTING, 0, NULL);
      if (connect_pipe == INVALID_HANDLE_VALUE)
	{
	  CloseHandle (listen_pipe);
	  listen_pipe = INVALID_HANDLE_VALUE;
	}
    }

  if (listen_pipe == INVALID_HANDLE_VALUE)
    {
      system_printf ("failed to create named pipe: "
		     "is the daemon already running?");
      return -1;
    }

  return 0;
}

class transport_layer_pipes *
transport_layer_pipes::accept (bool *const recoverable)
{
  assert (!_hPipe);
  assert (!_is_accepted_endpoint);
  assert (_is_listening_endpoint);

  debug ("Try to create named pipe instance: %ls", _pipe_name);

  const HANDLE accept_pipe =
    CreateNamedPipeW (_pipe_name, PIPE_ACCESS_DUPLEX,
		      PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
		      0, 0, 1000, &sec_all_nih);

  if (accept_pipe == INVALID_HANDLE_VALUE)
    {
      debug_printf ("error creating pipe (%u).", GetLastError ());
      *recoverable = true;	// FIXME: case analysis?
      return NULL;
    }

  if (!ConnectNamedPipe (accept_pipe, NULL)
      && GetLastError () != ERROR_PIPE_CONNECTED)
    {
      debug_printf ("error connecting to pipe (%u)", GetLastError ());
      (void) CloseHandle (accept_pipe);
      *recoverable = true;	// FIXME: case analysis?
      return NULL;
    }

  return new transport_layer_pipes (accept_pipe);
}

#endif /* !__INSIDE_CYGWIN__ */

void
transport_layer_pipes::close ()
{
  // verbose: debug_printf ("closing pipe %p", _hPipe);

  if (_hPipe)
    {
      assert (_hPipe != INVALID_HANDLE_VALUE);

#ifndef __INSIDE_CYGWIN__

      if (_is_accepted_endpoint)
	{
	  (void) FlushFileBuffers (_hPipe); // Blocks until client reads.
	  (void) DisconnectNamedPipe (_hPipe);
	  (void) CloseHandle (_hPipe);
	}
      else
	(void) CloseHandle (_hPipe);

#else /* __INSIDE_CYGWIN__ */

      assert (!_is_accepted_endpoint);
      (void) ForceCloseHandle (_hPipe);

#endif /* __INSIDE_CYGWIN__ */

      _hPipe = NULL;
    }
}

ssize_t
transport_layer_pipes::read (void *const buf, const size_t len)
{
  // verbose: debug_printf ("reading from pipe %p", _hPipe);

  assert (_hPipe);
  assert (_hPipe != INVALID_HANDLE_VALUE);
  assert (!_is_listening_endpoint);

  DWORD count;
  if (!ReadFile (_hPipe, buf, len, &count, NULL))
    {
      debug_printf ("error reading from pipe (%u)", GetLastError ());
      SET_ERRNO (EINVAL);	// FIXME?
      return -1;
    }

  return count;
}

ssize_t
transport_layer_pipes::write (void *const buf, const size_t len)
{
  // verbose: debug_printf ("writing to pipe %p", _hPipe);

  assert (_hPipe);
  assert (_hPipe != INVALID_HANDLE_VALUE);
  assert (!_is_listening_endpoint);

  DWORD count;
  if (!WriteFile (_hPipe, buf, len, &count, NULL))
    {
      debug_printf ("error writing to pipe, error = %u", GetLastError ());
      SET_ERRNO (EINVAL);	// FIXME?
      return -1;
    }

  return count;
}

/*
 * This routine holds a static variable, assume_cygserver, that is set
 * if the transport has good reason to think that cygserver is
 * running, i.e. if if successfully connected to it with the previous
 * attempt.  If this is set, the code tries a lot harder to get a
 * connection, making the assumption that any failures are just
 * congestion and overloading problems.
 */

int
transport_layer_pipes::connect ()
{
  assert (!_hPipe);
  assert (!_is_accepted_endpoint);
  assert (!_is_listening_endpoint);

  static bool assume_cygserver = false;

  BOOL rc = TRUE;
  int retries = 0;

  debug_printf ("Try to connect to named pipe: %W", _pipe_name);
  while (rc)
    {
      _hPipe = CreateFileW (_pipe_name,
			    GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    &sec_all_nih,
			    OPEN_EXISTING,
			    SECURITY_IMPERSONATION,
			    NULL);

      if (_hPipe != INVALID_HANDLE_VALUE)
	{
	  assert (_hPipe);
#ifdef __INSIDE_CYGWIN__
	  ProtectHandle (_hPipe);
#endif
	  assume_cygserver = true;
	  return 0;
	}

      _hPipe = NULL;

      if (!assume_cygserver && GetLastError () != ERROR_PIPE_BUSY)
	{
	  debug_printf ("Error opening the pipe (%u)", GetLastError ());
	  return -1;
	}

      /* Note: `If no instances of the specified named pipe exist, the
       * WaitNamedPipe function returns immediately, regardless of the
       * time-out value.'  Thus the explicit Sleep if the call fails
       * with ERROR_FILE_NOT_FOUND.
       */
      while (retries != MAX_WAIT_NAMED_PIPE_RETRY
	     && !(rc = WaitNamedPipeW (_pipe_name, WAIT_NAMED_PIPE_TIMEOUT)))
	{
	  if (GetLastError () == ERROR_FILE_NOT_FOUND)
	    Sleep (0);		// Give the server a chance.

	  retries += 1;
	}
    }

  assert (retries == MAX_WAIT_NAMED_PIPE_RETRY);

  system_printf ("lost connection to cygserver, error = %u",
		 GetLastError ());

  assume_cygserver = false;

  return -1;
}

#ifndef __INSIDE_CYGWIN__

bool
transport_layer_pipes::impersonate_client ()
{
  assert (_hPipe);
  assert (_hPipe != INVALID_HANDLE_VALUE);
  assert (_is_accepted_endpoint);

  if (_hPipe && !ImpersonateNamedPipeClient (_hPipe))
    {
      debug_printf ("Failed to Impersonate client, (%u)", GetLastError ());
      return false;
    }

  return true;
}

bool
transport_layer_pipes::revert_to_self ()
{
  assert (_is_accepted_endpoint);

  if (!RevertToSelf ())
    {
      debug_printf ("Failed to RevertToSelf, (%u)", GetLastError ());
      return false;
    }
  return true;
}

#endif /* !__INSIDE_CYGWIN__ */
