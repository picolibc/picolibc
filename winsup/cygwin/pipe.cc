/* pipe.cc: pipe for Cygwin.

   Copyright 1996, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "thread.h"
#include "sigproc.h"
#include "pinfo.h"

static unsigned pipecount;
static const NO_COPY char pipeid_fmt[] = "stupid_pipe.%u.%u";

fhandler_pipe::fhandler_pipe (const char *name, DWORD devtype) :
	fhandler_base (devtype, name),
	guard (0), writepipe_exists(0), orig_pid (0), id (0)
{
  set_cb (sizeof *this);
}

off_t
fhandler_pipe::lseek (off_t offset, int whence)
{
  debug_printf ("(%d, %d)", offset, whence);
  set_errno (ESPIPE);
  return -1;
}

void
fhandler_pipe::set_close_on_exec (int val)
{
  this->fhandler_base::set_close_on_exec (val);
  if (guard)
    set_inheritance (guard, val);
  if (writepipe_exists)
    set_inheritance (writepipe_exists, val);
}

int
fhandler_pipe::read (void *in_ptr, size_t in_len)
{
  int res = this->fhandler_base::read (in_ptr, in_len);
  ReleaseMutex (guard);
  return res;
}

int fhandler_pipe::close ()
{
  int res = this->fhandler_base::close ();
  if (guard)
    CloseHandle (guard);
  if (writepipe_exists)
    CloseHandle (writepipe_exists);
  return res;
}

bool
fhandler_pipe::hit_eof ()
{
  char buf[80];
  HANDLE ev;
  if (!orig_pid)
    return false;
  __small_sprintf (buf, pipeid_fmt, orig_pid, id);
  if ((ev = OpenEvent (EVENT_ALL_ACCESS, FALSE, buf)))
    CloseHandle (ev);
  debug_printf ("%s %p", buf, ev);
  return ev == NULL;
}

void
fhandler_pipe::fixup_after_fork (HANDLE parent)
{
  this->fhandler_base::fixup_after_fork (parent);
  if (guard)
    fork_fixup (parent, guard, "guard");
  if (writepipe_exists)
    fork_fixup (parent, writepipe_exists, "guard");
}

int
fhandler_pipe::dup (fhandler_base *child)
{
  int res = this->fhandler_base::dup (child);
  if (res)
    return res;

  fhandler_pipe *ftp = (fhandler_pipe *) child;

  if (guard == NULL)
    ftp->guard = NULL;
  else if (!DuplicateHandle (hMainProc, guard, hMainProc, &ftp->guard, 0, 1,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate guard %p, %E", guard);
      return -1;
    }

  if (writepipe_exists == NULL)
    ftp->writepipe_exists = NULL;
  else if (!DuplicateHandle (hMainProc, writepipe_exists, hMainProc,
			     &ftp->writepipe_exists, 0, 1,
			     DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("couldn't duplicate writepipe_exists %p, %E", writepipe_exists);
      return -1;
    }

  ftp->id = id;
  ftp->orig_pid = orig_pid;
  return 0;
}


int
make_pipe (int fildes[2], unsigned int psize, int mode)
{
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "make_pipe");

  HANDLE r, w;
  int  fdr = -1, fdw = -1;
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;
  int res = -1;

  if ((fdr = cygheap->fdtab.find_unused_handle ()) < 0)
    set_errno (ENMFILE);
  else if ((fdw = cygheap->fdtab.find_unused_handle (fdr + 1)) < 0)
    set_errno (ENMFILE);
  else if (!CreatePipe (&r, &w, sa, psize))
    __seterrno ();
  else
    {
      fhandler_pipe *fhr = (fhandler_pipe *) cygheap->fdtab.build_fhandler (fdr, FH_PIPER, "/dev/piper");
      fhandler_pipe *fhw = (fhandler_pipe *) cygheap->fdtab.build_fhandler (fdw, FH_PIPEW, "/dev/pipew");

      int binmode = mode & O_TEXT ? 0 : 1;
      fhr->init (r, GENERIC_READ, binmode);
      fhw->init (w, GENERIC_WRITE, binmode);
      if (mode & O_NOINHERIT)
       {
	 fhr->set_close_on_exec_flag (1);
	 fhw->set_close_on_exec_flag (1);
       }

      fildes[0] = fdr;
      fildes[1] = fdw;

      res = 0;
      fhr->create_guard (sa);
      if (wincap.has_unreliable_pipes ())
	{
	  char buf[80];
	  int count = pipecount++;	/* FIXME: Should this be InterlockedIncrement? */
	  __small_sprintf (buf, pipeid_fmt, myself->pid, count);
	  fhw->writepipe_exists = CreateEvent (sa, TRUE, FALSE, buf);
	  fhr->orig_pid = myself->pid;
	  fhr->id = count;
	}
    }

  syscall_printf ("%d = make_pipe ([%d, %d], %d, %p)", res, fdr, fdw, psize, mode);
  ReleaseResourceLock(LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "make_pipe");
  return res;
}

extern "C" int
pipe (int filedes[2])
{
  extern DWORD binmode;
  return make_pipe (filedes, 16384, (!binmode || binmode == O_BINARY) ? O_BINARY : O_TEXT);
}

extern "C" int
_pipe (int filedes[2], unsigned int psize, int mode)
{
  int res = make_pipe (filedes, psize, mode);
  /* This type of pipe is not interruptible so set the appropriate flag. */
  if (!res)
    cygheap->fdtab[filedes[0]]->set_r_no_interrupt (1);
  return res;
}
