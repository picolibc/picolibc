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
#include "dtable.h"
#include "cygheap.h"
#include "thread.h"

fhandler_pipe::fhandler_pipe (const char *name, DWORD devtype) :
	fhandler_base (devtype, name), guard (0)
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
  set_inheritance (guard, val);
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
  return res;
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
    return -1;
  return 0;
}

static int
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

int
dup (int fd)
{
  int res;
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");

  res = dup2 (fd, cygheap->fdtab.find_unused_handle ());

  ReleaseResourceLock(LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");

  return res;
}

int
dup2 (int oldfd, int newfd)
{
  return cygheap->fdtab.dup2 (oldfd, newfd);
}
