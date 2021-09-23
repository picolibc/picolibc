/* fhandler_pipe.cc: pipes for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Should this really be fhandler_pipe.cc? */

#include "winsup.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "shared_info.h"
#include "tls_pbuf.h"
#include <psapi.h>

/* This is only to be used for writing.  When reading,
STATUS_PIPE_EMPTY simply means there's no data to be read. */
#define STATUS_PIPE_IS_CLOSED(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_PIPE_CLOSING \
		   || _s == STATUS_PIPE_BROKEN \
		   || _s == STATUS_PIPE_EMPTY; })

fhandler_pipe_fifo::fhandler_pipe_fifo ()
  : fhandler_base (), pipe_buf_size (DEFAULT_PIPEBUFSIZE)
{
}


fhandler_pipe::fhandler_pipe ()
  : fhandler_pipe_fifo (), popen_pid (0)
{
  need_fork_fixup (true);
}

/* The following function is intended for fhandler_pipe objects
   created by the second version of fhandler_pipe::create below.  See
   the comment preceding the latter.

   In addition to setting the blocking mode of the pipe handle, it
   also sets the pipe's read mode to byte_stream unconditionally. */
void
fhandler_pipe::set_pipe_non_blocking (bool nonblocking)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_PIPE_INFORMATION fpi;

  fpi.ReadMode = FILE_PIPE_BYTE_STREAM_MODE;
  fpi.CompletionMode = nonblocking ? FILE_PIPE_COMPLETE_OPERATION
    : FILE_PIPE_QUEUE_OPERATION;
  status = NtSetInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				 FilePipeInformation);
  if (!NT_SUCCESS (status))
    debug_printf ("NtSetInformationFile(FilePipeInformation): %y", status);
}

int
fhandler_pipe::init (HANDLE f, DWORD a, mode_t mode, int64_t uniq_id)
{
  /* FIXME: Have to clean this up someday
     FIXME: Do we have to check for both !get_win32_name() and
     !*get_win32_name()? */
  if ((!get_win32_name () || !*get_win32_name ()) && get_name ())
    {
      char *d;
      const char *s;
      char *hold_normalized_name = (char *) alloca (strlen (get_name ()) + 1);
      for (s = get_name (), d = hold_normalized_name; *s; s++, d++)
	if (*s == '/')
	  *d = '\\';
	else
	  *d = *s;
      *d = '\0';
      set_name (hold_normalized_name);
    }

  bool opened_properly = a & FILE_CREATE_PIPE_INSTANCE;
  a &= ~FILE_CREATE_PIPE_INSTANCE;
  fhandler_base::init (f, a, mode);
  close_on_exec (mode & O_CLOEXEC);
  set_ino (uniq_id);
  set_unique_id (uniq_id | !!(mode & GENERIC_WRITE));
  if (opened_properly)
    set_pipe_non_blocking (is_nonblocking ());
  return 1;
}

extern "C" int sscanf (const char *, const char *, ...);

