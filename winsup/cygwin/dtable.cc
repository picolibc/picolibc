/* dtable.cc: file descriptor support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/cygwin.h>
#include <assert.h>

#define USE_SYS_TYPES_FD_SET
#include <winsock.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"

static const NO_COPY DWORD std_consts[] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE,
			     STD_ERROR_HANDLE};

/* Set aside space for the table of fds */
void
dtable_init (void)
{
  if (!cygheap->fdtab.size)
    cygheap->fdtab.extend (NOFILE_INCR);
}

void __stdcall
set_std_handle (int fd)
{
  if (fd == 0)
    SetStdHandle (std_consts[fd], cygheap->fdtab[fd]->get_handle ());
  else if (fd <= 2)
    SetStdHandle (std_consts[fd], cygheap->fdtab[fd]->get_output_handle ());
}

void
dtable::dec_console_fds ()
{
  if (console_fds > 0 && !--console_fds &&
      myself->ctty != TTY_CONSOLE && !check_pty_fds())
    FreeConsole ();
}

int
dtable::extend (int howmuch)
{
  int new_size = size + howmuch;
  fhandler_base **newfds;

  if (howmuch <= 0)
    return 0;

  /* Try to allocate more space for fd table. We can't call realloc ()
     here to preserve old table if memory allocation fails */

  if (!(newfds = (fhandler_base **) ccalloc (HEAP_ARGV, new_size, sizeof newfds[0])))
    {
      debug_printf ("calloc failed");
      return 0;
    }
  if (fds)
    {
      memcpy (newfds, fds, size * sizeof (fds[0]));
      cfree (fds);
    }

  size = new_size;
  fds = newfds;
  debug_printf ("size %d, fds %p", size, fds);
  return 1;
}

/* Initialize the file descriptor/handle mapping table.
   We only initialize the parent table here.  The child table is
   initialized at each fork () call.  */

void
stdio_init (void)
{
  extern void set_console_ctty ();
  /* Set these before trying to output anything from strace.
     Also, always set them even if we're to pick up our parent's fds
     in case they're missed.  */

  if (!myself->ppid_handle && NOTSTATE (myself, PID_CYGPARENT))
    {
      HANDLE in = GetStdHandle (STD_INPUT_HANDLE);
      HANDLE out = GetStdHandle (STD_OUTPUT_HANDLE);
      HANDLE err = GetStdHandle (STD_ERROR_HANDLE);

      cygheap->fdtab.init_std_file_from_handle (0, in, GENERIC_READ);

      /* STD_ERROR_HANDLE has been observed to be the same as
	 STD_OUTPUT_HANDLE.  We need separate handles (e.g. using pipes
	 to pass data from child to parent).  */
      if (out == err)
	{
	  /* Since this code is not invoked for forked tasks, we don't have
	     to worry about the close-on-exec flag here.  */
	  if (!DuplicateHandle (hMainProc, out, hMainProc, &err, 0,
				 1, DUPLICATE_SAME_ACCESS))
	    {
	      /* If that fails, do this as a fall back.  */
	      err = out;
	      system_printf ("couldn't make stderr distinct from stdout");
	    }
	}

      cygheap->fdtab.init_std_file_from_handle (1, out, GENERIC_WRITE);
      cygheap->fdtab.init_std_file_from_handle (2, err, GENERIC_WRITE);
      /* Assign the console as the controlling tty for this process if we actually
	 have a console and no other controlling tty has been assigned. */
      if (myself->ctty < 0 && GetConsoleCP () > 0)
	set_console_ctty ();
    }
}

int
dtable::find_unused_handle (int start)
{
  AssertResourceOwner (LOCK_FD_LIST, READ_LOCK);

  do
    {
      for (int i = start; i < (int) size; i++)
	/* See if open -- no need for overhead of not_open */
	if (fds[i] == NULL)
	  return i;
    }
  while (extend (NOFILE_INCR));
  return -1;
}

