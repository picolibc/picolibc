/* pipe.cc: pipe for Cygwin.

   Copyright 1996, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005 Red Hat, Inc.

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
  : fhandler_base (), guard (NULL), broken_pipe (false), writepipe_exists(0),
    orig_pid (0), id (0)
{
}

int
fhandler_pipe::open (int flags, mode_t mode)
{
  const char *path = get_name ();
  debug_printf ("path: %s", path);
  if (!strncmp (get_name (), "/proc/", 6))
    {
      char *c;
      HANDLE hdl;
      int pid = strtol (path += 6, &c, 10);
      if (!pid || !c || *c != '/')
        goto out;
      path = c;
      if (strncmp (path, "/fd/pipe:[", 10))
        goto out;
      path += 10;
      hdl = (HANDLE) atoi (path);
      if (pid == myself->pid)
        {
	  cygheap_fdenum cfd;
	  while (cfd.next () >= 0)
	    {
	      if (cfd->get_handle () == hdl)
	        {
		  if (!cfd->dup (this))
		    return 1;
		  return 0;
		}
	    }
	}
      else
        {
	  /* TODO: Open pipes of different process.  Is that possible? */
	}
    }
out:
  set_errno (ENXIO);
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
    set_no_inheritance (guard, val);
  if (writepipe_exists)
    set_no_inheritance (writepipe_exists, val);
}

char *fhandler_pipe::get_proc_fd_name (char *buf)
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
      ResetEvent (read_state);
      cygthread *th = new cygthread (read_pipe, &pi, "read_pipe");
      if (th->detach (read_state) && !in_len)
	in_len = (size_t) -1;	/* received a signal */
    }
  (void) ReleaseMutex (guard);
  return;
}

int
fhandler_pipe::close ()
{
  if (guard)
    CloseHandle (guard);
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
  if (get_handle ())
    {
      CloseHandle (get_handle ());
      set_io_handle (NULL);
    }
  return 0;
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
fhandler_pipe::fixup_after_exec ()
{
  if (read_state)
    {
      create_read_state (2);
      ProtectHandle (read_state);
    }
}

void
fhandler_pipe::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
  if (guard)
    fork_fixup (parent, guard, "guard");
  if (writepipe_exists)
    fork_fixup (parent, writepipe_exists, "guard");
  fixup_after_exec ();
}

