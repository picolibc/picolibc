/* fhandler_fifo.cc - See fhandler.h for a description of the fhandler classes.

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011
   Red Hat, Inc.

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
#include "ntdll.h"

fhandler_fifo::fhandler_fifo ():
  fhandler_base_overlapped (),
  read_ready (NULL), write_ready (NULL)
{
  max_atomic_write = DEFAULT_PIPEBUFSIZE;
  need_fork_fixup (true);
}

#define fnevent(w) fifo_name (npbuf, w "-event")
#define fnpipe() fifo_name (npbuf, "fifo")
#define create_pipe(r, w) \
  fhandler_pipe::create_selectable (sa_buf, (r), (w), 0, fnpipe (), open_mode)

char *
fhandler_fifo::fifo_name (char *buf, const char *what)
{
  /* Generate a semi-unique name to associate with this fifo. */
  __small_sprintf (buf, "%s.%08x.%016X", what, get_dev (),
		   get_ino ());
  return buf;
}

inline PSECURITY_ATTRIBUTES
sec_user_cloexec (bool cloexec, PSECURITY_ATTRIBUTES sa, PSID sid)
{
  return cloexec ? sec_user_nih (sa, sid) : sec_user (sa, sid);
}

int
fhandler_fifo::open (int flags, mode_t)
{
  enum
  {
    success,
    error_errno_set,
    error_set_errno
  } res;
  bool reader, writer;
  DWORD open_mode = FILE_FLAG_OVERLAPPED;

  /* Determine what we're doing with this fhandler: reading, writing, both */
  switch (flags & O_ACCMODE)
    {
    case O_RDONLY:
      reader = true;
      writer = false;
      break;
    case O_WRONLY:
      writer = true;
      reader = false;
      break;
    case O_RDWR:
      reader = true;
      writer = true;
      open_mode |= PIPE_ACCESS_DUPLEX;
      break;
    default:
      set_errno (EINVAL);
      res = error_errno_set;
      goto out;
    }

  set_flags (flags);
  char char_sa_buf[1024];
  LPSECURITY_ATTRIBUTES sa_buf;
  sa_buf = sec_user_cloexec (flags & O_CLOEXEC, (PSECURITY_ATTRIBUTES) char_sa_buf,
		      cygheap->user.sid());
  char npbuf[MAX_PATH];

  /* Create control events for this named pipe */
  if (!(read_ready = CreateEvent (sa_buf, true, false, fnevent ("r"))))
    {
      debug_printf ("CreatEvent for %s failed, %E", npbuf);
      res = error_set_errno;
      goto out;
    }
  if (!(write_ready = CreateEvent (sa_buf, true, false, fnevent ("w"))))
    {
      debug_printf ("CreatEvent for %s failed, %E", npbuf);
      res = error_set_errno;
      goto out;
    }

  /* If we're reading, create the pipe, signal that we're ready and wait for
     a writer.
     FIXME: Probably need to special case O_RDWR case.  */
  if (!reader)
    /* We are not a reader */;
  else if (create_pipe (&get_io_handle (), NULL))
    {
      debug_printf ("create of reader failed");
      res = error_set_errno;
      goto out;
    }
  else if (!SetEvent (read_ready))
    {
      debug_printf ("SetEvent for read_ready failed, %E");
      res = error_set_errno;
      goto out;
    }
  else if (!writer && !wait (write_ready))
    {
      debug_printf ("wait for write_ready failed, %E");
      res = error_errno_set;
      goto out;
    }

  /* If we're writing, it's a little tricky since it is possible that
     we're attempting to open the other end of a pipe which is already
     connected.  In that case, we detect ERROR_PIPE_BUSY, reset the
     read_ready event and wait for the reader to allow us to connect
     by signalling read_ready.

     Once the pipe has been set up, we signal write_ready.  */
  if (writer)
    {
      int err;
      while (1)
	if (!wait (read_ready))
	  {
	    res = error_errno_set;
	    goto out;
	  }
	else if ((err = create_pipe (NULL, &get_io_handle ())) == 0)
	  break;
	else if (err == ERROR_PIPE_BUSY)
	  {
	    debug_only_printf ("pipe busy");
	    ResetEvent (read_ready);
	  }
	else
	  {
	    debug_printf ("create of writer failed");
	    res = error_set_errno;
	    goto out;
	  }
      if (!SetEvent (write_ready))
	{
	  debug_printf ("SetEvent for write_ready failed, %E");
	  res = error_set_errno;
	  goto out;
	}
    }

  /* If setup_overlapped() succeeds (and why wouldn't it?) we are all set. */
  if (setup_overlapped () == 0)
    res = success;
  else
    {
      debug_printf ("setup_overlapped failed, %E");
      res = error_set_errno;
    }

out:
  if (res == error_set_errno)
    __seterrno ();
  if (res != success)
    {
      if (read_ready)
	{
	  CloseHandle (read_ready);
	  read_ready = NULL;
	}
      if (write_ready)
	{
	  CloseHandle (write_ready);
	  write_ready = NULL;
	}
      if (get_io_handle ())
	CloseHandle (get_io_handle ());
    }
  debug_printf ("res %d", res);
  return res == success;
}