void
dtable::release (int fd)
{
  if (!not_open (fd))
    {
      switch (fds[fd]->get_device ())
	{
	case FH_SOCKET:
	  dec_need_fixup_before ();
	  break;
	case FH_CONSOLE:
	  dec_console_fds ();
	  break;
	}
      delete fds[fd];
      fds[fd] = NULL;
    }
}

extern "C"
int
cygwin_attach_handle_to_fd (char *name, int fd, HANDLE handle, mode_t bin,
			    DWORD myaccess)
{
  if (fd == -1)
    fd = cygheap->fdtab.find_unused_handle ();
  path_conv pc;
  fhandler_base *res = cygheap->fdtab.build_fhandler_from_name (fd, name, handle,
								pc);
  res->init (handle, myaccess, bin);
  return fd;
}

void
dtable::init_std_file_from_handle (int fd, HANDLE handle, DWORD myaccess)
{
  int bin;
  const char *name;
  CONSOLE_SCREEN_BUFFER_INFO buf;
  struct sockaddr sa;
  int sal = sizeof (sa);
  DCB dcb;

  first_fd_for_open = 0;

  if (!handle || handle == INVALID_HANDLE_VALUE)
    return;

  if (__fmode)
    bin = __fmode;
  else
    bin = binmode ?: 0;

  /* See if we can consoleify it  - if it is a console,
   don't open it in binary.  That will screw up our crlfs*/
  if (GetConsoleScreenBufferInfo (handle, &buf))
    {
      if (ISSTATE (myself, PID_USETTY))
	name = "/dev/tty";
      else
	name = "/dev/conout";
      bin = 0;
    }
  else if (GetNumberOfConsoleInputEvents (handle, (DWORD *) &buf))
    {
      if (ISSTATE (myself, PID_USETTY))
	name = "/dev/tty";
      else
	name = "/dev/conin";
      bin = 0;
    }
  else if (GetFileType (handle) == FILE_TYPE_PIPE)
    {
      if (fd == 0)
	name = "/dev/piper";
      else
	name = "/dev/pipew";
      if (bin == 0)
	bin = O_BINARY;
    }
  else if (wsock_started && getpeername ((SOCKET) handle, &sa, &sal) == 0)
    name = "/dev/socket";
  else if (GetCommState (handle, &dcb))
    name = "/dev/ttyS0"; // FIXME - determine correct device
  else
    name = "unknown disk file";

  path_conv pc;
  build_fhandler_from_name (fd, name, handle, pc)->init (handle, myaccess, bin);
  set_std_handle (fd);
  paranoid_printf ("fd %d, handle %p", fd, handle);
}

fhandler_base *
dtable::build_fhandler_from_name (int fd, const char *name, HANDLE handle,
				  path_conv& pc, unsigned opt, suffix_info *si)
{
  pc.check (name, opt | PC_NULLEMPTY | PC_FULL, si);
  if (pc.error)
    {
      set_errno (pc.error);
      return NULL;
    }

  fhandler_base *fh = build_fhandler (fd, pc.get_devn (), name, pc.get_unitn ());
  fh->set_name (name, pc, pc.get_unitn ());
  return fh;
}

