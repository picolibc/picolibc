/* fhandler_fifo.cc - See fhandler.h for a description of the fhandler classes.

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include "miscfuncs.h"

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "cygtls.h"
#include "shared_info.h"

fhandler_fifo::fhandler_fifo ():
  wait_state (fifo_unknown), dummy_client (NULL)
{
  get_overlapped ()->hEvent = NULL;
  need_fork_fixup (true);
}

HANDLE
fhandler_fifo::open_nonserver (const char *npname, unsigned low_flags,
			       LPSECURITY_ATTRIBUTES sa_buf)
{
  DWORD mode = 0;
  if (low_flags == O_RDONLY)
    mode = GENERIC_READ;
  else if (low_flags == O_WRONLY)
    mode = GENERIC_WRITE;
  else
    mode = GENERIC_READ | GENERIC_WRITE;
  while (1)
    {
      HANDLE h = CreateFile (npname, mode, 0, sa_buf, OPEN_EXISTING,
			     FILE_FLAG_OVERLAPPED, NULL);
      if (h != INVALID_HANDLE_VALUE || GetLastError () != ERROR_PIPE_NOT_CONNECTED)
	return h;
      if (&_my_tls != _main_tls)
	low_priority_sleep (0);
      else if (WaitForSingleObject (signal_arrived, 0) == WAIT_OBJECT_0)
	{
	  set_errno (EINTR);
	  return NULL;
	}
    }
}

char *
fhandler_fifo::fifo_name (char *buf)
{
  /* Generate a semi-unique name to associate with this fifo. */
  __small_sprintf (buf, "\\\\.\\pipe\\__cygfifo__%S_%08x_%016X",
		   &installation_key, get_dev (), get_ino ());
  return buf;
}

#define FIFO_PIPE_MODE (PIPE_TYPE_BYTE | PIPE_READMODE_BYTE)
#define FIFO_BUF_SIZE  4096
#define cnp(m, s) CreateNamedPipe(npname, (m), FIFO_PIPE_MODE, \
			       PIPE_UNLIMITED_INSTANCES, (s), (s), \
			       NMPWAIT_WAIT_FOREVER, sa_buf)


int
fhandler_fifo::open (int flags, mode_t)
{
  int res = 1;
  char npname[MAX_PATH];

  fifo_name (npname);
  unsigned low_flags = flags & O_ACCMODE;
  DWORD mode = 0;
  if (low_flags == O_WRONLY)
    mode = PIPE_ACCESS_OUTBOUND;
  else if (low_flags == O_RDONLY || low_flags == O_RDWR)
    mode = PIPE_ACCESS_DUPLEX;
  else
    {
      set_errno (EINVAL);
      res = 0;
    }

  if (res)
    {
      char char_sa_buf[1024];
      LPSECURITY_ATTRIBUTES sa_buf =
	sec_user ((PSECURITY_ATTRIBUTES) char_sa_buf, cygheap->user.sid());
      bool do_seterrno = true;

      HANDLE h;
      bool nonblocking_write = !!((flags & (O_WRONLY | O_NONBLOCK)) == (O_WRONLY | O_NONBLOCK));
      wait_state = fifo_unknown;
      if (mode != PIPE_ACCESS_OUTBOUND)
	{
	  h = cnp (mode |  FILE_FLAG_OVERLAPPED, FIFO_BUF_SIZE);
	  wait_state = fifo_wait_for_client;
	}
      else
	{
	  h = open_nonserver (npname, low_flags, sa_buf);
	  if (h != INVALID_HANDLE_VALUE)
	    wait_state = fifo_ok;
	  else if (nonblocking_write)
	    {
	      set_errno (ENXIO);
	      do_seterrno = false;
	    }
	  else if ((h = cnp (PIPE_ACCESS_DUPLEX, 1)) != INVALID_HANDLE_VALUE)
	    {
	      if ((dummy_client = open_nonserver (npname, low_flags, sa_buf))
		  != INVALID_HANDLE_VALUE)
		{
		  wait_state = fifo_wait_for_server;
		  ProtectHandle (dummy_client);
		}
	      else
		{
		  DWORD saveerr = GetLastError ();
		  CloseHandle (h);
		  h = INVALID_HANDLE_VALUE;
		  SetLastError (saveerr);
		}
	    }
	}
      if (h == INVALID_HANDLE_VALUE)
	{
	  if (do_seterrno)
	    __seterrno ();
	  res = 0;
	}
      else if (!setup_overlapped ())
	{
	  CloseHandle (h);
	  __seterrno ();
	  res = 0;
	}
      else
	{
	  set_io_handle (h);
	  set_flags (flags);
	  res = 1;
	}
    }

  debug_printf ("returning %d, errno %d", res, get_errno ());
  return res;
}

