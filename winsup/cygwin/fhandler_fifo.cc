/* fhandler_fifo.cc - See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <w32api/winioctl.h>
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
#include "cygwait.h"

/* This is only to be used for writers.  When reading,
STATUS_PIPE_EMPTY simply means there's no data to be read. */
#define STATUS_PIPE_IS_CLOSED(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_PIPE_CLOSING \
		   || _s == STATUS_PIPE_BROKEN \
		   || _s == STATUS_PIPE_EMPTY; })

fhandler_fifo::fhandler_fifo ():
  fhandler_base (), read_ready (NULL), write_ready (NULL),
  listen_client_thr (NULL), lct_termination_evt (NULL), nclients (0),
  nconnected (0), _duplexer (false)
{
  pipe_name_buf[0] = L'\0';
  need_fork_fixup (true);
}

PUNICODE_STRING
fhandler_fifo::get_pipe_name ()
{
  if (!pipe_name_buf[0])
    {
      __small_swprintf (pipe_name_buf, L"%S-fifo.%08x.%016X",
			&cygheap->installation_key, get_dev (), get_ino ());
      RtlInitUnicodeString (&pipe_name, pipe_name_buf);
    }
  return &pipe_name;
}

inline PSECURITY_ATTRIBUTES
sec_user_cloexec (bool cloexec, PSECURITY_ATTRIBUTES sa, PSID sid)
{
  return cloexec ? sec_user_nih (sa, sid) : sec_user (sa, sid);
}

bool inline
fhandler_fifo::arm (HANDLE h)
{
#ifdef DEBUGGING
  const char *what;
  if (h == read_ready)
    what = "reader";
  else
    what = "writer";
  debug_only_printf ("arming %s", what);
#endif

  bool res = SetEvent (h);
  if (!res)
#ifdef DEBUGGING
    debug_printf ("SetEvent for %s failed, %E", what);
#else
    debug_printf ("SetEvent failed, %E");
#endif
  return res;
}

static HANDLE
create_event ()
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  HANDLE evt = NULL;

  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return evt;
}


static void
set_pipe_non_blocking (HANDLE ph, bool nonblocking)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_PIPE_INFORMATION fpi;

  fpi.ReadMode = FILE_PIPE_MESSAGE_MODE;
  fpi.CompletionMode = nonblocking ? FILE_PIPE_COMPLETE_OPERATION
    : FILE_PIPE_QUEUE_OPERATION;
  status = NtSetInformationFile (ph, &io, &fpi, sizeof fpi,
				 FilePipeInformation);
  if (!NT_SUCCESS (status))
    debug_printf ("NtSetInformationFile(FilePipeInformation): %y", status);
}

/* The pipe instance is always in blocking mode when this is called. */
int
fifo_client_handler::connect ()
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  if (connect_evt)
    ResetEvent (connect_evt);
  else if (!(connect_evt = create_event ()))
    return -1;
  status = NtFsControlFile (fh->get_handle (), connect_evt, NULL, NULL, &io,
			    FSCTL_PIPE_LISTEN, NULL, 0, NULL, 0);
  switch (status)
    {
    case STATUS_PENDING:
    case STATUS_PIPE_LISTENING:
      state = fc_connecting;
      break;
    case STATUS_PIPE_CONNECTED:
      state = fc_connected;
      set_pipe_non_blocking (fh->get_handle (), true);
      break;
    default:
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

int
fhandler_fifo::disconnect_and_reconnect (int i)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  HANDLE ph = client[i].fh->get_handle ();

  status = NtFsControlFile (ph, NULL, NULL, NULL, &io, FSCTL_PIPE_DISCONNECT,
			    NULL, 0, NULL, 0);
  /* Short-lived.  Don't use cygwait.  We don't want to be interrupted. */
  if (status == STATUS_PENDING
      && NtWaitForSingleObject (ph, FALSE, NULL) == WAIT_OBJECT_0)
    status = io.Status;
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  set_pipe_non_blocking (client[i].fh->get_handle (), false);
  if (client[i].connect () < 0)
    return -1;
  if (client[i].state == fc_connected)
    nconnected++;
  return 0;
}