#define cnew(name) new ((void *) ccalloc (HEAP_FHANDLER, 1, sizeof (name))) name
fhandler_base *
dtable::build_fhandler (int fd, DWORD dev, const char *name, int unit)
{
  fhandler_base *fh;

  dev &= FH_DEVMASK;
  switch (dev)
    {
      case FH_TTYM:
	fh = cnew (fhandler_tty_master) (unit);
	break;
      case FH_CONSOLE:
      case FH_CONIN:
      case FH_CONOUT:
	if ((fh = cnew (fhandler_console) ()))
	  inc_console_fds ();
	break;
      case FH_PTYM:
	fh = cnew (fhandler_pty_master) ();
	break;
      case FH_TTYS:
	if (unit < 0)
	  fh = cnew (fhandler_tty_slave) ();
	else
	  fh = cnew (fhandler_tty_slave) (unit);
	break;
      case FH_WINDOWS:
	fh = cnew (fhandler_windows) ();
	break;
      case FH_SERIAL:
	fh = cnew (fhandler_serial) (unit);
	break;
      case FH_PIPE:
      case FH_PIPER:
      case FH_PIPEW:
	fh = cnew (fhandler_pipe) (dev);
	break;
      case FH_SOCKET:
	if ((fh = cnew (fhandler_socket) ()))
	  inc_need_fixup_before ();
	break;
      case FH_DISK:
	fh = cnew (fhandler_disk_file) ();
	break;
      case FH_CYGDRIVE:
	fh = cnew (fhandler_cygdrive) (unit);
	break;
      case FH_FLOPPY:
	fh = cnew (fhandler_dev_floppy) (unit);
	break;
      case FH_TAPE:
	fh = cnew (fhandler_dev_tape) (unit);
	break;
      case FH_NULL:
	fh = cnew (fhandler_dev_null) ();
	break;
      case FH_ZERO:
	fh = cnew (fhandler_dev_zero) ();
	break;
      case FH_RANDOM:
	fh = cnew (fhandler_dev_random) (unit);
	break;
      case FH_MEM:
	fh = cnew (fhandler_dev_mem) (unit);
	break;
      case FH_CLIPBOARD:
	fh = cnew (fhandler_dev_clipboard) ();
	break;
      case FH_OSS_DSP:
	fh = cnew (fhandler_dev_dsp) ();
	break;
      default:
	system_printf ("internal error -- unknown device - %p", dev);
	fh = NULL;
    }

  debug_printf ("fd %d, fh %p", fd, fh);
  return fd >= 0 ? (fds[fd] = fh) : fh;
}

fhandler_base *
dtable::dup_worker (fhandler_base *oldfh)
{
  fhandler_base *newfh = build_fhandler (-1, oldfh->get_device (), NULL);
  *newfh = *oldfh;
  newfh->set_io_handle (NULL);
  if (oldfh->dup (newfh))
    {
      cfree (newfh);
      newfh = NULL;
      return NULL;
    }

  newfh->set_close_on_exec_flag (0);
  MALLOC_CHECK;
  debug_printf ("duped '%s' old %p, new %p", oldfh->get_name (), oldfh->get_io_handle (), newfh->get_io_handle ());
  return newfh;
}

int
dtable::dup2 (int oldfd, int newfd)
{
  int res = -1;
  fhandler_base *newfh = NULL;	// = NULL to avoid an incorrect warning

  MALLOC_CHECK;
  debug_printf ("dup2 (%d, %d)", oldfd, newfd);

  if (not_open (oldfd))
    {
      syscall_printf ("fd %d not open", oldfd);
      set_errno (EBADF);
      goto done;
    }

  if (newfd == oldfd)
    {
      res = 0;
      goto done;
    }

  if ((newfh = dup_worker (fds[oldfd])) == NULL)
    {
      res = -1;
      goto done;
    }

  debug_printf ("newfh->io_handle %p, oldfh->io_handle %p", newfh->get_io_handle (), fds[oldfd]->get_io_handle ());
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");

  if (newfd < 0)
    {
      syscall_printf ("new fd out of bounds: %d", newfd);
      set_errno (EBADF);
      goto done;
    }

  if ((size_t) newfd >= size)
   {
     int inc_size = NOFILE_INCR * ((newfd + NOFILE_INCR - 1) / NOFILE_INCR) -
		    size;
     extend (inc_size);
   }

  if (!not_open (newfd))
    _close (newfd);
  fds[newfd] = newfh;

  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");
  MALLOC_CHECK;

  if ((res = newfd) <= 2)
    set_std_handle (res);

  MALLOC_CHECK;
done:
  syscall_printf ("%d = dup2 (%d, %d)", res, oldfd, newfd);

  return res;
}

void
dtable::reset_unix_path_name (int fd, const char *name)
{
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "reset_unix_name");
  fds[fd]->reset_unix_path_name (name);
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "reset_unix_name");
}

select_record *
dtable::select_read (int fd, select_record *s)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fds[fd];
  s = fh->select_read (s);
  s->fd = fd;
  s->fh = fh;
  s->saw_error = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return s;
}

