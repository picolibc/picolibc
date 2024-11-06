/* fhandler_pipe.cc: pipes for Cygwin.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "select.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "shared_info.h"
#include "tls_pbuf.h"
#include "sigproc.h"
#include <assert.h>

/* This is only to be used for writing.  When reading,
STATUS_PIPE_EMPTY simply means there's no data to be read. */
#define STATUS_PIPE_IS_CLOSED(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_PIPE_CLOSING \
		   || _s == STATUS_PIPE_BROKEN \
		   || _s == STATUS_PIPE_EMPTY; })

fhandler_pipe_fifo::fhandler_pipe_fifo ()
  : fhandler_base (), pipe_buf_size (DEFAULT_PIPEBUFSIZE), pipe_mtx (NULL)
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
NTSTATUS
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
  return status;
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
    /* Set pipe always blocking and simulate non-blocking mode in
       raw_read()/raw_write() to allow signal handling even with
       FILE_SYNCHRONOUS_IO_NONALERT. */
    set_pipe_non_blocking (false);

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

extern "C" int swscanf (const wchar_t *, const wchar_t *, ...);

static char *
get_mtx_name (HANDLE h, const char *io, char *name)
{
  ULONG len;
  NTSTATUS status;
  tmp_pathbuf tp;
  OBJECT_NAME_INFORMATION *ntfn = (OBJECT_NAME_INFORMATION *) tp.w_get ();
  DWORD pid;
  LONG uniq_id;

  status = NtQueryObject (h, ObjectNameInformation, ntfn, 65536, &len);
  if (!NT_SUCCESS (status) || !ntfn->Name.Buffer)
    return NULL;
  ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = L'\0';
  WCHAR *p = wcschr (ntfn->Name.Buffer, L'-');
  if (p == NULL)
    return NULL;
  if (2 != swscanf (p, L"-%u-pipe-nt-0x%x", &pid, &uniq_id))
    return NULL;
  __small_sprintf (name, "cygpipe.%s.mutex.%u-%p",
		   io, pid, (intptr_t) uniq_id);
  return name;
}

