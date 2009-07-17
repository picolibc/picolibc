/* fhandler_fifo.cc - See fhandler.h for a description of the fhandler classes.

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008 Red Hat, Inc.

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

fhandler_fifo::fhandler_fifo ():
  wait_state (fifo_unknown)
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

#define FIFO_PIPE_MODE (PIPE_TYPE_BYTE | PIPE_READMODE_BYTE)

char *
fhandler_fifo::fifo_name (char *buf)
{
  /* Generate a semi-unique name to associate with this fifo. */
  __small_sprintf (buf, "\\\\.\\pipe\\__cygfifo__%08x_%016X",
		   get_dev (), get_ino ());
  return buf;
}

int
fhandler_fifo::open (int flags, mode_t)
{
  int res = 1;
  char npname[MAX_PATH];

  fifo_name (npname);

  unsigned low_flags = flags & O_ACCMODE;
  DWORD mode = 0;
  if (low_flags == O_WRONLY)
    /* ok */;
  else if (low_flags == O_RDONLY)
    mode = PIPE_ACCESS_INBOUND;
  else if (low_flags == O_RDWR)
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
      mode |= FILE_FLAG_OVERLAPPED;

      HANDLE h;
      DWORD err;
      bool nonblocking_write = !!((flags & (O_WRONLY | O_NONBLOCK)) == (O_WRONLY | O_NONBLOCK));
      if (flags & O_WRONLY)
	{
	  h = INVALID_HANDLE_VALUE;
	  err = ERROR_ACCESS_DENIED;
	}
      else
	{
	  h = CreateNamedPipe(npname, mode, FIFO_PIPE_MODE,
			      PIPE_UNLIMITED_INSTANCES, 0, 0,
			      NMPWAIT_WAIT_FOREVER, sa_buf);
	  err = GetLastError ();
	}
      if (h != INVALID_HANDLE_VALUE)
	wait_state = fifo_wait_for_client;
      else
	  switch (err)
	    {
	    case ERROR_ACCESS_DENIED:
	      h = open_nonserver (npname, low_flags, sa_buf);
	      if (h != INVALID_HANDLE_VALUE)
		{
		  wait_state = fifo_ok;
		  break;
		}
	      if (GetLastError () != ERROR_FILE_NOT_FOUND)
		__seterrno ();
	      else if (nonblocking_write)
		set_errno (ENXIO);
	      else
		{
		  h = NULL;
		  nohandle (true);
		  wait_state = fifo_wait_for_server;
		}
	      break;
	    default:
	      __seterrno ();
	      break;
	    }
      if (h == INVALID_HANDLE_VALUE)
	res = 0;
      else if (!setup_overlapped ())
	{
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
  switch (wait_state)
    {
    case fifo_wait_for_next_client:
      DisconnectNamedPipe (get_handle ());
    case fifo_wait_for_client:
      {
	int res;
	if ((res = ConnectNamedPipe (get_handle (), get_overlapped ()))
	    || GetLastError () == ERROR_PIPE_CONNECTED)
	  {
	    wait_state = fifo_ok;
	    return true;
	  }
	if (wait_state == fifo_wait_for_next_client)
	  {
	    CancelIo (get_handle ());
	    res = 0;
	  }
	else
	  {
	    DWORD dummy_bytes;
	    res = wait_overlapped (res, iswrite, &dummy_bytes);
	  }
	wait_state = res ? fifo_ok : fifo_eof;
      }
      break;
    case fifo_wait_for_server:
      if (get_io_handle ())
	break;
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
	      wait_state = fifo_ok;
	      set_io_handle (h);
	      nohandle (false);
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

void
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  while (wait_state != fifo_eof)
    if (!wait (false))
      len = 0;
    else
      {
	read_overlapped (in_ptr, len);
	if (!len)
	  wait_state = fifo_wait_for_next_client;
      }
}

int
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