NTSTATUS
fhandler_fifo::npfs_handle (HANDLE &nph)
{
  static NO_COPY SRWLOCK npfs_lock;
  static NO_COPY HANDLE npfs_dirh;

  NTSTATUS status = STATUS_SUCCESS;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;

  /* Lockless after first call. */
  if (npfs_dirh)
    {
      nph = npfs_dirh;
      return STATUS_SUCCESS;
    }
  AcquireSRWLockExclusive (&npfs_lock);
  if (!npfs_dirh)
    {
      InitializeObjectAttributes (&attr, &ro_u_npfs, 0, NULL, NULL);
      status = NtOpenFile (&npfs_dirh, FILE_READ_ATTRIBUTES | SYNCHRONIZE,
			   &attr, &io, FILE_SHARE_READ | FILE_SHARE_WRITE,
			   0);
    }
  ReleaseSRWLockExclusive (&npfs_lock);
  if (NT_SUCCESS (status))
    nph = npfs_dirh;
  return status;
}

/* Called when a FIFO is first opened for reading and again each time
   a new client is needed.  Each pipe instance is created in blocking
   mode so that we can easily wait for a connection.  After it is
   connected, it is put in nonblocking mode. */
HANDLE
fhandler_fifo::create_pipe_instance (bool first)
{
  NTSTATUS status;
  HANDLE npfsh;
  HANDLE ph = NULL;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG hattr;
  ULONG sharing;
  ULONG nonblocking = FILE_PIPE_QUEUE_OPERATION;
  ULONG max_instances = -1;
  LARGE_INTEGER timeout;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  access = GENERIC_READ | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES
    | SYNCHRONIZE;
  if (first && _duplexer)
    access |= GENERIC_WRITE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  hattr = OBJ_INHERIT;
  if (first)
    hattr |= OBJ_CASE_INSENSITIVE;
  InitializeObjectAttributes (&attr, get_pipe_name (),
			      hattr, npfsh, NULL);
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  first ? FILE_CREATE : FILE_OPEN, 0,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  DEFAULT_PIPEBUFSIZE, DEFAULT_PIPEBUFSIZE,
				  &timeout);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return ph;
}

/* Called when a FIFO is opened for writing. */
NTSTATUS
fhandler_fifo::open_pipe ()
{
  NTSTATUS status;
  HANDLE npfsh;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  HANDLE ph = NULL;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    return status;
  access = GENERIC_WRITE | SYNCHRONIZE;
  InitializeObjectAttributes (&attr, get_pipe_name (), OBJ_INHERIT,
			      npfsh, NULL);
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  status = NtOpenFile (&ph, access, &attr, &io, sharing, 0);
  if (NT_SUCCESS (status))
    set_handle (ph);
  return status;
}

int
fhandler_fifo::add_client ()
{
  fifo_client_handler fc;
  fhandler_base *fh;
  bool first = (nclients == 0);

  if (nclients == MAX_CLIENTS)
    {
      set_errno (EMFILE);
      return -1;
    }
  if (!(fc.dummy_evt = create_event ()))
    return -1;
  if (!(fh = build_fh_dev (dev ())))
    {
      set_errno (EMFILE);
      return -1;
    }
  fc.fh = fh;
  HANDLE ph = create_pipe_instance (first);
  if (!ph)
    goto errout;
  fh->set_handle (ph);
  fh->set_flags (get_flags ());
  if (fc.connect () < 0)
    {
      fc.close ();
      goto errout;
    }
  if (fc.state == fc_connected)
    nconnected++;
  client[nclients++] = fc;
  return 0;
errout:
  delete fh;
  return -1;

}

/* Just hop to the listen_client_thread method. */
DWORD WINAPI
listen_client_func (LPVOID param)
{
  fhandler_fifo *fh = (fhandler_fifo *) param;
  return fh->listen_client_thread ();
}

/* Start a thread that listens for client connections.  Whenever a new
   client connects, it creates a new pipe_instance if necessary.
   (There may already be an available instance if a client has
   disconnected.)  */
bool
fhandler_fifo::listen_client ()
{
  if (!(lct_termination_evt = create_event ()))
    return false;

  listen_client_thr = CreateThread (NULL, PREFERRED_IO_BLKSIZE,
				    listen_client_func, (PVOID) this, 0, NULL);
  if (!listen_client_thr)
    {
      __seterrno ();
      HANDLE evt = InterlockedExchangePointer (&lct_termination_evt, NULL);
      if (evt)
	CloseHandle (evt);
      return false;
    }
  return true;
}

