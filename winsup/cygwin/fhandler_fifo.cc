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

int
fhandler_fifo::open (int flags, mode_t)
{
  int res;
  char npname[MAX_PATH];
  DWORD mode = 0;

  /* Generate a semi-unique name to associate with this fifo. */
  __small_sprintf (npname, "\\\\.\\pipe\\__cygfifo__%08x_%016X",
		   get_dev (), get_ino ());

  unsigned low_flags = flags & O_ACCMODE;
  if (low_flags == O_RDONLY)
    mode = PIPE_ACCESS_INBOUND;
  else if (low_flags == O_WRONLY)
    mode = PIPE_ACCESS_OUTBOUND;
  else if (low_flags == O_RDWR)
    mode = PIPE_ACCESS_DUPLEX;

  if (!mode)
    {
      set_errno (EINVAL);
      res = 0;
    }
  else
    {
      char char_sa_buf[1024];
      LPSECURITY_ATTRIBUTES sa_buf =
	sec_user ((PSECURITY_ATTRIBUTES) char_sa_buf, cygheap->user.sid());
      mode |= FILE_FLAG_OVERLAPPED;
      HANDLE h = CreateNamedPipe(npname, mode, FIFO_PIPE_MODE,
				 PIPE_UNLIMITED_INSTANCES, 0, 0,
				 NMPWAIT_WAIT_FOREVER, sa_buf);
      if (h != INVALID_HANDLE_VALUE)
	wait_state = fifo_wait_for_client;
      else
	  switch (GetLastError ())
	    {
	    case ERROR_ACCESS_DENIED:
	      h = open_nonserver (npname, low_flags, sa_buf);
	      if (h != INVALID_HANDLE_VALUE)
		{
		  wait_state = fifo_wait_for_server;
		  break;
		}
	      /* fall through intentionally */
	    default:
	      __seterrno ();
	      break;
	    }
      if (!h || h == INVALID_HANDLE_VALUE)
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
    case fifo_wait_for_client:
      {
	bool res = ConnectNamedPipe (get_handle (), get_overlapped ());
	DWORD dummy_bytes;
	if (res || GetLastError () == ERROR_PIPE_CONNECTED)
	  return true;
	return wait_overlapped (res, iswrite, &dummy_bytes);
      }
    case fifo_unknown:
    case fifo_wait_for_server:
      /* CGF FIXME SOON: test if these really need to be handled. */
    default:
      break;
    }
    return true;
}

void
fhandler_fifo::read (void *in_ptr, size_t& len)
{
  if (!wait (false))
    len = 0;
  else
    read_overlapped (in_ptr, len);
}

int
fhandler_fifo::write (const void *ptr, size_t len)
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
