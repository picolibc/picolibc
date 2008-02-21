/* pipe.cc: pipe for Cygwin.

   Copyright 1996, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* FIXME: Should this really be fhandler_pipe.cc? */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <limits.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "thread.h"
#include "pinfo.h"
#include "cygthread.h"
#include "ntdll.h"

static unsigned pipecount;
static const NO_COPY char pipeid_fmt[] = "stupid_pipe.%u.%u";

fhandler_pipe::fhandler_pipe ()
  : fhandler_base (), guard (NULL), broken_pipe (false), writepipe_exists (NULL),
    orig_pid (0), id (0), popen_pid (0)
{
  need_fork_fixup (true);
}

extern "C" int sscanf (const char *, const char *, ...);

void
fhandler_pipe::init (HANDLE f, DWORD a, mode_t mode)
{
  bool isread = !!(a & GENERIC_READ);
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;
  if (mode & O_NOINHERIT)
     close_on_exec (true);

  if (isread)
    {
      create_read_state (2);
      create_guard (sa);
    }

  if (!wincap.has_unreliable_pipes ())
    /* nothing to do */;
  else if (isread)
    {
      orig_pid = myself->pid;
      id = ++pipecount;
    }
  else /* NOTE: This assumes that pipecount has been incremented by a previous
	  init of the read end of the pipe.  That isn't really true of native
	  pipes but, ask me if I care.  */
    {
      char buf[80];
      __small_sprintf (buf, pipeid_fmt, myself->pid, pipecount);
     writepipe_exists = CreateEvent (sa, TRUE, FALSE, buf);
    }

  fhandler_base::init (f, a, mode);
}

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
  if (!fh->guard)
    /* nothing to do */;
  else if (DuplicateHandle (proc, fh->guard, hMainProc, &guard,
			    0, inh, DUPLICATE_SAME_ACCESS))
    ProtectHandle (guard);
  else
    {
      __seterrno ();
      goto out;
    }
  if (!fh->writepipe_exists)
    /* nothing to do */;
  else if (!DuplicateHandle (proc, fh->writepipe_exists,
			     hMainProc, &writepipe_exists,
			     0, inh, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      goto out;
    }
  if (fh->read_state)
    create_read_state (2);
  init (nio_hdl, fh->get_access (), mode & O_TEXT ?: O_BINARY);
  if (flags & O_NOINHERIT)
    close_on_exec (true);
  uninterruptible_io (fh->uninterruptible_io ());
  cfree (fh);
  CloseHandle (proc);
  return 1;
out:
  if (writepipe_exists)
    CloseHandle (writepipe_exists);
  if (guard)
    CloseHandle (guard);
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

void
fhandler_pipe::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  if (guard)
    {
      set_no_inheritance (guard, val);
      ModifyHandle (guard, !val);
    }
  if (writepipe_exists)
    set_no_inheritance (writepipe_exists, val);
}

char *
fhandler_pipe::get_proc_fd_name (char *buf)
{
  __small_sprintf (buf, "pipe:[%d]", get_handle ());
  return buf;
}

struct pipeargs
{
  fhandler_base *fh;
  void *ptr;
  size_t *len;
};

static DWORD WINAPI
read_pipe (void *arg)
{
  pipeargs *pi = (pipeargs *) arg;
  pi->fh->fhandler_base::read (pi->ptr, *pi->len);
  return 0;
}

void __stdcall
fhandler_pipe::read (void *in_ptr, size_t& in_len)
{
  if (broken_pipe)
    in_len = 0;
  else
    {
      pipeargs pi = {dynamic_cast<fhandler_base *>(this), in_ptr, &in_len};
      cygthread *th = new cygthread (read_pipe, 0, &pi, "read_pipe");
      if (th->detach (read_state) && !in_len)
	in_len = (size_t) -1;	/* received a signal */
    }
  ReleaseMutex (guard);
}