DWORD
fhandler_fifo::listen_client_thread ()
{
  while (1)
    {
      bool found;
      HANDLE w[MAX_CLIENTS + 1];
      int i;
      DWORD wait_ret;

      fifo_client_lock ();
      found = false;
      for (i = 0; i < nclients; i++)
	switch (client[i].state)
	  {
	  case fc_invalid:
	    if (disconnect_and_reconnect (i) < 0)
	      {
		fifo_client_unlock ();
		goto errout;
	      }
	    /* Fall through. */
	  case fc_connected:
	    w[i] = client[i].dummy_evt;
	    break;
	  case fc_connecting:
	    found = true;
	    w[i] = client[i].connect_evt;
	    break;
	  case fc_unknown:	/* Shouldn't happen. */
	  default:
	    break;
	  }
      w[nclients] = lct_termination_evt;
      int res = 0;
      if (!found)
	res = add_client ();
      fifo_client_unlock ();
      if (res < 0)
	goto errout;
      else if (!found)
	continue;

      if (!arm (read_ready))
	{
	  __seterrno ();
	  goto errout;
	}

      /* Wait for a client to connect. */
      wait_ret = WaitForMultipleObjects (nclients + 1, w, false, INFINITE);
      i = wait_ret - WAIT_OBJECT_0;
      if (i < 0 || i > nclients)
	goto errout;
      else if (i == nclients)	/* Reader is closing. */
	return 0;
      else
	{
	  fifo_client_lock ();
	  client[i].state = fc_connected;
	  nconnected++;
	  set_pipe_non_blocking (client[i].fh->get_handle (), true);
	  fifo_client_unlock ();
	  yield ();
	}
    }
errout:
  ResetEvent (read_ready);
  return -1;
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
  bool reader, writer, duplexer;

  /* Determine what we're doing with this fhandler: reading, writing, both */
  switch (flags & O_ACCMODE)
    {
    case O_RDONLY:
      reader = true;
      writer = false;
      duplexer = false;
      break;
    case O_WRONLY:
      writer = true;
      reader = false;
      duplexer = false;
      break;
    case O_RDWR:
      reader = true;
      writer = false;
      duplexer = _duplexer = true;
      break;
    default:
      set_errno (EINVAL);
      res = error_errno_set;
      goto out;
    }

  debug_only_printf ("reader %d, writer %d, duplexer %d", reader, writer, duplexer);
  set_flags (flags);
  if (reader && !duplexer)
    nohandle (true);

  /* Create control events for this named pipe */
  char char_sa_buf[1024];
  LPSECURITY_ATTRIBUTES sa_buf;
  sa_buf = sec_user_cloexec (flags & O_CLOEXEC, (PSECURITY_ATTRIBUTES) char_sa_buf,
		      cygheap->user.sid());

  char npbuf[MAX_PATH];
  __small_sprintf (npbuf, "r-event.%08x.%016X", get_dev (), get_ino ());
  if (!(read_ready = CreateEvent (sa_buf, duplexer, false, npbuf)))
    {
      debug_printf ("CreateEvent for %s failed, %E", npbuf);
      res = error_set_errno;
      goto out;
    }
  npbuf[0] = 'w';
  if (!(write_ready = CreateEvent (sa_buf, false, false, npbuf)))
    {
      debug_printf ("CreateEvent for %s failed, %E", npbuf);
      res = error_set_errno;
      goto out;
    }

  /* If we're a duplexer, create the pipe and the first client. */
  if (duplexer)
    {
      HANDLE ph, connect_evt, dummy_evt;
      fhandler_base *fh;

      ph = create_pipe_instance (true);
      if (!ph)
	{
	  res = error_errno_set;
	  goto out;
	}
      set_handle (ph);
      set_pipe_non_blocking (ph, true);
      if (!(fh = build_fh_dev (dev ())))
	{
	  set_errno (EMFILE);
	  res = error_errno_set;
	  goto out;
	}
      fh->set_handle (ph);
      fh->set_flags (flags);
      if (!(connect_evt = create_event ()))
	{
	  res = error_errno_set;
	  fh->close ();
	  delete fh;
	  goto out;
	}
      if (!(dummy_evt = create_event ()))
	{
	  res = error_errno_set;
	  delete fh;
	  fh->close ();
	  CloseHandle (connect_evt);
	  goto out;
	}
      client[0] = fifo_client_handler (fh, fc_connected, connect_evt,
				       dummy_evt);
      nconnected = nclients = 1;
    }

  /* If we're reading, start the listen_client thread (which should
     signal read_ready), and wait for a writer. */
  if (reader)
    {
      if (!listen_client ())
	{
	  debug_printf ("create of listen_client thread failed");
	  res = error_errno_set;
	  goto out;
	}
      /* Wait for the listen_client thread to signal read_ready.  This
	 should be quick.  */
      HANDLE w[2] = { listen_client_thr, read_ready };
      switch (WaitForMultipleObjects (2, w, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  debug_printf ("listen_client_thread exited unexpectedly");
	  DWORD err;
	  GetExitCodeThread (listen_client_thr, &err);
	  __seterrno_from_win_error (err);
	  res = error_errno_set;
	  goto out;
	  break;
	case WAIT_OBJECT_0 + 1:
	  if (!arm (read_ready))
	    {
	      res = error_set_errno;
	      goto out;
	    }
	  break;
	default:
	  res = error_set_errno;
	  goto out;
	  break;
	}
      if (!duplexer && !wait (write_ready))
	{
	  res = error_errno_set;
	  goto out;
	}
      else
	res = success;
    }

  /* If we're writing, wait for read_ready and then connect to the
     pipe.  This should always succeed quickly if the reader's
     listen_client thread is running.  Then signal write_ready.  */
  if (writer)
    {
      if (!wait (read_ready))
	{
	  res = error_errno_set;
	  goto out;
	}
      NTSTATUS status = open_pipe ();
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("create of writer failed");
	  __seterrno_from_nt_status (status);
	  res = error_errno_set;
	  goto out;
	}
      else if (!arm (write_ready))
	{
	  res = error_set_errno;
	  goto out;
	}
      else
	{
	  set_pipe_non_blocking (get_handle (), true);
	  res = success;
	}
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
      if (get_handle ())
	CloseHandle (get_handle ());
      if (listen_client_thr)
	CloseHandle (listen_client_thr);
    }
  debug_printf ("res %d", res);
  return res == success;
}