bool
fhandler_fifo::wait (bool iswrite)
{
  DWORD ninstances;
  switch (wait_state)
    {
    case fifo_wait_for_next_client:
      DisconnectNamedPipe (get_handle ());
      if (!GetNamedPipeHandleState (get_handle (), NULL, &ninstances, NULL, NULL, NULL, 0))
	{
	  __seterrno ();
	  wait_state = fifo_error;
	  return false;
	}
      if (ninstances <= 1)
	{
	  wait_state = fifo_eof;
	  return false;
	}
    case fifo_wait_for_client:
      {
	DWORD dummy_bytes;
	while (1)
	  {
	    int res = ConnectNamedPipe (get_handle (), get_overlapped ());
	    if (GetLastError () != ERROR_NO_DATA && GetLastError () != ERROR_PIPE_CONNECTED)
	      {
		res = wait_overlapped (res, iswrite, &dummy_bytes);
		if (!res)
		  {
		    if (get_errno () != EINTR)
		      wait_state = fifo_error;
		    else if (!_my_tls.call_signal_handler ())
		      wait_state = fifo_eintr;
		    else
		      continue;
		    return false;
		  }
	      }
	    wait_state = fifo_ok;
	    break;
	  }
      }
      break;
    case fifo_wait_for_server:
      char npname[MAX_PATH];
      fifo_name (npname);
      char char_sa_buf[1024];
      LPSECURITY_ATTRIBUTES sa_buf;
      sa_buf = sec_user ((PSECURITY_ATTRIBUTES) char_sa_buf, cygheap->user.sid());
      while (1)
	{
	  if (WaitNamedPipe (npname, 10))
	    /* connected, maybe */;
	  else if (GetLastError () != ERROR_SEM_TIMEOUT)
	    {
	      __seterrno ();
	      return false;
	    }
	  else if (WaitForSingleObject (signal_arrived, 0) != WAIT_OBJECT_0)
	    continue;
	  else if (_my_tls.call_signal_handler ())
	    continue;
	  else
	    {
	      set_errno (EINTR);
	      return false;
	    }
	  HANDLE h = open_nonserver (npname, get_flags () & O_ACCMODE, sa_buf);
	  if (h != INVALID_HANDLE_VALUE)
	    {
	      ForceCloseHandle (get_handle ());
	      ForceCloseHandle (dummy_client);
	      dummy_client = NULL;
	      wait_state = fifo_ok;
	      set_io_handle (h);
	      break;
	    }
	  if (GetLastError () == ERROR_PIPE_LISTENING)
	    continue;
	  else
	    {
	      __seterrno ();
	      return false;
	    }
	}
    default:
      break;
    }
    return true;
}

void __stdcall
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  while (wait_state != fifo_eof && wait_state != fifo_error && wait_state != fifo_eintr)
    if (!wait (false))
      len = (wait_state == fifo_error || wait_state == fifo_eintr) ? (size_t) -1 : 0;
    else
      {
	size_t prev_len = len;
	read_overlapped (in_ptr, len);
	if (len)
	  break;
	wait_state = fifo_wait_for_next_client;
	len = prev_len;
      }
  if (wait_state == fifo_eintr)
    wait_state = fifo_wait_for_client;
  debug_printf ("returning %d, mode %d, %E\n", len, get_errno ());
}

ssize_t __stdcall
fhandler_fifo::raw_write (const void *ptr, size_t len)
{
  return wait (true) ? write_overlapped (ptr, len) : -1;
}

int __stdcall
fhandler_fifo::fstatvfs (struct statvfs *sfs)
{
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

int
fhandler_fifo::close ()
{
  wait_state = fifo_eof;
  if (dummy_client)
    {
      ForceCloseHandle (dummy_client);
      dummy_client = NULL;
    }
  return fhandler_base::close ();
}

int
fhandler_fifo::dup (fhandler_base *child)
{
  int res = fhandler_base::dup (child);
  fhandler_fifo *fifo_child = (fhandler_fifo *) child;
  if (res == 0 && dummy_client)
    {
      bool dres = DuplicateHandle (hMainProc, dummy_client, hMainProc,
				   &fifo_child->dummy_client, 0,
				   TRUE, DUPLICATE_SAME_ACCESS);
      if (!dres)
	{
	  fifo_child->dummy_client = NULL;
	  child->close ();
	  __seterrno ();
	  res = -1;
	}
    }
  return res;
}

void
fhandler_fifo::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  if (dummy_client)
    set_no_inheritance (dummy_client, val);
}