int
fhandler_pipe::dup (fhandler_base *child)
{
  int res = -1;
  fhandler_pipe *ftp = (fhandler_pipe *) child;
  ftp->guard = ftp->writepipe_exists = ftp->read_state = NULL;

  if (get_handle ())
    {
      res = fhandler_base::dup (child);
      if (res)
	goto err;
    }

  /* FIXME: This leaks handles in the failing condition */
  if (guard == NULL)
    ftp->guard = NULL;
  else if (!DuplicateHandle (hMainProc, guard, hMainProc, &ftp->guard, 0, 1,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate guard %p, %E", guard);
      goto err;
    }

  if (writepipe_exists == NULL)
    ftp->writepipe_exists = NULL;
  else if (!DuplicateHandle (hMainProc, writepipe_exists, hMainProc,
			     &ftp->writepipe_exists, 0, 1,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate writepipe_exists %p, %E", writepipe_exists);
      goto err;
    }

  if (read_state == NULL)
    ftp->read_state = NULL;
  else if (!DuplicateHandle (hMainProc, read_state, hMainProc,
			     &ftp->read_state, 0, 0,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate read_state %p, %E", read_state);
      goto err;
    }

  res = 0;
  goto out;

err:
  if (ftp->guard)
    CloseHandle (ftp->guard);
  if (ftp->writepipe_exists)
    CloseHandle (ftp->writepipe_exists);
  if (ftp->read_state)
    CloseHandle (ftp->read_state);
  goto leave;

out:
  ftp->id = id;
  ftp->orig_pid = orig_pid;
  VerifyHandle (ftp->guard);
  VerifyHandle (ftp->writepipe_exists);
  VerifyHandle (ftp->read_state);

leave:
  debug_printf ("res %d", res);
  return res;
}

/* Create a pipe, and return handles to the read and write ends,
   just like CreatePipe, but ensure that the write end permits
   FILE_READ_ATTRIBUTES access, on later versions of win32 where
   this is supported.  This access is needed by NtQueryInformationFile,
   which is used to implement select and nonblocking writes.
   Note that the return value is either NO_ERROR or GetLastError,
   unlike CreatePipe, which returns a bool for success or failure.  */
static int
create_selectable_pipe (PHANDLE read_pipe_ptr,
			PHANDLE write_pipe_ptr,
			LPSECURITY_ATTRIBUTES sa_ptr,
			DWORD psize)
{
  /* Default to error. */
  *read_pipe_ptr = *write_pipe_ptr = INVALID_HANDLE_VALUE;

  HANDLE read_pipe = INVALID_HANDLE_VALUE, write_pipe = INVALID_HANDLE_VALUE;

  /* Ensure that there is enough pipe buffer space for atomic writes.  */
  if (psize < PIPE_BUF)
    psize = PIPE_BUF;

  char pipename[CYG_MAX_PATH];

  /* Retry CreateNamedPipe as long as the pipe name is in use.
     Retrying will probably never be necessary, but we want
     to be as robust as possible.  */
  while (1)
    {
      static volatile LONG pipe_unique_id;

      __small_sprintf (pipename, "\\\\.\\pipe\\cygwin-%d-%ld", myself->pid,
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
      SetLastError (0);
      read_pipe = CreateNamedPipe (pipename,
				   PIPE_ACCESS_INBOUND,
				   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
				   1,       /* max instances */
				   psize,   /* output buffer size */
				   psize,   /* input buffer size */
				   NMPWAIT_USE_DEFAULT_WAIT,
				   sa_ptr);

      DWORD err = GetLastError ();
      /* Win 95 seems to return NULL instead of INVALID_HANDLE_VALUE */
      if ((read_pipe || !err) && read_pipe != INVALID_HANDLE_VALUE)
	{
	  debug_printf ("pipe read handle %p", read_pipe);
	  break;
	}

      switch (err)
	{
	case ERROR_PIPE_BUSY:
	  /* The pipe is already open with compatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe busy, retrying");
	  continue;
	case ERROR_ACCESS_DENIED:
	  /* The pipe is already open with incompatible parameters.
	     Pick a new name and retry.  */
	  debug_printf ("pipe access denied, retrying");
	  continue;
	case ERROR_CALL_NOT_IMPLEMENTED:
	  /* We are on an older Win9x platform without named pipes.
	     Return an anonymous pipe as the best approximation.  */
	  debug_printf ("CreateNamedPipe not implemented, resorting to "
			"CreatePipe size %lu", psize);
	  if (CreatePipe (read_pipe_ptr, write_pipe_ptr, sa_ptr, psize))
	    {
	      debug_printf ("pipe read handle %p", *read_pipe_ptr);
	      debug_printf ("pipe write handle %p", *write_pipe_ptr);
	      return NO_ERROR;
	    }
	  err = GetLastError ();
	  debug_printf ("CreatePipe failed, %E");
	  return err;
	default:
	  debug_printf ("CreateNamedPipe failed, %E");
	  return err;
	}
      /* NOTREACHED */
    }

  debug_printf ("CreateFile: name %s", pipename);

  /* Open the named pipe for writing.
     Be sure to permit FILE_READ_ATTRIBUTES access.  */
  write_pipe = CreateFile (pipename,
			   GENERIC_WRITE | FILE_READ_ATTRIBUTES,
			   0,       /* share mode */
			   sa_ptr,
			   OPEN_EXISTING,
			   0,       /* flags and attributes */
			   0);      /* handle to template file */

  if (write_pipe == INVALID_HANDLE_VALUE)
    {
      /* Failure. */
      DWORD err = GetLastError ();
      debug_printf ("CreateFile failed, %E");
      CloseHandle (read_pipe);
      return err;
    }

  debug_printf ("pipe write handle %p", write_pipe);

  /* Success. */
  *read_pipe_ptr = read_pipe;
  *write_pipe_ptr = write_pipe;
  return NO_ERROR;
}

int
fhandler_pipe::create (fhandler_pipe *fhs[2], unsigned psize, int mode, bool fifo)
{
  HANDLE r, w;
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;
  int res = -1;
  int ret;

  if ((ret = create_selectable_pipe (&r, &w, sa, psize)) != NO_ERROR)
    __seterrno_from_win_error (ret);
  else
    {
      fhs[0] = (fhandler_pipe *) build_fh_dev (*piper_dev);
      fhs[1] = (fhandler_pipe *) build_fh_dev (*pipew_dev);

      int binmode = mode & O_TEXT ?: O_BINARY;
      fhs[0]->init (r, GENERIC_READ, binmode);
      fhs[1]->init (w, GENERIC_WRITE, binmode);
      if (mode & O_NOINHERIT)
       {
	 fhs[0]->close_on_exec (true);
	 fhs[1]->close_on_exec (true);
       }

      fhs[0]->create_read_state (2);
      fhs[0]->need_fork_fixup (true);
      ProtectHandle1 (fhs[0]->read_state, read_state);

      res = 0;
      fhs[0]->create_guard (sa);
      if (wincap.has_unreliable_pipes ())
	{
	  char buf[80];
	  int count = pipecount++;	/* FIXME: Should this be InterlockedIncrement? */
	  __small_sprintf (buf, pipeid_fmt, myself->pid, count);
	  fhs[1]->writepipe_exists = CreateEvent (sa, TRUE, FALSE, buf);
	  fhs[0]->orig_pid = myself->pid;
	  fhs[0]->id = count;
	}
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

#define DEFAULT_PIPEBUFSIZE (4 * PIPE_BUF)

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
