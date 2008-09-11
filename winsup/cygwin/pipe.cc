/* pipe.cc: pipe for Cygwin.

   Copyright 1996, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008 Hat, Inc.

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

fhandler_pipe::fhandler_pipe ()
  : fhandler_base (), popen_pid (0)
{
  get_overlapped ()->hEvent = NULL;
  need_fork_fixup (true);
}

struct pipesync
{
  bool reader;
  HANDLE ev, non_cygwin_h, ret_handle;
  pipesync(HANDLE, DWORD);
  int operator == (int x) const {return !!ev;}
  static DWORD WINAPI handler (LPVOID *);
};

inline bool
getov_result (HANDLE h, DWORD& nbytes, LPOVERLAPPED ov)
{
  if (ov && (GetLastError () != ERROR_IO_PENDING
	     || !GetOverlappedResult (h, ov, &nbytes, true)))
    {
      __seterrno ();
      return false;
    }
  return true;
}

static DWORD WINAPI
pipe_handler (LPVOID in_ps)
{
  pipesync ps = *(pipesync *) in_ps;
  HANDLE h, in, out;
  DWORD err = fhandler_pipe::create_selectable (&sec_none_nih, in, out, 0);
  if (err)
    {
      SetLastError (err);
      system_printf ("couldn't create a shadow pipe for non-cygwin pipe I/O, %E");
      return 0;
    }
  h = ((pipesync *) in_ps)->ret_handle = ps.reader ? in : out;
  SetHandleInformation (h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
  SetEvent (ps.ev);

  char buf[4096];
  DWORD read_bytes, write_bytes;
  HANDLE hread, hwrite, hclose;
  OVERLAPPED ov, *rov, *wov;
  memset (&ov, 0, sizeof (ov));
  ov.hEvent = CreateEvent (&sec_none_nih, true, false, NULL);
  if (ps.reader)
    {
      hread = ps.non_cygwin_h;
      hclose = hwrite = out;
      wov = &ov;
      rov = NULL;
    }
  else
    {
      hclose = hread = in;
      hwrite = ps.non_cygwin_h;
      rov = &ov;
      wov = NULL;
    }

  while (1)
    {
      ResetEvent (ov.hEvent);
      BOOL res = ReadFile (hread, buf, 4096, &read_bytes, rov);
      if (!res && !getov_result (hread, read_bytes, rov))
	break;
      if (!read_bytes)
	break;

      res = WriteFile (hwrite, buf, read_bytes, &write_bytes, wov);
      if (!res && !getov_result (hwrite, write_bytes, wov))
	break;
      if (write_bytes != read_bytes)
	break;
    }

  err = GetLastError ();
  CloseHandle (ov.hEvent);
  CloseHandle (hclose);
  CloseHandle (ps.non_cygwin_h);
  SetLastError (err);
  return 0;
}

pipesync::pipesync (HANDLE f, DWORD is_reader):
  reader (false), ret_handle (NULL)
{
  ev = CreateEvent (&sec_none_nih, true, false, NULL);
  if (!ev)
    system_printf ("couldn't create synchronization event for non-cygwin pipe, %E");
  else
    {
      debug_printf ("created thread synchronization event %p", ev);
      non_cygwin_h = f;
      reader = !!is_reader;
      ret_handle = NULL;

      DWORD tid;
      HANDLE ht = CreateThread (&sec_none_nih, 0, pipe_handler, this, 0, &tid);

      if (!ht)
	goto out;
      CloseHandle (ht);

      switch (WaitForSingleObject (ev, INFINITE))
	{
	case WAIT_OBJECT_0:
	  break;
	default:
	  system_printf ("WFSO failed waiting for synchronization event for non-cygwin pipe, %E");
	  break;
	}
    }

out:
  if (ev)
    {
      CloseHandle (ev);
      ev = NULL;
    }
  return;
}

void
fhandler_pipe::init (HANDLE f, DWORD a, mode_t mode)
{
  // FIXME: Have to clean this up someday
  if (!*get_win32_name () && get_name ())
    {
      char *hold_normalized_name = (char *) alloca (strlen (get_name ()) + 1);
      strcpy (hold_normalized_name, get_name ());
      char *s, *d;
      for (s = hold_normalized_name, d = (char *) get_win32_name (); *s; s++, d++)
	if (*s == '/')
	  *d = '\\';
	else
	  *d = *s;
      *d = '\0';
      set_name (hold_normalized_name);
    }

  bool opened_properly = a & FILE_CREATE_PIPE_INSTANCE;
  a &= ~FILE_CREATE_PIPE_INSTANCE;
  if (!opened_properly)
    {
      pipesync ps (f, a & GENERIC_READ);
      f = ps.ret_handle;
    }

  fhandler_base::init (f, a, mode);
  if (mode & O_NOINHERIT)
    close_on_exec (true);
  setup_overlapped ();
}

extern "C" int sscanf (const char *, const char *, ...);

int
fhandler_pipe::open (int flags, mode_t mode)
{
  HANDLE proc, pipe_hdl, nio_hdl = NULL;
  fhandler_pipe *fh = NULL;
  size_t size;
  int pid, rwflags = (flags & O_ACCMODE);
  bool inh;

  sscanf (get_name (), "/proc/%d/fd/pipe:[%d]", &pid, (int *) &pipe_hdl);
  if (pid == myself->pid)
    {
      cygheap_fdenum cfd (true);
      while (cfd.next () >= 0)
	{
	  if (cfd->get_handle () != pipe_hdl)
	    continue;
	  if ((rwflags == O_RDONLY && !(cfd->get_access () & GENERIC_READ))
	      || (rwflags == O_WRONLY && !(cfd->get_access () & GENERIC_WRITE)))
	    {
	      set_errno (EACCES);
	      return 0;
	    }
	  if (!cfd->dup (this))
	    return 1;
	  return 0;
	}
      set_errno (ENOENT);
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
  if (!(fh = p->pipe_fhandler (pipe_hdl, size)) || !size)
    {
      set_errno (ENOENT);
      goto out;
    }
  /* Too bad, but Windows only allows the same access mode when dup'ing
     the pipe. */
  if ((rwflags == O_RDONLY && !(fh->get_access () & GENERIC_READ))
      || (rwflags == O_WRONLY && !(fh->get_access () & GENERIC_WRITE)))
    {
      set_errno (EACCES);
      goto out;
    }
  inh = !(flags & O_NOINHERIT);
  if (!DuplicateHandle (proc, pipe_hdl, hMainProc, &nio_hdl,
			0, inh, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto out;
    }
  init (nio_hdl, fh->get_access (), mode & O_TEXT ?: O_BINARY);
  if (flags & O_NOINHERIT)
    close_on_exec (true);
  uninterruptible_io (fh->uninterruptible_io ());
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

_off64_t
fhandler_pipe::lseek (_off64_t offset, int whence)
{
  debug_printf ("(%d, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_pipe::fadvise (_off64_t offset, _off64_t length, int advice)
{
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_pipe::ftruncate (_off64_t length, bool allow_truncate)
{
  set_errno (allow_truncate ? EINVAL : ESPIPE);
  return -1;
}

char *
fhandler_pipe::get_proc_fd_name (char *buf)
{
  __small_sprintf (buf, "pipe:[%d]", get_handle ());
  return buf;
}

void
fhandler_pipe::raw_read (void *in_ptr, size_t& in_len)
{
  return read_overlapped (in_ptr, in_len);
}

int
fhandler_pipe::raw_write (const void *ptr, size_t len)
{
  return write_overlapped (ptr, len);
}

int
fhandler_pipe::dup (fhandler_base *child)
{
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->set_popen_pid (0);

  int res;
  if (get_handle () && fhandler_base::dup (child))
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
int
fhandler_pipe::create_selectable (LPSECURITY_ATTRIBUTES sa_ptr, HANDLE& r,
				  HANDLE& w, DWORD psize, const char *name)
{
  /* Default to error. */
  r = w = INVALID_HANDLE_VALUE;

  /* Ensure that there is enough pipe buffer space for atomic writes.  */
  if (psize < PIPE_BUF)
    psize = PIPE_BUF;

  char pipename[MAX_PATH] = PIPE_INTRO;
  /* FIXME: Eventually make ttys work with overlapped I/O. */
  DWORD overlapped = name ? 0 : FILE_FLAG_OVERLAPPED;

  /* Retry CreateNamedPipe as long as the pipe name is in use.
     Retrying will probably never be necessary, but we want
     to be as robust as possible.  */
  DWORD err;
  do
    {
      static volatile ULONG pipe_unique_id;
      if (!name)
	__small_sprintf (pipename + strlen(PIPE_INTRO), "%p-%p", myself->pid,
			InterlockedIncrement ((LONG *) &pipe_unique_id));
      else
	strcpy (pipename + strlen(PIPE_INTRO), name);

      debug_printf ("CreateNamedPipe: name %s, size %lu", pipename, psize);

      err = 0;
      /* Use CreateNamedPipe instead of CreatePipe, because the latter
	 returns a write handle that does not permit FILE_READ_ATTRIBUTES
	 access, on versions of win32 earlier than WinXP SP2.
	 CreatePipe also stupidly creates a full duplex pipe, which is
	 a waste, since only a single direction is actually used.
	 It's important to only allow a single instance, to ensure that
	 the pipe was not created earlier by some other process, even if
	 the pid has been reused.  We avoid FILE_FLAG_FIRST_PIPE_INSTANCE
	 because that is only available for Win2k SP2 and WinXP.  */
      r = CreateNamedPipe (pipename, PIPE_ACCESS_INBOUND | overlapped,
			   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1, psize,
			   psize, NMPWAIT_USE_DEFAULT_WAIT, sa_ptr);

      /* Win 95 seems to return NULL instead of INVALID_HANDLE_VALUE */
      if (r && r != INVALID_HANDLE_VALUE)
	{
	  debug_printf ("pipe read handle %p", r);
	  break;
	}

      err = GetLastError ();
      switch (err)
	{
	case ERROR_PIPE_BUSY:
	  /* The pipe is already open with compatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe busy, retrying");
	  break;
	case ERROR_ACCESS_DENIED:
	  /* The pipe is already open with incompatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe access denied, retrying");
	  break;
	default:
	  {
	    err = GetLastError ();
	    debug_printf ("CreatePipe failed, %E");
	    return err;
	  }
	}
    }
  while (!name);

  if (err)
    return err;

  debug_printf ("CreateFile: name %s", pipename);

  /* Open the named pipe for writing.
     Be sure to permit FILE_READ_ATTRIBUTES access.  */
  w = CreateFile (pipename, GENERIC_WRITE | FILE_READ_ATTRIBUTES, 0, sa_ptr,
		  OPEN_EXISTING, overlapped, 0);

  if (!w || w == INVALID_HANDLE_VALUE)
    {
      /* Failure. */
      DWORD err = GetLastError ();
      debug_printf ("CreateFile failed, %E");
      CloseHandle (r);
      return err;
    }

  debug_printf ("pipe write handle %p", w);

  /* Success. */
  return 0;
}

int
fhandler_pipe::create (fhandler_pipe *fhs[2], unsigned psize, int mode)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;
  int res;

  int ret = create_selectable (sa, r, w, psize);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      res = -1;
    }
  else
    {
      fhs[0] = (fhandler_pipe *) build_fh_dev (*piper_dev);
      fhs[1] = (fhandler_pipe *) build_fh_dev (*pipew_dev);

      mode |= mode & O_TEXT ?: O_BINARY;
      fhs[0]->init (r, FILE_CREATE_PIPE_INSTANCE | GENERIC_READ, mode);
      fhs[1]->init (w, FILE_CREATE_PIPE_INSTANCE | GENERIC_WRITE, mode);
      res = 0;
    }

  syscall_printf ("%d = pipe ([%p, %p], %d, %p)", res, fhs[0], fhs[1], psize, mode);
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

int __stdcall
fhandler_pipe::fstatvfs (struct statvfs *sfs)
{
  set_errno (EBADF);
  return -1;
}

#define DEFAULT_PIPEBUFSIZE 65536

extern "C" int
pipe (int filedes[2])
{
  fhandler_pipe *fhs[2];
  int res = fhandler_pipe::create (fhs, DEFAULT_PIPEBUFSIZE, O_BINARY);
  if (res == 0)
    {
      cygheap_fdnew fdin;
      cygheap_fdnew fdout (fdin, false);
      fdin = fhs[0];
      fdout = fhs[1];
      filedes[0] = fdin;
      filedes[1] = fdout;
    }

  return res;
}

extern "C" int
_pipe (int filedes[2], unsigned int psize, int mode)
{
  fhandler_pipe *fhs[2];
  int res = fhandler_pipe::create (fhs, psize, mode);
  /* This type of pipe is not interruptible so set the appropriate flag. */
  if (!res)
    {
      cygheap_fdnew fdin;
      cygheap_fdnew fdout (fdin, false);
      fhs[0]->uninterruptible_io (true);
      fdin = fhs[0];
      fdout = fhs[1];
      filedes[0] = fdin;
      filedes[1] = fdout;
    }

  return res;
}
