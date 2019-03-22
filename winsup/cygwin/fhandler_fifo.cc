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
  fhandler_base (),
  read_ready (NULL), write_ready (NULL)
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

/* Called when pipe is opened for reading. */
HANDLE
fhandler_fifo::create_pipe ()
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
  ULONG max_instances = 1;
  LARGE_INTEGER timeout;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  access = GENERIC_READ | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES
    | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  hattr = OBJ_INHERIT | OBJ_CASE_INSENSITIVE;
  InitializeObjectAttributes (&attr, get_pipe_name (),
			      hattr, npfsh, NULL);
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_CREATE, 0,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  DEFAULT_PIPEBUFSIZE, DEFAULT_PIPEBUFSIZE,
				  &timeout);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return ph;
}

/* Called when file is opened for writing. */
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
    set_io_handle (ph);
  return status;
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
  HANDLE ph = NULL;

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
      duplexer = true;
      break;
    default:
      set_errno (EINVAL);
      res = error_errno_set;
      goto out;
    }

  debug_only_printf ("reader %d, writer %d, duplexer %d", reader, writer, duplexer);
  set_flags (flags);
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

  /* If we're reading, create the pipe, signal that we're ready and wait for
     a writer.
     FIXME: Probably need to special case O_RDWR case.  */
  if (reader)
    {
      ph = create_pipe ();
      if (!ph)
	{
	  debug_printf ("create of reader failed");
	  res = error_errno_set;
	  goto out;
	}
      else if (!arm (read_ready))
	{
	  res = error_set_errno;
	  goto out;
	}
      else if (!duplexer && !wait (write_ready))
	{
	  res = error_errno_set;
	  goto out;
	}
      else
	res = success;
    }

  /* If we're writing, wait for read_ready and then connect to the
     pipe.  Then signal write_ready.  */
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
	res = success;
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

void __reg3
fhandler_fifo::raw_read (void *in_ptr, size_t& len)
{
  size_t orig_len = len;
  while (1)
    {
      len = orig_len;
      fhandler_base::raw_read (in_ptr, len);
      ssize_t nread = (ssize_t) len;
      if (nread > 0)
	return;
      else if (nread < 0 && GetLastError () != ERROR_NO_DATA)
	goto errout;
      else if (nread == 0) /* Writer has disconnected. */
	{
	  /* Not implemented yet. */
	}
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
fhandler_fifo::close ()
{
  CloseHandle (read_ready);
  CloseHandle (write_ready);
  return fhandler_base::close ();
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
  return 0;
}

void
fhandler_fifo::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
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