int
fhandler_pipe::close ()
{
  if (guard)
    ForceCloseHandle (guard);
  if (writepipe_exists)
    CloseHandle (writepipe_exists);
#ifndef NEWVFORK
  if (read_state)
#else
  // FIXME is this vfork_cleanup test right?  Is it responsible for some of
  // the strange pipe behavior that has been reported in the cygwin mailing
  // list?
  if (read_state && !cygheap->fdtab.in_vfork_cleanup ())
#endif
    ForceCloseHandle (read_state);
  return fhandler_base::close ();
}

bool
fhandler_pipe::hit_eof ()
{
  char buf[80];
  HANDLE ev;
  if (broken_pipe)
    return 1;
  if (!orig_pid)
    return false;
  __small_sprintf (buf, pipeid_fmt, orig_pid, id);
  if ((ev = OpenEvent (EVENT_ALL_ACCESS, FALSE, buf)))
    CloseHandle (ev);
  debug_printf ("%s %p", buf, ev);
  return ev == NULL;
}

void
fhandler_pipe::fixup_in_child ()
{
  if (read_state)
    create_read_state (2);
}

void
fhandler_pipe::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_in_child ();
}

void
fhandler_pipe::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
  if (guard && fork_fixup (parent, guard, "guard"))
    ProtectHandle (guard);
  if (writepipe_exists)
    fork_fixup (parent, writepipe_exists, "writepipe_exists");
  fixup_in_child ();
}