off_t
fhandler_fifo::lseek (off_t offset, int whence)
{
  debug_printf ("(%D, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

bool
fhandler_fifo::wait (HANDLE h)
{
#ifdef DEBUGGING
  const char *what;
  if (h == read_ready)
    what = "reader";
  else
    what = "writer";
#endif
  /* Set the wait to zero for non-blocking I/O-related events. */
  DWORD wait = ((h == read_ready || h == write_ready)
		&& get_flags () & O_NONBLOCK) ? 0 : INFINITE;

  debug_only_printf ("waiting for %s", what);
  /* Wait for the event.  Set errno, as appropriate if something goes wrong. */
  switch (cygwait (h, wait))
    {
    case WAIT_OBJECT_0:
      debug_only_printf ("successfully waited for %s", what);
      return true;
    case WAIT_SIGNALED:
      debug_only_printf ("interrupted by signal while waiting for %s", what);
      set_errno (EINTR);
      return false;
    case WAIT_CANCELED:
      debug_only_printf ("cancellable interruption while waiting for %s", what);
      pthread::static_cancel_self ();	/* never returns */
      break;
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
    default:
      debug_only_printf ("unknown error while waiting for %s", what);
      __seterrno ();
      return false;
   }
}

ssize_t __reg3
fhandler_fifo::raw_write (const void *ptr, size_t len)
{
  ssize_t ret = -1;
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  status = NtWriteFile (get_handle (), NULL, NULL, NULL, &io,
			(PVOID) ptr, len, NULL, NULL);
  if (NT_SUCCESS (status))
    {
      /* NtWriteFile returns success with # of bytes written == 0 in
	 case writing on a non-blocking pipe fails if the pipe buffer
	 is full. */
      if (io.Information == 0)
	set_errno (EAGAIN);
      else
	ret = io.Information;
    }
  else if (STATUS_PIPE_IS_CLOSED (status))
    {
      set_errno (EPIPE);
      raise (SIGPIPE);
    }
  else
    __seterrno_from_nt_status (status);
  return ret;
}

/* A FIFO open for reading is at EOF if no process has it open for
   writing.  We test this by checking nconnected.  But we must take
   account of the possible delay from the time of connection to the
   time the connection is recorded by the listen_client thread. */
bool
fhandler_fifo::hit_eof ()
{
  fifo_client_lock ();
  bool eof = (nconnected == 0);
  fifo_client_unlock ();
  if (eof)
    {
      /* Give the listen_client thread time to catch up, then recheck. */
      Sleep (1);
      eof = (nconnected == 0);
    }
  return eof;
}

void __reg3
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  size_t orig_len = len;

  /* Start the listen_client thread if necessary (e.g., after dup or fork). */
  if (!listen_client_thr && !listen_client ())
    goto errout;

  while (1)
    {
      if (hit_eof ())
	{
	  len = 0;
	  return;
	}

      /* Poll the connected clients for input. */
      fifo_client_lock ();
      for (int i = 0; i < nclients; i++)
	if (client[i].state == fc_connected)
	  {
	    len = orig_len;
	    client[i].fh->fhandler_base::raw_read (in_ptr, len);
	    ssize_t nread = (ssize_t) len;
	    if (nread > 0)
	      {
		fifo_client_unlock ();
		return;
	      }
	    /* In the duplex case with no data, we seem to get nread
	       == -1 with ERROR_PIPE_LISTENING on the first attempt to
	       read from the duplex pipe (client[0]), and nread == 0
	       on subsequent attempts. */
	    else if (nread < 0)
	      switch (GetLastError ())
		{
		case ERROR_NO_DATA:
		  break;
		case ERROR_PIPE_LISTENING:
		  if (_duplexer && i == 0)
		    break;
		  /* Fall through. */
		default:
		  fifo_client_unlock ();
		  goto errout;
		}
	    else if (nread == 0 && (!_duplexer || i > 0))
	      /* Client has disconnected. */
	      {
		client[i].state = fc_invalid;
		nconnected--;
	      }
	  }
      fifo_client_unlock ();
      if (is_nonblocking ())
	{
	  set_errno (EAGAIN);
	  goto errout;
	}
      else
	{
	  /* Allow interruption.  Copied from
	     fhandler_socket_unix::open_reparse_point. */
	  pthread_testcancel ();
	  if (cygwait (NULL, cw_nowait, cw_sig_eintr) == WAIT_SIGNALED
	      && !_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      goto errout;
	    }
	  /* Don't hog the CPU. */
	  Sleep (1);
	}
    }
errout:
  len = -1;
}

int __reg2
fhandler_fifo::fstatvfs (struct statvfs *sfs)
{
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

int
fifo_client_handler::close ()
{
  int res = 0;

  if (fh)
    res = fh->close ();
  if (connect_evt)
    CloseHandle (connect_evt);
  if (dummy_evt)
    CloseHandle (dummy_evt);
  return res;
}

int
fhandler_fifo::close ()
{
  int res = 0;
  HANDLE evt = InterlockedExchangePointer (&lct_termination_evt, NULL);
  HANDLE thr = InterlockedExchangePointer (&listen_client_thr, NULL);
  if (thr)
    {
      if (evt)
	SetEvent (evt);
      WaitForSingleObject (thr, INFINITE);
      DWORD err;
      GetExitCodeThread (thr, &err);
      if (err)
	debug_printf ("listen_client_thread exited with code %d", err);
      CloseHandle (thr);
    }
  if (evt)
    CloseHandle (evt);
  if (read_ready)
    CloseHandle (read_ready);
  if (write_ready)
    CloseHandle (write_ready);
  for (int i = 0; i < nclients; i++)
    if (client[i].close () < 0)
      res = -1;
  return fhandler_base::close () || res;
}

int
fhandler_fifo::dup (fhandler_base *child, int flags)
{
  if (fhandler_base::dup (child, flags))
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
  for (int i = 0; i < nclients; i++)
    {
      if (!DuplicateHandle (GetCurrentProcess (), client[i].fh->get_handle (),
			    GetCurrentProcess (),
			    &fhf->client[i].fh->get_handle (),
			    0, true, DUPLICATE_SAME_ACCESS)
	  || !DuplicateHandle (GetCurrentProcess (), client[i].connect_evt,
			       GetCurrentProcess (),
			       &fhf->client[i].connect_evt,
			       0, true, DUPLICATE_SAME_ACCESS)
	  || !DuplicateHandle (GetCurrentProcess (), client[i].dummy_evt,
			       GetCurrentProcess (),
			       &fhf->client[i].dummy_evt,
			       0, true, DUPLICATE_SAME_ACCESS))
	{
	  CloseHandle (fhf->read_ready);
	  CloseHandle (fhf->write_ready);
	  fhf->close ();
	  __seterrno ();
	  return -1;
	}
    }
  fhf->listen_client_thr = NULL;
  fhf->lct_termination_evt = NULL;
  fhf->fifo_client_unlock ();
  return 0;
}

void
fhandler_fifo::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
  fork_fixup (parent, read_ready, "read_ready");
  fork_fixup (parent, write_ready, "write_ready");
  for (int i = 0; i < nclients; i++)
    {
      client[i].fh->fhandler_base::fixup_after_fork (parent);
      fork_fixup (parent, client[i].connect_evt, "connect_evt");
      fork_fixup (parent, client[i].dummy_evt, "dummy_evt");
    }
  listen_client_thr = NULL;
  lct_termination_evt = NULL;
  fifo_client_unlock ();
}

void
fhandler_fifo::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  set_no_inheritance (read_ready, val);
  set_no_inheritance (write_ready, val);
  for (int i = 0; i < nclients; i++)
    {
      client[i].fh->fhandler_base::set_close_on_exec (val);
      set_no_inheritance (client[i].connect_evt, val);
      set_no_inheritance (client[i].dummy_evt, val);
    }
}
