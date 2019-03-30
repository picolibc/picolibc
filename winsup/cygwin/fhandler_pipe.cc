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

fhandler_pipe::fhandler_pipe ()
  : fhandler_base_overlapped (), popen_pid (0)
{
  max_atomic_write = DEFAULT_PIPEBUFSIZE;
  need_fork_fixup (true);
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
    setup_overlapped ();
  else
    destroy_overlapped ();
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
	  cfd->copyto (this);
	  set_handle (NULL);
	  pc.reset_conv_handle ();
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

int
fhandler_pipe::dup (fhandler_base *child, int flags)
{
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->set_popen_pid (0);

  int res;
  if (get_handle () && fhandler_base_overlapped::dup (child, flags))
    res = -1;
  else
    res = 0;

  debug_printf ("res %d", res);
  return res;
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

int
fhandler_pipe::create (fhandler_pipe *fhs[2], unsigned psize, int mode)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES *sa = sec_none_cloexec (mode);
  int res = -1;
  int64_t unique_id;

  int ret = create (sa, &r, &w, psize, NULL, FILE_FLAG_OVERLAPPED, &unique_id);
  if (ret)
    __seterrno_from_win_error (ret);
  else if ((fhs[0] = (fhandler_pipe *) build_fh_dev (*piper_dev)) == NULL)
    {
      CloseHandle (r);
      CloseHandle (w);
    }
  else if ((fhs[1] = (fhandler_pipe *) build_fh_dev (*pipew_dev)) == NULL)
    {
      delete fhs[0];
      CloseHandle (w);
    }
  else
    {
      mode |= mode & O_TEXT ?: O_BINARY;
      fhs[0]->init (r, FILE_CREATE_PIPE_INSTANCE | GENERIC_READ, mode,
		    unique_id);
      fhs[1]->init (w, FILE_CREATE_PIPE_INSTANCE | GENERIC_WRITE, mode,
		    unique_id);
      res = 0;
    }

  debug_printf ("%R = pipe([%p, %p], %d, %y)", res, fhs[0], fhs[1], psize, mode);
  return res;
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