bool
fhandler_pipe::open_setup (int flags)
{
  if (!fhandler_base::open_setup (flags))
    return false;
  if (!pipe_mtx)
    {
      SECURITY_ATTRIBUTES *sa = sec_none_cloexec (flags);
      char name[MAX_PATH];
      const char *io = get_device () == FH_PIPER ? "input" : "output";
      char *mtx_name = get_mtx_name (get_handle (), io, name);
      pipe_mtx = CreateMutex (sa, FALSE, mtx_name);
      if (!pipe_mtx)
	{
	  debug_printf ("CreateMutex pipe_mtx failed: %E");
	  return false;
	}
    }
  return true;
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
fhandler_pipe::fallocate (int mode, off_t offset, off_t length)
{
  return (mode & __FALLOC_FL_TRUNCATE) ? EINVAL : ESPIPE;
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
  LONG n_release = get_obj_handle_count (select_sem)
      - get_obj_handle_count (get_handle ());
  debug_printf("%s(%s) release %d", from,
	       get_dev () == FH_PIPER ? "PIPER" : "PIPEW", n_release);
  if (n_release)
    ReleaseSemaphore (select_sem, n_release, NULL);
}

void
fhandler_pipe::raw_read (void *ptr, size_t& len)
{
  size_t nbytes = 0;
  NTSTATUS status = STATUS_SUCCESS;
  IO_STATUS_BLOCK io;
  ULONGLONG t0 = GetTickCount64 (); /* Init timer */
  const ULONGLONG t0_threshold = 20;

  if (!len)
    return;

  DWORD timeout = is_nonblocking () ? 0 : INFINITE;
  DWORD waitret = cygwait (pipe_mtx, timeout);
  switch (waitret)
    {
    case WAIT_OBJECT_0:
      break;
    case WAIT_TIMEOUT:
      set_errno (EAGAIN);
      len = (size_t) -1;
      return;
    case WAIT_SIGNALED:
      set_errno (EINTR);
      len = (size_t) -1;
      return;
    case WAIT_CANCELED:
      pthread::static_cancel_self ();
      /* NOTREACHED */
    default:
      /* Should not reach here. */
      __seterrno ();
      len = (size_t) -1;
      return;
    }
  while (nbytes < len)
    {
      ULONG_PTR nbytes_now = 0;
      ULONG len1 = (ULONG) (len - nbytes);
      DWORD select_sem_timeout = 0;
      bool real_non_blocking_mode = false;

      FILE_PIPE_LOCAL_INFORMATION fpli;
      status = NtQueryInformationFile (get_handle (), &io,
				       &fpli, sizeof (fpli),
				       FilePipeLocalInformation);
      if (NT_SUCCESS (status))
	{
	  if (fpli.ReadDataAvailable == 0)
	    {
	      if (fpli.NamedPipeState == FILE_PIPE_CLOSING_STATE)
		/* EOF */
		break;
	      if (nbytes != 0)
		break;
	      if (is_nonblocking ())
		{
		  set_errno (EAGAIN);
		  nbytes = (size_t) -1;
		  break;
		}
	      /* If the pipe is a non-cygwin pipe, select_sem trick
		 does not work. As a result, the following cygwait()
		 will return only after timeout occurs. This causes
		 performance degradation. However, setting timeout
		 to zero causes high CPU load. So, set timeout to
		 non-zero only when select_sem is valid or pipe is
		 not ready to read for more than t0_threshold.
		 This prevents both the performance degradation and
		 the high CPU load. */
	      if (select_sem || GetTickCount64 () - t0 > t0_threshold)
		select_sem_timeout = 1;
	      waitret = cygwait (select_sem, select_sem_timeout);
	      if (waitret == WAIT_CANCELED)
		pthread::static_cancel_self ();
	      else if (waitret == WAIT_SIGNALED)
		{
		  set_errno (EINTR);
		  nbytes = (size_t) -1;
		  break;
		}
	      continue;
	    }
	}
      else if (nbytes != 0)
	break;
      else if (status == STATUS_END_OF_FILE || status == STATUS_PIPE_BROKEN)
	/* EOF */
	break;
      else if (!NT_SUCCESS (status) && is_nonblocking ())
	{
	  status = set_pipe_non_blocking (true);
	  if (status == STATUS_END_OF_FILE || status == STATUS_PIPE_BROKEN)
	    /* EOF */
	    break;
	  if (!NT_SUCCESS (status))
	    {
	      /* Cannot continue. What should we do? */
	      set_errno (EIO);
	      nbytes = (size_t) -1;
	      break;
	    }
	  real_non_blocking_mode = true;
	}
      status = NtReadFile (get_handle (), NULL, NULL, NULL, &io, ptr,
			   len1, NULL, NULL);
      if (real_non_blocking_mode)
	set_pipe_non_blocking (false);
      if (isclosed ())  /* A signal handler might have closed the fd. */
	{
	  set_errno (EBADF);
	  nbytes = (size_t) -1;
	}
      else if (NT_SUCCESS (status) || status == STATUS_BUFFER_OVERFLOW)
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
	      /* Only for real_non_blocking_mode */
	      if (nbytes != 0)
		break;
	      set_errno (EAGAIN);
	      nbytes = (size_t) -1;
	      break;
	    default:
	      __seterrno_from_nt_status (status);
	      nbytes = (size_t) -1;
	      break;
	    }
	}

      if ((nbytes_now == 0 && !NT_SUCCESS (status))
	  || status == STATUS_BUFFER_OVERFLOW)
	break;
    }
  ReleaseMutex (pipe_mtx);
  len = nbytes;
}