int
fhandler_pipe::dup (fhandler_base *child)
{
  int res = -1;
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->set_popen_pid (0);
  ftp->guard = ftp->writepipe_exists = ftp->read_state = NULL;

  if (get_handle () && fhandler_base::dup (child))
    goto err;

  if (!guard)
    /* nothing to do */;
  else if (DuplicateHandle (hMainProc, guard, hMainProc, &ftp->guard, 0, true,
			    DUPLICATE_SAME_ACCESS))
    ProtectHandle1 (ftp->guard, guard);
  else
    {
      debug_printf ("couldn't duplicate guard %p, %E", guard);
      goto err;
    }

  if (!writepipe_exists)
    /* nothing to do */;
  else if (!DuplicateHandle (hMainProc, writepipe_exists, hMainProc,
			     &ftp->writepipe_exists, 0, true,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate writepipe_exists %p, %E", writepipe_exists);
      goto err;
    }

  if (!read_state)
    /* nothing to do */;
  else if (DuplicateHandle (hMainProc, read_state, hMainProc,
			    &ftp->read_state, 0, false,
			    DUPLICATE_SAME_ACCESS))
    ProtectHandle1 (ftp->read_state, read_state);
  else
    {
      debug_printf ("couldn't duplicate read_state %p, %E", read_state);
      goto err;
    }

  res = 0;
  goto out;

err:
  if (ftp->guard)
    ForceCloseHandle1 (ftp->guard, guard);
  if (ftp->writepipe_exists)
    CloseHandle (ftp->writepipe_exists);
  if (ftp->read_state)
    ForceCloseHandle1 (ftp->read_state, read_state);
  goto leave;

out:
  ftp->id = id;
  ftp->orig_pid = orig_pid;
  VerifyHandle (ftp->writepipe_exists);

leave:
  debug_printf ("res %d", res);
  return res;
}

/* Create a pipe, and return handles to the read and write ends,
   just like CreatePipe, but ensure that the write end permits
   FILE_READ_ATTRIBUTES access, on later versions of win32 where
   this is supported.  This access is needed by NtQueryInformationFile,
   which is used to implement select and nonblocking writes.
   Note that the return value is either 0 or GetLastError,
   unlike CreatePipe, which returns a bool for success or failure.  */
int
fhandler_pipe::create_selectable (LPSECURITY_ATTRIBUTES sa_ptr, HANDLE& r,
				  HANDLE& w, DWORD psize, bool fifo)
{
  /* Default to error. */
  r = w = INVALID_HANDLE_VALUE;

  /* Ensure that there is enough pipe buffer space for atomic writes.  */
  if (!fifo && psize < PIPE_BUF)
    psize = PIPE_BUF;

  char pipename[CYG_MAX_PATH];

  /* Retry CreateNamedPipe as long as the pipe name is in use.
     Retrying will probably never be necessary, but we want
     to be as robust as possible.  */
  while (1)
    {
      static volatile ULONG pipe_unique_id;

      __small_sprintf (pipename, "\\\\.\\pipe\\cygwin-%p-%p", myself->pid,
		       InterlockedIncrement ((LONG *) &pipe_unique_id));

      debug_printf ("CreateNamedPipe: name %s, size %lu", pipename, psize);

      /* Use CreateNamedPipe instead of CreatePipe, because the latter
	 returns a write handle that does not permit FILE_READ_ATTRIBUTES
	 access, on versions of win32 earlier than WinXP SP2.
	 CreatePipe also stupidly creates a full duplex pipe, which is
	 a waste, since only a single direction is actually used.
	 It's important to only allow a single instance, to ensure that
	 the pipe was not created earlier by some other process, even if
	 the pid has been reused.  We avoid FILE_FLAG_FIRST_PIPE_INSTANCE
	 because that is only available for Win2k SP2 and WinXP.  */
      r = CreateNamedPipe (pipename, PIPE_ACCESS_INBOUND,
			   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1, psize,
			   psize, NMPWAIT_USE_DEFAULT_WAIT, sa_ptr);

      /* Win 95 seems to return NULL instead of INVALID_HANDLE_VALUE */
      if (r && r != INVALID_HANDLE_VALUE)
	{
	  debug_printf ("pipe read handle %p", r);
	  break;
	}

      DWORD err = GetLastError ();
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
	  /* CreateNamePipe failed.  Maybe we are on an older Win9x platform without
	     named pipes.  Return an anonymous pipe as the best approximation.  */
	  debug_printf ("CreateNamedPipe failed, resorting to CreatePipe size %lu",
			psize);
	  if (CreatePipe (&r, &w, sa_ptr, psize))
	    {
	      debug_printf ("pipe read handle %p", r);
	      debug_printf ("pipe write handle %p", w);
	      return 0;
	    }
	  err = GetLastError ();
	  debug_printf ("CreatePipe failed, %E");
	  return err;
	}
    }

  debug_printf ("CreateFile: name %s", pipename);

  /* Open the named pipe for writing.
     Be sure to permit FILE_READ_ATTRIBUTES access.  */
  w = CreateFile (pipename, GENERIC_WRITE | FILE_READ_ATTRIBUTES, 0, sa_ptr,
		  OPEN_EXISTING, 0, 0);

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
fhandler_pipe::create (fhandler_pipe *fhs[2], unsigned psize, int mode, bool fifo)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;
  int res = -1;

  int ret = create_selectable (sa, r, w, psize, fifo);
  if (ret)
    __seterrno_from_win_error (ret);
  else
    {
      fhs[0] = (fhandler_pipe *) build_fh_dev (*piper_dev);
      fhs[1] = (fhandler_pipe *) build_fh_dev (*pipew_dev);

      mode |= mode & O_TEXT ?: O_BINARY;
      fhs[0]->init (r, GENERIC_READ, binmode);
      fhs[1]->init (w, GENERIC_WRITE, binmode);
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

#define DEFAULT_PIPEBUFSIZE (16 * PIPE_BUF)

extern "C" int
pipe (int filedes[2])
{
  extern DWORD binmode;
  fhandler_pipe *fhs[2];
  int res = fhandler_pipe::create (fhs, DEFAULT_PIPEBUFSIZE,
				   (!binmode || binmode == O_BINARY)
				   ? O_BINARY : O_TEXT);
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