select_record *
dtable::select_write (int fd, select_record *s)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fds[fd];
  s = fh->select_write (s);
  s->fd = fd;
  s->fh = fh;
  s->saw_error = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return s;
}

select_record *
dtable::select_except (int fd, select_record *s)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fds[fd];
  s = fh->select_except (s);
  s->fd = fd;
  s->fh = fh;
  s->saw_error = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return s;
}

/* Function to walk the fd table after an exec and perform
   per-fhandler type fixups. */
void
dtable::fixup_before_fork (DWORD target_proc_id)
{
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "fixup_before_fork");
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	debug_printf ("fd %d (%s)", i, fh->get_name ());
	fh->fixup_before_fork_exec (target_proc_id);
      }
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "fixup_before_fork");
}

void
dtable::fixup_before_exec (DWORD target_proc_id)
{
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "fixup_before_exec");
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL && !fh->get_close_on_exec ())
      {
	debug_printf ("fd %d (%s)", i, fh->get_name ());
	fh->fixup_before_fork_exec (target_proc_id);
      }
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "fixup_before_exec");
}

void
dtable::fixup_after_exec (HANDLE parent)
{
  first_fd_for_open = 0;
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	fh->clear_readahead ();
	if (fh->get_close_on_exec ())
	  release (i);
	else
	  {
	    fh->fixup_after_exec (parent);
	    if (i == 0)
	      SetStdHandle (std_consts[i], fh->get_io_handle ());
	    else if (i <= 2)
	      SetStdHandle (std_consts[i], fh->get_output_handle ());
	  }
      }
}

void
dtable::fixup_after_fork (HANDLE parent)
{
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	if (fh->get_close_on_exec () || fh->get_need_fork_fixup ())
	  {
	    debug_printf ("fd %d (%s)", i, fh->get_name ());
	    fh->fixup_after_fork (parent);
	  }
	if (i == 0)
	  SetStdHandle (std_consts[i], fh->get_io_handle ());
	else if (i <= 2)
	  SetStdHandle (std_consts[i], fh->get_output_handle ());
      }
}

int
dtable::vfork_child_dup ()
{
  fhandler_base **newtable;
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");
  newtable = (fhandler_base **) ccalloc (HEAP_ARGV, size, sizeof (fds[0]));
  int res = 1;

  /* Remove impersonation */
  if (cygheap->user.impersonated && cygheap->user.token != INVALID_HANDLE_VALUE)
    RevertToSelf ();

  for (size_t i = 0; i < size; i++)
    if (not_open (i))
      continue;
    else if ((newtable[i] = dup_worker (fds[i])) != NULL)
      newtable[i]->set_close_on_exec (fds[i]->get_close_on_exec ());
    else
      {
	res = 0;
	set_errno (EBADF);
	goto out;
      }

  /* Restore impersonation */
  if (cygheap->user.impersonated && cygheap->user.token != INVALID_HANDLE_VALUE)
    ImpersonateLoggedOnUser (cygheap->user.token);

  fds_on_hold = fds;
  fds = newtable;

out:
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "dup");
  return 1;
}

void
dtable::vfork_parent_restore ()
{
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "restore");

  close_all_files ();
  fhandler_base **deleteme = fds;
  assert (fds_on_hold != NULL);
  fds = fds_on_hold;
  fds_on_hold = NULL;
  cfree (deleteme);

  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK | READ_LOCK, "restore");
  return;
}

void
dtable::vfork_child_fixup ()
{
  if (!fds_on_hold)
    return;
  debug_printf ("here");
  fhandler_base **saveme = fds;
  fds = fds_on_hold;

  fhandler_base *fh;
  for (int i = 0; i < (int) cygheap->fdtab.size; i++)
    if ((fh = cygheap->fdtab[i]) != NULL)
      {
	fh->clear_readahead ();
	if (fh->get_close_on_exec ())
	  release (i);
	else
	  {
	    fh->close ();
	    cygheap->fdtab.release (i);
	  }
      }

  fds = saveme;
  cfree (fds_on_hold);
  fds_on_hold = NULL;

  return;
}