ssize_t
fhandler_pipe_fifo::raw_write (const void *ptr, size_t len)
{
  size_t nbytes = 0;
  ULONG chunk;
  NTSTATUS status = STATUS_SUCCESS;
  IO_STATUS_BLOCK io;
  HANDLE evt;
  bool short_write_once = false;

  if (!len)
    return 0;

  ssize_t avail = pipe_buf_size;
  bool real_non_blocking_mode = false;

  if (pipe_mtx) /* pipe_mtx is NULL in the fifo case */
    {
      DWORD timeout = is_nonblocking () ? 0 : INFINITE;
      DWORD waitret = cygwait (pipe_mtx, timeout);
      switch (waitret)
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_TIMEOUT:
	  set_errno (EAGAIN);
	  return -1;
	case WAIT_SIGNALED:
	  set_errno (EINTR);
	  return -1;
	case WAIT_CANCELED:
	  pthread::static_cancel_self ();
	  /* NOTREACHED */
	default:
	  /* Should not reach here. */
	  __seterrno ();
	  return -1;
	}
    }
  if (get_device () == FH_PIPEW && is_nonblocking ())
    {
      fhandler_pipe *fh = (fhandler_pipe *) this;
      FILE_PIPE_LOCAL_INFORMATION fpli;
      status = NtQueryInformationFile (get_handle (), &io, &fpli, sizeof fpli,
				       FilePipeLocalInformation);
      if (NT_SUCCESS (status))
	{
	  if (fpli.WriteQuotaAvailable != 0)
	    avail = fpli.WriteQuotaAvailable;
	  else /* WriteQuotaAvailable == 0 */
	    { /* Refer to the comment in select.cc: pipe_data_available(). */
	      /* NtSetInformationFile() in set_pipe_non_blocking(true) seems
		 to fail with STATUS_PIPE_BUSY if the pipe is not empty.
		 In this case, the pipe is really full if WriteQuotaAvailable
		 is zero. Otherwise, the pipe is empty. */
	      status = fh->set_pipe_non_blocking (true);
	      if (NT_SUCCESS (status))
		/* Pipe should be empty because reader is waiting for data. */
		/* Restore the pipe mode to blocking. */
		fh->set_pipe_non_blocking (false);
	      else if (status == STATUS_PIPE_BUSY)
		{
		  /* Full */
		  set_errno (EAGAIN);
		  goto err;
		}
	    }
	}
      else
	{
	  /* The pipe space is unknown. */
	  status = fh->set_pipe_non_blocking (true);
	  if (NT_SUCCESS (status))
	    real_non_blocking_mode = true;
	  else if (status == STATUS_PIPE_BUSY)
	    {
	      /* The pipe is not empty and may be full.
		 It is not safe to write now. */
	      set_errno (EAGAIN);
	      goto err;
	    }
	}
      if (STATUS_PIPE_IS_CLOSED (status))
	{
	  set_errno (EPIPE);
	  raise (SIGPIPE);
	  goto err;
	}
      else if (!NT_SUCCESS (status))
	{
	  /* Cannot continue. What should we do? */
	  set_errno (EIO);
	  goto err;
	}
    }

  if (len <= (size_t) avail)
    chunk = len;
  else
    chunk = avail;

  if (!(evt = CreateEvent (NULL, false, false, NULL)))
    {
      __seterrno ();
      goto err;
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

      if (left > chunk && !is_nonblocking ())
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
	  if (is_nonblocking() && !real_non_blocking_mode && len1 > avail)
	    /* Avoid being blocked in NtWriteFile() */
	    io.Information = 0;
	  else
	    status = NtWriteFile (get_handle (), evt, NULL, NULL, &io,
				  (PVOID) ptr, len1, NULL, NULL);
	  if (status == STATUS_PENDING)
	    {
	      do
		{
		  waitret = cygwait (evt, (DWORD) 0);
		  /* Break out if no SA_RESTART. */
		  if (waitret == WAIT_SIGNALED)
		    break;
		  /* Break out on completion */
		  if (waitret == WAIT_OBJECT_0)
		    break;
		  /* If we got a timeout in the blocking case, and we already
		     did a short write, we got a signal in the previous loop. */
		  if (waitret == WAIT_TIMEOUT && short_write_once)
		    {
		      waitret = WAIT_SIGNALED;
		      break;
		    }
		  cygwait (select_sem, 10, cw_cancel);
		}
	      while (waitret == WAIT_TIMEOUT);
	      /* If io.Status is STATUS_CANCELLED after CancelIo, IO has
		 actually been cancelled and io.Information contains the
		 number of bytes processed so far.
		 Otherwise IO has been finished regulary and io.Status
		 contains valid success or error information. */
	      CancelIo (get_handle ());
	      if (waitret == WAIT_SIGNALED && io.Status != STATUS_CANCELLED)
		waitret = WAIT_OBJECT_0;

	      if (waitret == WAIT_CANCELED)
		status = STATUS_THREAD_CANCELED;
	      else if (waitret == WAIT_SIGNALED)
		status = STATUS_THREAD_SIGNALED;
	      else if (waitret == WAIT_TIMEOUT && io.Status == STATUS_CANCELLED)
		status = STATUS_SUCCESS;
	      else
		status = io.Status;
	    }
	  if (status != STATUS_THREAD_SIGNALED && !NT_SUCCESS (status))
	    break;
	  if (io.Information > 0 || len <= PIPE_BUF || short_write_once)
	    break;
	  /* Independent of being blocking or non-blocking, if we're here,
	     the pipe has less space than requested.  If the pipe is a
	     non-Cygwin pipe, just try the old strategy of trying a half
	     write.  If the pipe has at
	     least PIPE_BUF bytes available, try to write all matching
	     PIPE_BUF sized blocks.  If it's less than PIPE_BUF,  try
	     the next less power of 2 bytes.  This is not really the Linux
	     strategy because Linux is filling the pages of a pipe buffer
	     in a very implementation-defined way we can't emulate, but it
	     resembles it closely enough to get useful results. */
	  avail = pipe_data_available (-1, this, get_handle (), PDA_WRITE);
	  if (avail < 1)	/* error or pipe closed */
	    break;
	  if (avail > len1)	/* somebody read from the pipe */
	    avail = len1;
	  if (avail == 1)	/* 1 byte left or non-Cygwin pipe */
	    len1 >>= 1;
	  else if (avail >= PIPE_BUF)
	    len1 = avail & ~(PIPE_BUF - 1);
	  else
	    len1 = 1 << (63 - __builtin_clzl (avail));
	  short_write_once = true;
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

      if (nbytes_now == 0 || short_write_once)
	break;
    }

  if (real_non_blocking_mode)
    ((fhandler_pipe *) this)->set_pipe_non_blocking (false);

  CloseHandle (evt);
  if (pipe_mtx) /* pipe_mtx is NULL in the fifo case */
    ReleaseMutex (pipe_mtx);
  if (status == STATUS_THREAD_SIGNALED && nbytes == 0)
    set_errno (EINTR);
  else if (status == STATUS_THREAD_CANCELED)
    pthread::static_cancel_self ();
  return nbytes ?: -1;