int
fhandler_pipe::open (int flags, mode_t mode)
{
  HANDLE proc, nio_hdl = NULL;
  int64_t uniq_id;
  fhandler_pipe *fh = NULL, *fhr = NULL, *fhw = NULL;
  size_t size;
  int pid, rwflags = (flags & O_ACCMODE);
  bool inh;
  bool got_one = false;

  if (sscanf (get_name (), "/proc/%d/fd/pipe:[%llu]",
	      &pid, (long long *) &uniq_id) < 2)
    {
      set_errno (ENOENT);
      return 0;
    }
  if (pid == myself->pid)
    {
      cygheap_fdenum cfd (true);
      while (cfd.next () >= 0)
	{
	  /* Windows doesn't allow to copy a pipe HANDLE with another access
	     mode.  So we check for read and write side of pipe and try to
	     find the one matching the requested access mode. */
	  if (cfd->get_unique_id () == uniq_id)
	    got_one = true;
	  else if (cfd->get_unique_id () == uniq_id + 1)
	    got_one = true;
	  else
	    continue;
	  if ((rwflags == O_RDONLY && !(cfd->get_access () & GENERIC_READ))
	      || (rwflags == O_WRONLY && !(cfd->get_access () & GENERIC_WRITE)))
	    continue;
	  copy_from (cfd);
	  set_handle (NULL);
	  pc.close_conv_handle ();
	  if (!cfd->dup (this, flags))
	    return 1;
	  return 0;
	}
      /* Found the pipe but access mode didn't match? EACCES.
	 Otherwise ENOENT */
      set_errno (got_one ? EACCES : ENOENT);
      return 0;
    }

  pinfo p (pid);
  if (!p)
    {
      set_errno (ESRCH);
      return 0;
    }
  if (!(proc = OpenProcess (PROCESS_DUP_HANDLE, false, p->dwProcessId)))
    {
      __seterrno ();
      return 0;
    }
  fhr = p->pipe_fhandler (uniq_id, size);
  if (fhr && rwflags == O_RDONLY)
    fh = fhr;
  else
    {
      fhw = p->pipe_fhandler (uniq_id + 1, size);
      if (fhw && rwflags == O_WRONLY)
	fh = fhw;
    }
  if (!fh)
    {
      /* Too bad, but Windows only allows the same access mode when dup'ing
	 the pipe. */
      set_errno (fhr || fhw ? EACCES : ENOENT);
      goto out;
    }
  inh = !(flags & O_CLOEXEC);
  if (!DuplicateHandle (proc, fh->get_handle (), GetCurrentProcess (),
			&nio_hdl, 0, inh, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto out;
    }
  init (nio_hdl, fh->get_access (), mode & O_TEXT ?: O_BINARY,
	fh->get_plain_ino ());
  cfree (fh);
  CloseHandle (proc);
  return 1;
out:
  if (nio_hdl)
    CloseHandle (nio_hdl);
  if (fh)
    free (fh);
  if (proc)
    CloseHandle (proc);
  return 0;
}

bool
fhandler_pipe::open_setup (int flags)
{
  bool read_mtx_created = false;

  if (!fhandler_base::open_setup (flags))
    goto err;
  if (get_dev () == FH_PIPER && !read_mtx)
    {
      SECURITY_ATTRIBUTES *sa = sec_none_cloexec (flags);
      read_mtx = CreateMutex (sa, FALSE, NULL);
      if (read_mtx)
	read_mtx_created = true;
      else
	{
	  debug_printf ("CreateMutex read_mtx failed: %E");
	  goto err;
	}
    }
  if (!hdl_cnt_mtx)
    {
      SECURITY_ATTRIBUTES *sa = sec_none_cloexec (flags);
      hdl_cnt_mtx = CreateMutex (sa, FALSE, NULL);
      if (!hdl_cnt_mtx)
	{
	  debug_printf ("CreateMutex hdl_cnt_mtx failed: %E");
	  goto err_close_read_mtx;
	}
    }
  return true;

err_close_read_mtx:
  if (read_mtx_created)
    CloseHandle (read_mtx);
err:
  return false;
}

off_t
fhandler_pipe::lseek (off_t offset, int whence)
{
  debug_printf ("(%D, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_pipe::fadvise (off_t offset, off_t length, int advice)
{
  return ESPIPE;
}

int
fhandler_pipe::ftruncate (off_t length, bool allow_truncate)
{
  return allow_truncate ? EINVAL : ESPIPE;
}

char *
fhandler_pipe::get_proc_fd_name (char *buf)
{
  __small_sprintf (buf, "pipe:[%U]", get_plain_ino ());
  return buf;
}

void
fhandler_pipe::release_select_sem (const char *from)
{
  LONG n_release;
  if (get_dev () == FH_PIPER) /* Number of select() and writer */
    n_release = get_obj_handle_count (select_sem)
      - get_obj_handle_count (read_mtx);
  else /* Number of select() call */
    n_release = get_obj_handle_count (select_sem)
      - get_obj_handle_count (hdl_cnt_mtx);
  debug_printf("%s(%s) release %d", from,
	       get_dev () == FH_PIPER ? "PIPER" : "PIPEW", n_release);
  if (n_release)
    ReleaseSemaphore (select_sem, n_release, NULL);
}

void __reg3
fhandler_pipe::raw_read (void *ptr, size_t& len)
{
  size_t nbytes = 0;
  NTSTATUS status = STATUS_SUCCESS;
  IO_STATUS_BLOCK io;
  HANDLE evt = NULL;

  if (!len)
    return;

  /* Create a wait event if we're in blocking mode. */
  if (!is_nonblocking () && !(evt = CreateEvent (NULL, false, false, NULL)))
    {
      __seterrno ();
      len = (size_t) -1;
      return;
    }

  DWORD timeout = is_nonblocking () ? 0 : INFINITE;
  DWORD waitret = cygwait (read_mtx, timeout);
  switch (waitret)
    {
    case WAIT_OBJECT_0:
      break;
    case WAIT_TIMEOUT:
      set_errno (EAGAIN);
      len = (size_t) -1;
      return;
    default:
      set_errno (EINTR);
      len = (size_t) -1;
      return;
    }
  while (nbytes < len)
    {
      ULONG_PTR nbytes_now = 0;
      ULONG len1 = (ULONG) (len - nbytes);
      waitret = WAIT_OBJECT_0;

      if (evt)
	ResetEvent (evt);
      FILE_PIPE_LOCAL_INFORMATION fpli;
      status = NtQueryInformationFile (get_handle (), &io,
				       &fpli, sizeof (fpli),
				       FilePipeLocalInformation);
      if (NT_SUCCESS (status))
	{
	if (fpli.ReadDataAvailable == 0 && nbytes != 0)
	  break;
	}
      else if (nbytes != 0)
	break;
      status = NtReadFile (get_handle (), evt, NULL, NULL, &io, ptr,
			   len1, NULL, NULL);
      if (evt && status == STATUS_PENDING)
	{
	  waitret = cygwait (evt, INFINITE, cw_cancel | cw_sig);
	  /* If io.Status is STATUS_CANCELLED after CancelIo, IO has actually
	     been cancelled and io.Information contains the number of bytes
	     processed so far.
	     Otherwise IO has been finished regulary and io.Status contains
	     valid success or error information. */
	  CancelIo (get_handle ());
	  if (waitret == WAIT_SIGNALED && io.Status != STATUS_CANCELLED)
	    waitret = WAIT_OBJECT_0;

	  if (waitret == WAIT_CANCELED)
	    status = STATUS_THREAD_CANCELED;
	  else if (waitret == WAIT_SIGNALED)
	    status = STATUS_THREAD_SIGNALED;
	  else
	    status = io.Status;
	}
      if (isclosed ())  /* A signal handler might have closed the fd. */
	{
	  if (waitret == WAIT_OBJECT_0)
	    set_errno (EBADF);
	  else
	    __seterrno ();
	  nbytes = (size_t) -1;
	}
      else if (NT_SUCCESS (status)
	       || status == STATUS_BUFFER_OVERFLOW
	       || status == STATUS_THREAD_CANCELED
	       || status == STATUS_THREAD_SIGNALED)
	{
	  nbytes_now = io.Information;
	  ptr = ((char *) ptr) + nbytes_now;
	  nbytes += nbytes_now;
	  if (select_sem && nbytes_now > 0)
	    release_select_sem ("raw_read");
	}
      else
	{
	  /* Some errors are not really errors.  Detect such cases here.  */
	  switch (status)
	    {
	    case STATUS_END_OF_FILE:
	    case STATUS_PIPE_BROKEN:
	      /* This is really EOF.  */
	      break;
	    case STATUS_PIPE_LISTENING:
	    case STATUS_PIPE_EMPTY:
	      if (nbytes != 0)
		break;
	      if (is_nonblocking ())
		{
		  set_errno (EAGAIN);
		  nbytes = (size_t) -1;
		  break;
		}
	      fallthrough;
	    default:
	      __seterrno_from_nt_status (status);
	      nbytes = (size_t) -1;
	      break;
	    }
	}

      if (nbytes_now == 0 || status == STATUS_BUFFER_OVERFLOW)
	break;
    }
  ReleaseMutex (read_mtx);
  if (evt)
    CloseHandle (evt);
  if (status == STATUS_THREAD_SIGNALED && nbytes == 0)
    {
      set_errno (EINTR);
      nbytes = (size_t) -1;
    }
  else if (status == STATUS_THREAD_CANCELED)
    pthread::static_cancel_self ();
  len = nbytes;
}

bool
fhandler_pipe::reader_closed ()
{
  if (!query_hdl)
    return false;
  WaitForSingleObject (hdl_cnt_mtx, INFINITE);
  int n_reader = get_obj_handle_count (query_hdl);
  int n_writer = get_obj_handle_count (get_handle ());
  ReleaseMutex (hdl_cnt_mtx);
  return n_reader == n_writer;
}

ssize_t __reg3
fhandler_pipe_fifo::raw_write (const void *ptr, size_t len)
{
  size_t nbytes = 0;
  ULONG chunk;
  NTSTATUS status = STATUS_SUCCESS;
  IO_STATUS_BLOCK io;
  HANDLE evt = NULL;

  if (!len)
    return 0;

  if (reader_closed ())
    {
      set_errno (EPIPE);
      raise (SIGPIPE);
      return -1;
    }

  if (len <= pipe_buf_size)
    chunk = len;
  else if (is_nonblocking ())
    chunk = len = pipe_buf_size;
  else
    chunk = pipe_buf_size;

  /* Create a wait event if the pipe or fifo is in blocking mode. */
  if (!is_nonblocking () && !(evt = CreateEvent (NULL, false, false, NULL)))
    {
      __seterrno ();
      return -1;
    }

  /* Write in chunks, accumulating a total.  If there's an error, just
     return the accumulated total unless the first write fails, in
     which case return -1. */
  while (nbytes < len)
    {
      ULONG_PTR nbytes_now = 0;
      size_t left = len - nbytes;
      ULONG len1;
      DWORD waitret = WAIT_OBJECT_0;

      if (left > chunk)
	len1 = chunk;
      else
	len1 = (ULONG) left;
      /* NtWriteFile returns success with # of bytes written == 0 if writing
         on a non-blocking pipe fails because the pipe buffer doesn't have
	 sufficient space.

	 POSIX requires
	 - A write request for {PIPE_BUF} or fewer bytes shall have the
	   following effect: if there is sufficient space available in the
	   pipe, write() shall transfer all the data and return the number
	   of bytes requested. Otherwise, write() shall transfer no data and
	   return -1 with errno set to [EAGAIN].

	 - A write request for more than {PIPE_BUF} bytes shall cause one
	   of the following:

	  - When at least one byte can be written, transfer what it can and
	    return the number of bytes written. When all data previously
	    written to the pipe is read, it shall transfer at least {PIPE_BUF}
	    bytes.

	  - When no data can be written, transfer no data, and return -1 with
	    errno set to [EAGAIN]. */
      while (len1 > 0)
	{
	  status = NtWriteFile (get_handle (), evt, NULL, NULL, &io,
				(PVOID) ptr, len1, NULL, NULL);
	  if (evt || !NT_SUCCESS (status) || io.Information > 0
	      || len <= PIPE_BUF)
	    break;
	  len1 >>= 1;
	}
      if (evt && status == STATUS_PENDING)
	{
	  while (WAIT_TIMEOUT ==
		 (waitret = cygwait (evt, (DWORD) 0, cw_cancel | cw_sig)))
	    {
	      if (reader_closed ())
		{
		  CancelIo (get_handle ());
		  set_errno (EPIPE);
		  raise (SIGPIPE);
		  goto out;
		}
	      else
		cygwait (select_sem, 10);
	    }
	  /* If io.Status is STATUS_CANCELLED after CancelIo, IO has actually
	     been cancelled and io.Information contains the number of bytes
	     processed so far.
	     Otherwise IO has been finished regulary and io.Status contains
	     valid success or error information. */
	  CancelIo (get_handle ());
	  if (waitret == WAIT_SIGNALED && io.Status != STATUS_CANCELLED)
	    waitret = WAIT_OBJECT_0;

	  if (waitret == WAIT_CANCELED)
	    status = STATUS_THREAD_CANCELED;
	  else if (waitret == WAIT_SIGNALED)
	    status = STATUS_THREAD_SIGNALED;
	  else
	    status = io.Status;
	}
      if (isclosed ())  /* A signal handler might have closed the fd. */
	{
	  if (waitret == WAIT_OBJECT_0)
	    set_errno (EBADF);
	  else
	    __seterrno ();
	}
      else if (NT_SUCCESS (status)
	       || status == STATUS_THREAD_CANCELED
	       || status == STATUS_THREAD_SIGNALED)
	{
	  nbytes_now = io.Information;
	  ptr = ((char *) ptr) + nbytes_now;
	  nbytes += nbytes_now;
	  if (select_sem && nbytes_now > 0)
	    release_select_sem ("raw_write");
	  /* 0 bytes returned?  EAGAIN.  See above. */
	  if (NT_SUCCESS (status) && nbytes == 0)
	    set_errno (EAGAIN);
	}
      else if (STATUS_PIPE_IS_CLOSED (status))
	{
	  set_errno (EPIPE);
	  raise (SIGPIPE);
	}
      else
	__seterrno_from_nt_status (status);

      if (nbytes_now == 0)
	break;
    }
out:
  if (evt)
    CloseHandle (evt);
  if (status == STATUS_THREAD_SIGNALED && nbytes == 0)
    set_errno (EINTR);
  else if (status == STATUS_THREAD_CANCELED)
    pthread::static_cancel_self ();
  return nbytes ?: -1;
}

void
fhandler_pipe::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  if (read_mtx)
    set_no_inheritance (read_mtx, val);
  if (select_sem)
    set_no_inheritance (select_sem, val);
  if (query_hdl)
    set_no_inheritance (query_hdl, val);
  set_no_inheritance (hdl_cnt_mtx, val);
}

void
fhandler_pipe::fixup_after_fork (HANDLE parent)
{
  fork_fixup (parent, hdl_cnt_mtx, "hdl_cnt_mtx");
  WaitForSingleObject (hdl_cnt_mtx, INFINITE);
  if (read_mtx)
    fork_fixup (parent, read_mtx, "read_mtx");
  if (select_sem)
    fork_fixup (parent, select_sem, "select_sem");
  if (query_hdl)
    fork_fixup (parent, query_hdl, "query_hdl");

  fhandler_base::fixup_after_fork (parent);
  ReleaseMutex (hdl_cnt_mtx);
}

int
fhandler_pipe::dup (fhandler_base *child, int flags)
{
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->set_popen_pid (0);

  int res = 0;
  WaitForSingleObject (hdl_cnt_mtx, INFINITE);
  if (fhandler_base::dup (child, flags))
    res = -1;
  else if (read_mtx &&
	   !DuplicateHandle (GetCurrentProcess (), read_mtx,
			     GetCurrentProcess (), &ftp->read_mtx,
			     0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      ftp->close ();
      res = -1;
    }
  else if (select_sem &&
	   !DuplicateHandle (GetCurrentProcess (), select_sem,
			     GetCurrentProcess (), &ftp->select_sem,
			     0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      ftp->close ();
      res = -1;
    }
  else if (query_hdl &&
	   !DuplicateHandle (GetCurrentProcess (), query_hdl,
			     GetCurrentProcess (), &ftp->query_hdl,
			     0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      ftp->close ();
      res = -1;
    }
  else if (!DuplicateHandle (GetCurrentProcess (), hdl_cnt_mtx,
			     GetCurrentProcess (), &ftp->hdl_cnt_mtx,
			     0, !(flags & O_CLOEXEC), DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      ftp->close ();
      res = -1;
    }
  ReleaseMutex (hdl_cnt_mtx);

  debug_printf ("res %d", res);
  return res;
}

int
fhandler_pipe::close ()
{
  if (select_sem)
    {
      release_select_sem ("close");
      CloseHandle (select_sem);
    }
  if (read_mtx)
    CloseHandle (read_mtx);
  WaitForSingleObject (hdl_cnt_mtx, INFINITE);
  if (query_hdl)
    CloseHandle (query_hdl);
  int ret = fhandler_base::close ();
  ReleaseMutex (hdl_cnt_mtx);
  CloseHandle (hdl_cnt_mtx);
  if (query_hdl_proc)
    CloseHandle (query_hdl_proc);
  return ret;
}

#define PIPE_INTRO "\\\\.\\pipe\\cygwin-"

/* Create a pipe, and return handles to the read and write ends,
   just like CreatePipe, but ensure that the write end permits
   FILE_READ_ATTRIBUTES access, on later versions of win32 where
   this is supported.  This access is needed by NtQueryInformationFile,
   which is used to implement select and nonblocking writes.
   Note that the return value is either 0 or GetLastError,
   unlike CreatePipe, which returns a bool for success or failure.  */
DWORD
fhandler_pipe::create (LPSECURITY_ATTRIBUTES sa_ptr, PHANDLE r, PHANDLE w,
		       DWORD psize, const char *name, DWORD open_mode,
		       int64_t *unique_id)
{
  /* Default to error. */
  if (r)
    *r = NULL;
  if (w)
    *w = NULL;

  /* Ensure that there is enough pipe buffer space for atomic writes.  */
  if (!psize)
    psize = DEFAULT_PIPEBUFSIZE;

  char pipename[MAX_PATH];
  size_t len = __small_sprintf (pipename, PIPE_INTRO "%S-",
				      &cygheap->installation_key);
  DWORD pipe_mode = PIPE_READMODE_BYTE | PIPE_REJECT_REMOTE_CLIENTS;
  if (!name)
    pipe_mode |= pipe_byte ? PIPE_TYPE_BYTE : PIPE_TYPE_MESSAGE;
  else
    pipe_mode |= PIPE_TYPE_MESSAGE;

  if (!name || (open_mode & PIPE_ADD_PID))
    {
      len += __small_sprintf (pipename + len, "%u-", GetCurrentProcessId ());
      open_mode &= ~PIPE_ADD_PID;
    }

  if (name)
    len += __small_sprintf (pipename + len, "%s", name);

  open_mode |= PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE;

  /* Retry CreateNamedPipe as long as the pipe name is in use.
     Retrying will probably never be necessary, but we want
     to be as robust as possible.  */
  DWORD err = 0;
  while (r && !*r)
    {
      static volatile ULONG pipe_unique_id;
      if (!name)
	{
	  LONG id = InterlockedIncrement ((LONG *) &pipe_unique_id);
	  __small_sprintf (pipename + len, "pipe-%p", id);
	  if (unique_id)
	    *unique_id = ((int64_t) id << 32 | GetCurrentProcessId ());
	}

      debug_printf ("name %s, size %u, mode %s", pipename, psize,
		    (pipe_mode & PIPE_TYPE_MESSAGE)
		    ? "PIPE_TYPE_MESSAGE" : "PIPE_TYPE_BYTE");

      /* Use CreateNamedPipe instead of CreatePipe, because the latter
	 returns a write handle that does not permit FILE_READ_ATTRIBUTES
	 access, on versions of win32 earlier than WinXP SP2.
	 CreatePipe also stupidly creates a full duplex pipe, which is
	 a waste, since only a single direction is actually used.
	 It's important to only allow a single instance, to ensure that
	 the pipe was not created earlier by some other process, even if
	 the pid has been reused.

	 Note that the write side of the pipe is opened as PIPE_TYPE_MESSAGE.
	 This *seems* to more closely mimic Linux pipe behavior and is
	 definitely required for pty handling since fhandler_pty_master
	 writes to the pipe in chunks, terminated by newline when CANON mode
	 is specified.  */
      *r = CreateNamedPipe (pipename, open_mode, pipe_mode, 1, psize,
			   psize, NMPWAIT_USE_DEFAULT_WAIT, sa_ptr);

      if (*r != INVALID_HANDLE_VALUE)
	{
	  debug_printf ("pipe read handle %p", *r);
	  err = 0;
	  break;
	}

      err = GetLastError ();
      switch (err)
	{
	case ERROR_PIPE_BUSY:
	  /* The pipe is already open with compatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe busy", !name ? ", retrying" : "");
	  if (!name)
	    *r = NULL;
	  break;
	case ERROR_ACCESS_DENIED:
	  /* The pipe is already open with incompatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe access denied%s", !name ? ", retrying" : "");
	  if (!name)
	    *r = NULL;
	  break;
	default:
	  {
	    err = GetLastError ();
	    debug_printf ("failed, %E");
	  }
	}
    }

  if (err)
    {
      *r = NULL;
      return err;
    }

  if (!w)
    debug_printf ("pipe write handle NULL");
  else
    {
      debug_printf ("CreateFile: name %s", pipename);

      /* Open the named pipe for writing.
	 Be sure to permit FILE_READ_ATTRIBUTES access.  */
      DWORD access = GENERIC_WRITE | FILE_READ_ATTRIBUTES;
      if ((open_mode & PIPE_ACCESS_DUPLEX) == PIPE_ACCESS_DUPLEX)
	access |= GENERIC_READ | FILE_WRITE_ATTRIBUTES;
      *w = CreateFile (pipename, access, 0, sa_ptr, OPEN_EXISTING,
		      open_mode & FILE_FLAG_OVERLAPPED, 0);

      if (!*w || *w == INVALID_HANDLE_VALUE)
	{
	  /* Failure. */
	  DWORD err = GetLastError ();
	  debug_printf ("CreateFile failed, r %p, %E", r);
	  if (r)
	    CloseHandle (*r);
	  *w = NULL;
	  return err;
	}

      debug_printf ("pipe write handle %p", *w);
    }

  /* Success. */
  return 0;
}

inline static bool
is_running_as_service (void)
{
  return check_token_membership (well_known_service_sid)
    || cygheap->user.saved_sid () == well_known_system_sid;
}

/* The next version of fhandler_pipe::create used to call the previous
   version.  But the read handle created by the latter doesn't have
   FILE_WRITE_ATTRIBUTES access unless the pipe is created with
   PIPE_ACCESS_DUPLEX, and it doesn't seem possible to add that access
   right.  This causes set_pipe_non_blocking to fail.

   To fix this we will define a helper function 'nt_create' that is
   similar to the above fhandler_pipe::create but uses
   NtCreateNamedPipeFile instead of CreateNamedPipe; this gives more
   flexibility in setting the access rights.  We will use this helper
   function in the version of fhandler_pipe::create below, which
   suffices for all of our uses of set_pipe_non_blocking.  For
   simplicity, nt_create will omit the 'open_mode' and 'name'
   parameters, which aren't needed for our purposes.  */

static int nt_create (LPSECURITY_ATTRIBUTES, HANDLE &, HANDLE &, DWORD,
		      int64_t *);

int
fhandler_pipe::create (fhandler_pipe *fhs[2], unsigned psize, int mode)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES *sa = sec_none_cloexec (mode);
  int res = -1;
  int64_t unique_id;

  int ret = nt_create (sa, r, w, psize, &unique_id);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      goto out;
    }
  if ((fhs[0] = (fhandler_pipe *) build_fh_dev (*piper_dev)) == NULL)
    goto err_close_rw_handle;
  if ((fhs[1] = (fhandler_pipe *) build_fh_dev (*pipew_dev)) == NULL)
    goto err_delete_fhs0;
  mode |= mode & O_TEXT ?: O_BINARY;
  fhs[0]->init (r, FILE_CREATE_PIPE_INSTANCE | GENERIC_READ, mode, unique_id);
  fhs[1]->init (w, FILE_CREATE_PIPE_INSTANCE | GENERIC_WRITE, mode, unique_id);

  /* For the read side of the pipe, add a mutex.  See raw_read for the
     usage. */
  fhs[0]->read_mtx = CreateMutexW (sa, FALSE, NULL);
  if (!fhs[0]->read_mtx)
    goto err_delete_fhs1;

  fhs[0]->select_sem = CreateSemaphore (sa, 0, INT32_MAX, NULL);
  if (!fhs[0]->select_sem)
    goto err_close_read_mtx;
  if (!DuplicateHandle (GetCurrentProcess (), fhs[0]->select_sem,
			GetCurrentProcess (), &fhs[1]->select_sem,
			0, sa->bInheritHandle, DUPLICATE_SAME_ACCESS))
    goto err_close_select_sem0;

  if (is_running_as_service () &&
      !DuplicateHandle (GetCurrentProcess (), r,
			GetCurrentProcess (), &fhs[1]->query_hdl,
			FILE_READ_DATA, sa->bInheritHandle, 0))
    goto err_close_select_sem1;

  fhs[0]->hdl_cnt_mtx = CreateMutexW (sa, FALSE, NULL);
  if (!fhs[0]->hdl_cnt_mtx)
    goto err_close_query_hdl;
  if (!DuplicateHandle (GetCurrentProcess (), fhs[0]->hdl_cnt_mtx,
			GetCurrentProcess (), &fhs[1]->hdl_cnt_mtx,
			0, sa->bInheritHandle, DUPLICATE_SAME_ACCESS))
    goto err_close_hdl_cnt_mtx0;

  res = 0;
  goto out;

err_close_hdl_cnt_mtx0:
  CloseHandle (fhs[0]->hdl_cnt_mtx);
err_close_query_hdl:
  if (fhs[1]->query_hdl)
    CloseHandle (fhs[1]->query_hdl);
err_close_select_sem1:
  CloseHandle (fhs[1]->select_sem);
err_close_select_sem0:
  CloseHandle (fhs[0]->select_sem);
err_close_read_mtx:
  CloseHandle (fhs[0]->read_mtx);
err_delete_fhs1:
  delete fhs[1];
err_delete_fhs0:
  delete fhs[0];
err_close_rw_handle:
  NtClose (r);
  NtClose (w);
out:
  debug_printf ("%R = pipe([%p, %p], %d, %y)",
		res, fhs[0], fhs[1], psize, mode);
  return res;
}

static int
nt_create (LPSECURITY_ATTRIBUTES sa_ptr, HANDLE &r, HANDLE &w,
		DWORD psize, int64_t *unique_id)
{
  NTSTATUS status;
  HANDLE npfsh;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  LARGE_INTEGER timeout;

  /* Default to error. */
  r = NULL;
  w = NULL;

  status = fhandler_base::npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return GetLastError ();
    }

  /* Ensure that there is enough pipe buffer space for atomic writes.  */
  if (!psize)
    psize = DEFAULT_PIPEBUFSIZE;

  UNICODE_STRING pipename;
  WCHAR pipename_buf[MAX_PATH];
  size_t len = __small_swprintf (pipename_buf, L"%S-%u-",
				 &cygheap->installation_key,
				 GetCurrentProcessId ());

  access = GENERIC_READ | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE;
  access |= FILE_WRITE_EA; /* Add this right as a marker of cygwin read pipe */

  ULONG pipe_type = pipe_byte ? FILE_PIPE_BYTE_STREAM_TYPE
    : FILE_PIPE_MESSAGE_TYPE;

  /* Retry NtCreateNamedPipeFile as long as the pipe name is in use.
     Retrying will probably never be necessary, but we want
     to be as robust as possible.  */
  DWORD err = 0;
  while (!r)
    {
      static volatile ULONG pipe_unique_id;
      LONG id = InterlockedIncrement ((LONG *) &pipe_unique_id);
      __small_swprintf (pipename_buf + len, L"pipe-nt-%p", id);
      if (unique_id)
	*unique_id = ((int64_t) id << 32 | GetCurrentProcessId ());

      debug_printf ("name %W, size %u, mode %s", pipename_buf, psize,
		    (pipe_type & FILE_PIPE_MESSAGE_TYPE)
		    ? "PIPE_TYPE_MESSAGE" : "PIPE_TYPE_BYTE");

      RtlInitUnicodeString (&pipename, pipename_buf);

      InitializeObjectAttributes (&attr, &pipename,
				  sa_ptr->bInheritHandle ? OBJ_INHERIT : 0,
				  npfsh, sa_ptr->lpSecurityDescriptor);

      timeout.QuadPart = -500000;
      status = NtCreateNamedPipeFile (&r, access, &attr, &io,
				      FILE_SHARE_READ | FILE_SHARE_WRITE,
				      FILE_CREATE, 0, pipe_type,
				      FILE_PIPE_BYTE_STREAM_MODE,
				      0, 1, psize, psize, &timeout);

      if (NT_SUCCESS (status))
	{
	  debug_printf ("pipe read handle %p", r);
	  err = 0;
	  break;
	}

      switch (status)
	{
	case STATUS_PIPE_BUSY:
	case STATUS_INSTANCE_NOT_AVAILABLE:
	case STATUS_PIPE_NOT_AVAILABLE:
	  /* The pipe is already open with compatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe busy, retrying");
	  r = NULL;
	  break;
	case STATUS_ACCESS_DENIED:
	  /* The pipe is already open with incompatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe access denied, retrying");
	  r = NULL;
	  break;
	default:
	  {
	    __seterrno_from_nt_status (status);
	    err = GetLastError ();
	    debug_printf ("failed, %E");
	    r = NULL;
	  }
	}
    }

  if (err)
    {
      r = NULL;
      return err;
    }

  debug_printf ("NtOpenFile: name %S", &pipename);

  access = GENERIC_WRITE | FILE_READ_ATTRIBUTES | SYNCHRONIZE;
  status = NtOpenFile (&w, access, &attr, &io, 0, 0);
  if (!NT_SUCCESS (status))
    {
      DWORD err = GetLastError ();
      debug_printf ("NtOpenFile failed, r %p, %E", r);
      if (r)
	NtClose (r);
      w = NULL;
      return err;
    }

  /* Success. */
  return 0;
}

/* Called by dtable::init_std_file_from_handle for stdio handles
   inherited from non-Cygwin processes. */
void
fhandler_pipe::set_pipe_buf_size ()
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_PIPE_LOCAL_INFORMATION fpli;

  status = NtQueryInformationFile (get_handle (), &io, &fpli, sizeof fpli,
				   FilePipeLocalInformation);
  if (NT_SUCCESS (status))
    pipe_buf_size = fpli.InboundQuota;
}

int
fhandler_pipe::ioctl (unsigned int cmd, void *p)
{
  int n;

  switch (cmd)
    {
    case FIONREAD:
      if (get_device () == FH_PIPEW)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      if (!PeekNamedPipe (get_handle (), NULL, 0, NULL, (DWORD *) &n, NULL))
	{
	  __seterrno ();
	  return -1;
	}
      break;
    default:
      return fhandler_base::ioctl (cmd, p);
      break;
    }
  *(int *) p = n;
  return 0;
}

int
fhandler_pipe::fcntl (int cmd, intptr_t arg)
{
  if (cmd != F_SETFL)
    return fhandler_base::fcntl (cmd, arg);

  const bool was_nonblocking = is_nonblocking ();
  int res = fhandler_base::fcntl (cmd, arg);
  const bool now_nonblocking = is_nonblocking ();
  if (now_nonblocking != was_nonblocking)
    set_pipe_non_blocking (now_nonblocking);
  return res;
}

int __reg2
fhandler_pipe::fstat (struct stat *buf)
{
  int ret = fhandler_base::fstat (buf);
  if (!ret)
    {
      buf->st_dev = FH_PIPE;
      if (!(buf->st_ino = get_plain_ino ()))
	sscanf (get_name (), "/proc/%*d/fd/pipe:[%llu]",
			     (long long *) &buf->st_ino);
    }
  return ret;
}

int __reg2
fhandler_pipe::fstatvfs (struct statvfs *sfs)
{
  set_errno (EBADF);
  return -1;
}

HANDLE
fhandler_pipe::temporary_query_hdl ()
{
  if (get_dev () != FH_PIPEW)
    return NULL;

  ULONG len;
  NTSTATUS status;
  tmp_pathbuf tp;
  OBJECT_NAME_INFORMATION *ntfn = (OBJECT_NAME_INFORMATION *) tp.w_get ();

  /* Try process handle opened and pipe handle value cached first
     in order to reduce overhead. */
  if (query_hdl_proc && query_hdl_value)
    {
      HANDLE h;
      if (!DuplicateHandle (query_hdl_proc, query_hdl_value,
			   GetCurrentProcess (), &h, FILE_READ_DATA, 0, 0))
	goto cache_err;
      /* Check name */
      status = NtQueryObject (h, ObjectNameInformation, ntfn, 65536, &len);
      if (!NT_SUCCESS (status) || !ntfn->Name.Buffer)
	goto hdl_err;
      ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';
      uint64_t key;
      DWORD pid;
      LONG id;
      if (swscanf (ntfn->Name.Buffer,
		   L"\\Device\\NamedPipe\\%llx-%u-pipe-nt-0x%x",
		   &key, &pid, &id) == 3 &&
	  key == pipename_key && pid == pipename_pid && id == pipename_id)
	return h;
hdl_err:
      CloseHandle (h);
cache_err:
      CloseHandle (query_hdl_proc);
      query_hdl_proc = NULL;
      query_hdl_value = NULL;
    }

  status = NtQueryObject (get_handle (), ObjectNameInformation, ntfn,
			  65536, &len);
  if (!NT_SUCCESS (status) || !ntfn->Name.Buffer)
    return NULL; /* Non cygwin pipe? */
  WCHAR name[MAX_PATH];
  int namelen = min (ntfn->Name.Length / sizeof (WCHAR), MAX_PATH-1);
  memcpy (name, ntfn->Name.Buffer, namelen * sizeof (WCHAR));
  name[namelen] = L'\0';
  if (swscanf (name, L"\\Device\\NamedPipe\\%llx-%u-pipe-nt-0x%x",
	       &pipename_key, &pipename_pid, &pipename_id) != 3)
    return NULL; /* Non cygwin pipe? */

  if (wincap.has_query_process_handle_info ())
    return get_query_hdl_per_process (name, ntfn); /* Since Win8 */
  else
    return get_query_hdl_per_system (name, ntfn); /* Vista or Win7 */
}

/* This function is faster than get_query_hdl_per_system(), however,
   only works since Windows 8 because ProcessHandleInformation is not
   suppoted by NtQueryInformationProcess() before Windows 8. */
HANDLE
fhandler_pipe::get_query_hdl_per_process (WCHAR *name,
					  OBJECT_NAME_INFORMATION *ntfn)
{
  ULONG len;
  BOOL res;
  DWORD n_process = 256;
  DWORD *proc_pids;
  do
    { /* Enumerate processes */
      DWORD nbytes = n_process * sizeof (DWORD);
      proc_pids = (DWORD *) HeapAlloc (GetProcessHeap (), 0, nbytes);
      if (!proc_pids)
	return NULL;
      res = EnumProcesses (proc_pids, nbytes, &len);
      if (res && len < nbytes)
	break;
      res = FALSE;
      HeapFree (GetProcessHeap (), 0, proc_pids);
      n_process *= 2;
    }
  while (n_process < (1L<<20));
  if (!res)
    return NULL;
  n_process = len / sizeof (DWORD);

  for (LONG i = (LONG) n_process - 1; i >= 0; i--)
    {
      HANDLE proc = OpenProcess (PROCESS_DUP_HANDLE
				 | PROCESS_QUERY_INFORMATION,
				 0, proc_pids[i]);
      if (!proc)
	continue;

      /* Retrieve process handles */
      NTSTATUS status;
      DWORD n_handle = 256;
      PPROCESS_HANDLE_SNAPSHOT_INFORMATION phi;
      do
	{
	  DWORD nbytes = 2 * sizeof (ULONG_PTR) +
	    n_handle * sizeof (PROCESS_HANDLE_TABLE_ENTRY_INFO);
	  phi = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)
	    HeapAlloc (GetProcessHeap (), 0, nbytes);
	  if (!phi)
	    goto close_proc;
	  status = NtQueryInformationProcess (proc, ProcessHandleInformation,
					      phi, nbytes, &len);
	  if (NT_SUCCESS (status))
	    break;
	  HeapFree (GetProcessHeap (), 0, phi);
	  n_handle *= 2;
	}
      while (n_handle < (1L<<20) && status == STATUS_INFO_LENGTH_MISMATCH);
      if (!NT_SUCCESS (status))
	goto close_proc;

      for (ULONG j = 0; j < phi->NumberOfHandles; j++)
	{
	  /* Check for the peculiarity of cygwin read pipe */
	  const ULONG access = FILE_READ_DATA | FILE_READ_EA
	    | FILE_WRITE_EA /* marker */
	    | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES
	    | READ_CONTROL | SYNCHRONIZE;
	  if (phi->Handles[j].GrantedAccess != access)
	    continue;

	  /* Retrieve handle */
	  HANDLE h = (HANDLE)(intptr_t) phi->Handles[j].HandleValue;
	  res = DuplicateHandle (proc, h, GetCurrentProcess (), &h,
				 FILE_READ_DATA, 0, 0);
	  if (!res)
	    continue;

	  /* Check object name */
	  status = NtQueryObject (h, ObjectNameInformation,
				  ntfn, 65536, &len);
	  if (!NT_SUCCESS (status) || !ntfn->Name.Buffer)
	    goto close_handle;
	  ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';
	  if (wcscmp (name, ntfn->Name.Buffer) == 0)
	    {
	      query_hdl_proc = proc;
	      query_hdl_value = (HANDLE)(intptr_t) phi->Handles[j].HandleValue;
	      HeapFree (GetProcessHeap (), 0, phi);
	      HeapFree (GetProcessHeap (), 0, proc_pids);
	      return h;
	    }
close_handle:
	  CloseHandle (h);
	}
      HeapFree (GetProcessHeap (), 0, phi);
close_proc:
      CloseHandle (proc);
    }
  HeapFree (GetProcessHeap (), 0, proc_pids);
  return NULL;
}

/* This function is slower than get_query_hdl_per_process(), however,
   works even before Windows 8. */
HANDLE
fhandler_pipe::get_query_hdl_per_system (WCHAR *name,
					 OBJECT_NAME_INFORMATION *ntfn)
{
  NTSTATUS status;
  SIZE_T n_handle = 65536;
  PSYSTEM_HANDLE_INFORMATION shi;
  do
    { /* Enumerate handles */
      SIZE_T nbytes =
	sizeof (ULONG) + n_handle * sizeof (SYSTEM_HANDLE_TABLE_ENTRY_INFO);
      shi = (PSYSTEM_HANDLE_INFORMATION) HeapAlloc (GetProcessHeap (),
						    0, nbytes);
      if (!shi)
	return NULL;
      status = NtQuerySystemInformation (SystemHandleInformation,
					 shi, nbytes, NULL);
      if (NT_SUCCESS (status))
	break;
      HeapFree (GetProcessHeap (), 0, shi);
      n_handle *= 2;
    }
  while (n_handle < (1L<<23) && status == STATUS_INFO_LENGTH_MISMATCH);
  if (!NT_SUCCESS (status))
    return NULL;

  for (LONG i = (LONG) shi->NumberOfHandles - 1; i >= 0; i--)
    {
      /* Check for the peculiarity of cygwin read pipe */
      const ULONG access = FILE_READ_DATA | FILE_READ_EA
	| FILE_WRITE_EA /* marker */
	| FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES
	| READ_CONTROL | SYNCHRONIZE;
      if (shi->Handles[i].GrantedAccess != access)
	continue;

      /* Retrieve handle */
      HANDLE proc = OpenProcess (PROCESS_DUP_HANDLE, 0,
				 shi->Handles[i].UniqueProcessId);
      if (!proc)
	continue;
      HANDLE h = (HANDLE)(intptr_t) shi->Handles[i].HandleValue;
      BOOL res  = DuplicateHandle (proc, h, GetCurrentProcess (), &h,
				   FILE_READ_DATA, 0, 0);
      if (!res)
	goto close_proc;

      /* Check object name */
      ULONG len;
      status = NtQueryObject (h, ObjectNameInformation, ntfn, 65536, &len);
      if (!NT_SUCCESS (status) || !ntfn->Name.Buffer)
	goto close_handle;
      ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';
      if (wcscmp (name, ntfn->Name.Buffer) == 0)
	{
	  query_hdl_proc = proc;
	  query_hdl_value = (HANDLE)(intptr_t) shi->Handles[i].HandleValue;
	  HeapFree (GetProcessHeap (), 0, shi);
	  return h;
	}
close_handle:
      CloseHandle (h);
close_proc:
      CloseHandle (proc);
    }
  HeapFree (GetProcessHeap (), 0, shi);
  return NULL;
}