bool
fhandler_fifo::wait (HANDLE h)
{
#ifdef DEBUGGING
  const char *what;
  if (h == read_ready)
    what = "reader";
  else if (h == write_ready)
    what = "writer";
  else
    what = "overlapped event";
#endif
  HANDLE w4[3] = {h, signal_arrived, pthread::get_cancel_event ()};

  /* Set the wait to zero for non-blocking I/O-related events. */
  DWORD wait = ((h == read_ready || h == write_ready)
		&& get_flags () & O_NONBLOCK) ? 0 : INFINITE;

  debug_only_printf ("waiting for %s", what);
  /* Wait for the event.  Set errno, as appropriate if something goes wrong. */
  switch (WaitForMultipleObjects (3, w4, false, wait))
    {
    case WAIT_OBJECT_0:
      debug_only_printf ("successfully waited for %s", what);
      return true;
    case WAIT_TIMEOUT:
      if (h == write_ready)
	{
	  debug_only_printf ("wait timed out waiting for write but will still open reader since non-blocking mode");
	  return true;
	}
      else
	{
	  set_errno (ENXIO);
	  return false;
	}
      break;
    case WAIT_OBJECT_0 + 1:
      debug_only_printf ("interrupted by signal while waiting for %s", what);
      set_errno (EINTR);
      return false;
    case WAIT_OBJECT_0 + 2:
      debug_only_printf ("cancellable interruption while waiting for %s", what);
      pthread::static_cancel_self ();	/* never returns */
      break;
    default:
      debug_only_printf ("unknown error while waiting for %s", what);
      __seterrno ();
      return false;
   }
}

void __stdcall
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  size_t orig_len = len;
  for (int i = 0; i < 2; i++)
    {
      fhandler_base_overlapped::raw_read (in_ptr, len);
      if (len || i || WaitForSingleObject (read_ready, 0) == WAIT_OBJECT_0)
	break;
      /* If we got here, then fhandler_base_overlapped::raw_read returned 0,
	 indicating "EOF" and something has set read_ready to zero.  That means
	 we should have a client waiting to connect.
	 FIXME: If the client CTRL-C's the open during this time then this
	 could hang indefinitely.  Maybe implement a timeout?  */
      if (!DisconnectNamedPipe (get_io_handle ()))
	{
	  debug_printf ("DisconnecttNamedPipe failed, %E");
	  goto errno_out;
	}
      else if (!ConnectNamedPipe (get_io_handle (), get_overlapped ())
	       && GetLastError () != ERROR_IO_PENDING)
	{
	  debug_printf ("ConnectNamedPipe failed, %E");
	  goto errno_out;
	}
      else if (!SetEvent (read_ready))
	{
	  debug_printf ("SetEvent (read_ready) failed, %E");
	  goto errno_out;
	}
      else if (!wait (get_overlapped_buffer ()->hEvent))
	goto errout;	/* If wait() fails, errno is set so no need to set it */
      len = orig_len;	/* Reset since raw_read above set it to zero. */
    }
  return;

errno_out:
  __seterrno ();
errout:
  len = -1;
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
  CloseHandle (read_ready);
  CloseHandle (write_ready);
  return fhandler_base::close ();
}

int
fhandler_fifo::dup (fhandler_base *child, int flags)
{
  if (fhandler_base_overlapped::dup (child, flags))
    {
      __seterrno ();
      return -1;
    }
  fhandler_fifo *fhf = (fhandler_fifo *) child;
  if (!DuplicateHandle (GetCurrentProcess (), read_ready,
			GetCurrentProcess (), &fhf->read_ready,
			0, true, DUPLICATE_SAME_ACCESS))
    {
      fhf->close ();
      __seterrno ();
      return -1;
    }
  if (!DuplicateHandle (GetCurrentProcess (), write_ready,
			GetCurrentProcess (), &fhf->write_ready,
			0, true, DUPLICATE_SAME_ACCESS))
    {
      CloseHandle (fhf->read_ready);
      fhf->close ();
      __seterrno ();
      return -1;
    }
  return 0;
}

void
fhandler_fifo::fixup_after_fork (HANDLE parent)
{
  fhandler_base_overlapped::fixup_after_fork (parent);
  fork_fixup (parent, read_ready, "read_ready");
  fork_fixup (parent, write_ready, "write_ready");
}

void
fhandler_fifo::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  set_no_inheritance (read_ready, val);
  set_no_inheritance (write_ready, val);
}