err:
  if (pipe_mtx)
    ReleaseMutex (pipe_mtx);
  return -1;
}

void
fhandler_pipe::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  if (pipe_mtx)
    set_no_inheritance (pipe_mtx, val);
  if (select_sem)
    set_no_inheritance (select_sem, val);
}

void
fhandler_pipe::fixup_after_fork (HANDLE parent)
{
  if (pipe_mtx)
    fork_fixup (parent, pipe_mtx, "pipe_mtx");
  if (select_sem)
    fork_fixup (parent, select_sem, "select_sem");
  fhandler_base::fixup_after_fork (parent);
}

int
fhandler_pipe::dup (fhandler_base *child, int flags)
{
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->set_popen_pid (0);

  int res = 0;
  if (fhandler_base::dup (child, flags))
    res = -1;
  else if (pipe_mtx &&
	   !DuplicateHandle (GetCurrentProcess (), pipe_mtx,
			     GetCurrentProcess (), &ftp->pipe_mtx,
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
  if (pipe_mtx)
    CloseHandle (pipe_mtx);
  int ret = fhandler_base::close ();
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
  int64_t unique_id = 0; /* Compiler complains if not initialized... */

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

  char name[MAX_PATH], *mtx_name;
  mtx_name = get_mtx_name (fhs[0]->get_handle (), "input", name);
  fhs[0]->pipe_mtx = CreateMutex (sa, FALSE, mtx_name);
  if (!fhs[0]->pipe_mtx)
    goto err_delete_fhs1;
  mtx_name = get_mtx_name (fhs[1]->get_handle (), "output", name);
  fhs[1]->pipe_mtx = CreateMutex (sa, FALSE, mtx_name);
  if (!fhs[1]->pipe_mtx)
    goto err_close_pipe_mtx0;

  fhs[0]->select_sem = CreateSemaphore (sa, 0, INT32_MAX, NULL);
  if (!fhs[0]->select_sem)
    goto err_close_pipe_mtx1;
  if (!DuplicateHandle (GetCurrentProcess (), fhs[0]->select_sem,
			GetCurrentProcess (), &fhs[1]->select_sem,
			0, sa->bInheritHandle, DUPLICATE_SAME_ACCESS))
    goto err_close_select_sem0;

  res = 0;
  goto out;

err_close_select_sem0:
  CloseHandle (fhs[0]->select_sem);
err_close_pipe_mtx1:
  CloseHandle (fhs[1]->pipe_mtx);
err_close_pipe_mtx0:
  CloseHandle (fhs[0]->pipe_mtx);
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
      /* Set FILE_SYNCHRONOUS_IO_NONALERT flag so that native
	 C# programs work with cygwin pipe. */
      status = NtCreateNamedPipeFile (&r, access, &attr, &io,
				      FILE_SHARE_READ | FILE_SHARE_WRITE,
				      FILE_CREATE,
				      FILE_SYNCHRONOUS_IO_NONALERT, pipe_type,
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

int
fhandler_pipe::fstatvfs (struct statvfs *sfs)
{
  set_errno (EBADF);
  return -1;
}
