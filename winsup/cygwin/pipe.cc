/* pipe.cc: pipe for Cygwin.

   Copyright 1996, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "dtable.h"
#include "thread.h"
#include "security.h"

static int
make_pipe (int fildes[2], unsigned int psize, int mode)
{
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," make_pipe");

  HANDLE r, w;
  int  fdr, fdw;
  SECURITY_ATTRIBUTES *sa = (mode & O_NOINHERIT) ?  &sec_none_nih : &sec_none;

  if ((fdr = fdtab.find_unused_handle ()) < 0)
    set_errno (ENMFILE);
  else if ((fdw = fdtab.find_unused_handle (fdr + 1)) < 0)
    set_errno (ENMFILE);
  else if (!CreatePipe (&r, &w, sa, psize))
    __seterrno ();
  else
    {
      fhandler_base *fhr = fdtab.build_fhandler (fdr, FH_PIPER, "/dev/piper");
      fhandler_base *fhw = fdtab.build_fhandler (fdw, FH_PIPEW, "/dev/pipew");

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

      debug_printf ("0 = pipe (%p) (%d:%p, %d:%p)", fildes,
		    fdr, fhr->get_handle (), fdw, fhw->get_handle ());

      ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," make_pipe");
      return 0;
    }

  syscall_printf ("-1 = pipe (%p)", fildes);
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," make_pipe");
  return -1;
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
    fdtab[filedes[0]]->set_r_no_interrupt (1);
  return res;
}

int
dup (int fd)
{
  int res;
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," dup");

  res = dup2 (fd, fdtab.find_unused_handle ());

  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," dup");

  return res;
}

int
dup2 (int oldfd, int newfd)
{
  return fdtab.dup2 (oldfd, newfd);
}
