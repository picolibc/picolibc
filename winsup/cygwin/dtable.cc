/* dtable.cc: file descriptor support.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#define Win32_Winsock
#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <winsock.h>
#include "dtable.h"
#include "pinfo.h"

dtable fdtab;

/* Set aside space for the table of fds */
void
dtable_init (void)
{
  if (!fdtab.size)
    fdtab.extend(NOFILE_INCR);
}

void __stdcall
set_std_handle (int fd)
{
  if (fd == 0)
    SetStdHandle (STD_INPUT_HANDLE, fdtab[fd]->get_handle ());
  else if (fd == 1)
    SetStdHandle (STD_OUTPUT_HANDLE, fdtab[fd]->get_output_handle ());
  else if (fd == 2)
    SetStdHandle (STD_ERROR_HANDLE, fdtab[fd]->get_output_handle ());
}

int
dtable::extend (int howmuch)
{
  int new_size = size + howmuch;
  fhandler_base **newfds;

  if (howmuch <= 0)
    return 0;

  /* Try to allocate more space for fd table. We can't call realloc()
     here to preserve old table if memory allocation fails */

  if (!(newfds = (fhandler_base **) calloc (new_size, sizeof newfds[0])))
    {
      debug_printf ("calloc failed");
      return 0;
    }
  if (fds)
    {
      memcpy (newfds, fds, size * sizeof (fds[0]));
      free (fds);
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
  /* Set these before trying to output anything from strace.
     Also, always set them even if we're to pick up our parent's fds
     in case they're missed.  */

  if (!parent_alive && NOTSTATE(myself, PID_CYGPARENT))
    {
      HANDLE in = GetStdHandle (STD_INPUT_HANDLE);
      HANDLE out = GetStdHandle (STD_OUTPUT_HANDLE);
      HANDLE err = GetStdHandle (STD_ERROR_HANDLE);

      fdtab.init_std_file_from_handle (0, in, GENERIC_READ, "{stdin}");

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

      fdtab.init_std_file_from_handle (1, out, GENERIC_WRITE, "{stdout}");
      fdtab.init_std_file_from_handle (2, err, GENERIC_WRITE, "{stderr}");
    }
}

int
dtable::not_open (int fd)
{
  SetResourceLock(LOCK_FD_LIST,READ_LOCK," not_open");

  int res = fd < 0 || fd >= (int)size || fds[fd] == NULL;

  ReleaseResourceLock(LOCK_FD_LIST,READ_LOCK," not open");
  return res;
}

int
dtable::find_unused_handle (int start)
{
  AssertResourceOwner(LOCK_FD_LIST, READ_LOCK);

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
      delete (fds[fd]);
      fds[fd] = NULL;
    }
}

void
dtable::init_std_file_from_handle (int fd, HANDLE handle,
				  DWORD myaccess, const char *name)
{
  int bin = binmode ? O_BINARY : 0;
  /* Check to see if we're being redirected - if not then
     we open then as consoles */
  if (fd == 0 || fd == 1 || fd == 2)
    {
      first_fd_for_open = 0;
      /* See if we can consoleify it  - if it is a console,
       don't open it in binary.  That will screw up our crlfs*/
      CONSOLE_SCREEN_BUFFER_INFO buf;
      if (GetConsoleScreenBufferInfo (handle, &buf))
	{
	  bin = 0;
	  if (ISSTATE (myself, PID_USETTY))
	    name = "/dev/tty";
	  else
	    name = "/dev/conout";
	}
      else if (FlushConsoleInputBuffer (handle))
	{
	  bin = 0;
	  if (ISSTATE (myself, PID_USETTY))
	    name = "/dev/tty";
	  else
	    name = "/dev/conin";
	}
    }

  build_fhandler (fd, name, handle)->init (handle, myaccess, bin);
  set_std_handle (fd);
  paranoid_printf ("fd %d, handle %p", fd, handle);
}

extern "C"
int
cygwin_attach_handle_to_fd (char *name, int fd, HANDLE handle, mode_t bin,
			      DWORD myaccess)
{
  if (fd == -1)
    fd = fdtab.find_unused_handle();
  fhandler_base *res = fdtab.build_fhandler (fd, name, handle);
  res->init (handle, myaccess, bin);
  return fd;
}

fhandler_base *
dtable::build_fhandler (int fd, const char *name, HANDLE handle)
{
  int unit;
  DWORD devn;

  if ((devn = get_device_number (name, unit)) == FH_BAD)
    {
      struct sockaddr sa;
      int sal = sizeof (sa);
      CONSOLE_SCREEN_BUFFER_INFO cinfo;
      DCB dcb;

      if (handle == NULL)
	devn = FH_DISK;
      else if (GetNumberOfConsoleInputEvents (handle, (DWORD *) &cinfo))
	devn = FH_CONIN;
      else if (GetConsoleScreenBufferInfo (handle, &cinfo))
	devn= FH_CONOUT;
      else if (wsock32_handle && getpeername ((SOCKET) handle, &sa, &sal))
	devn = FH_SOCKET;
      else if (GetFileType (handle) == FILE_TYPE_PIPE)
	devn = FH_PIPE;
      else if (GetCommState (handle, &dcb))
	devn = FH_SERIAL;
      else
	devn = FH_DISK;
    }

  return build_fhandler (fd, devn, name, unit);
}

fhandler_base *
dtable::build_fhandler (int fd, DWORD dev, const char *name, int unit)
{
  fhandler_base *fh;
  void *buf = calloc (1, sizeof (fhandler_union) + 100);

  dev &= FH_DEVMASK;
  switch (dev)
    {
      case FH_TTYM:
	fh = new (buf) fhandler_tty_master (name, unit);
	break;
      case FH_CONSOLE:
      case FH_CONIN:
      case FH_CONOUT:
	fh = new (buf) fhandler_console (name);
	break;
      case FH_PTYM:
	fh = new (buf) fhandler_pty_master (name);
	break;
      case FH_TTYS:
	if (unit < 0)
	  fh = new (buf) fhandler_tty_slave (name);
	else
	  fh = new (buf) fhandler_tty_slave (unit, name);
	break;
      case FH_WINDOWS:
	fh = new (buf) fhandler_windows (name);
	break;
      case FH_SERIAL:
	fh = new (buf) fhandler_serial (name, dev, unit);
	break;
      case FH_PIPE:
      case FH_PIPER:
      case FH_PIPEW:
	fh = new (buf) fhandler_pipe (name, dev);
	break;
      case FH_SOCKET:
	fh = new (buf) fhandler_socket (name);
	break;
      case FH_DISK:
	fh = new (buf) fhandler_disk_file (NULL);
	break;
      case FH_FLOPPY:
	fh = new (buf) fhandler_dev_floppy (name, unit);
	break;
      case FH_TAPE:
	fh = new (buf) fhandler_dev_tape (name, unit);
	break;
      case FH_NULL:
	fh = new (buf) fhandler_dev_null (name);
	break;
      case FH_ZERO:
	fh = new (buf) fhandler_dev_zero (name);
	break;
      case FH_RANDOM:
	fh = new (buf) fhandler_dev_random (name, unit);
	break;
      default:
	/* FIXME - this could recurse forever */
	return build_fhandler (fd, name, NULL);
    }

  debug_printf ("%s - cb %d, fd %d, fh %p", fh->get_name () ?: "", fh->cb,
		fd, fh);
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
      free (newfh);
      newfh = NULL;
      return NULL;
    }

  newfh->set_close_on_exec_flag (0);
  MALLOC_CHECK;
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

  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  if ((size_t) newfd >= fdtab.size || newfd < 0)
    {
      syscall_printf ("new fd out of bounds: %d", newfd);
      set_errno (EBADF);
      goto done;
    }
  if (!not_open (newfd))
    _close (newfd);
  fds[newfd] = newfh;
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  MALLOC_CHECK;

  if ((res = newfd) <= 2)
    set_std_handle (res);

  MALLOC_CHECK;
done:
  syscall_printf ("%d = dup2 (%d, %d)", res, oldfd, newfd);

  return res;
}

select_record *
dtable::select_read (int fd, select_record *s)
{
  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fdtab[fd];
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
  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fdtab[fd];
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
  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fdtab[fd];
  s = fh->select_except (s);
  s->fd = fd;
  s->fh = fh;
  s->saw_error = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return s;
}

/*
 * Function to take an existant dtable array
 * and linearize it into a memory buffer.
 * If memory buffer is NULL, it returns the size
 * of memory buffer needed to do the linearization.
 * On error returns -1.
 */

int
dtable::linearize_fd_array (unsigned char *in_buf, int buflen)
{
  /* If buf == NULL, just precalculate length */
  if (in_buf == NULL)
    {
      buflen = sizeof (size_t);
      for (int i = 0, max_used_fd = -1; i < (int)size; i++)
	if (!not_open (i) && !fds[i]->get_close_on_exec ())
	  {
	    buflen += i - (max_used_fd + 1);
	    buflen += fds[i]->cb + strlen (fds[i]->get_name ()) + 1
				 + strlen (fds[i]->get_win32_name ()) + 1;
	    max_used_fd = i;
	  }
      debug_printf ("needed buflen %d", buflen);
      return buflen;
    }

  debug_printf ("in_buf = %x, buflen = %d", in_buf, buflen);

  /*
   * Now linearize each open fd (write a 0xff byte for a closed fd).
   * Write the name of the open fd first (null terminated). This
   * allows the de_linearizeing code to determine what kind of fhandler_xxx
   * to create.
   */

  size_t i;
  int len, total_size;

  total_size = sizeof (size_t);
  if (total_size > buflen)
    {
      system_printf ("FATAL: linearize_fd_array exceeded buffer size");
      return -1;
    }

  unsigned char *buf = in_buf;
  buf += sizeof (size_t);	/* skip over length which is added later */

  for (i = 0, total_size = sizeof (size_t); total_size < buflen; i++)
    {
      if (not_open (i) || fds[i]->get_close_on_exec ())
	{
	  debug_printf ("linearizing closed fd %d",i);
	  *buf = 0xff;		/* place holder */
	  len = 1;
	}
      else
	{
	  len = fds[i]->linearize (buf);
	  debug_printf ("fd %d, len %d, name %s, device %p", i, len, buf,
			fds[i]->get_device ());
	}

      total_size += len;
      buf += len;
    }

  i--;
  memcpy (in_buf, &i, sizeof (size_t));
  if (total_size != buflen)
    system_printf ("out of sync %d != %d", total_size, buflen);
  return total_size;
}

/*
 * Function to take a linearized dtable array in a memory buffer and
 * re-create the original dtable array.
 */

LPBYTE
dtable::de_linearize_fd_array (LPBYTE buf)
{
  int len, max_used_fd;
  size_t inc_size;

  debug_printf ("buf %x", buf);

  /* First get the number of fd's - use this to set the fdtabsize.
     NB. This is the only place in the code this should be done !!
  */

  memcpy ((char *) &max_used_fd, buf, sizeof (int));
  buf += sizeof (size_t);

  inc_size = NOFILE_INCR * ((max_used_fd + NOFILE_INCR - 1) / NOFILE_INCR) -
	     size;
  debug_printf ("max_used_fd %d, inc size %d", max_used_fd, inc_size);
  if (inc_size > 0 && !extend (inc_size))
    {
      system_printf ("out of memory");
      return NULL;
    }

  for (int i = 0; i <= max_used_fd; i++)
    {
      /* 0xFF means closed */
      if (*buf == 0xff)
	{
	  fds[i] = NULL;
	  buf++;
	  debug_printf ("closed fd %d", i);
	  continue;
	}
      /* fd was open - de_linearize it */
      /* Get the null-terminated name.  It is followed by an image of
	 the actual fhandler_* structure.  Use the status field from
	 this to build a new fhandler type. */

      DWORD status;
      LPBYTE obuf = buf;
      char *win32;
      win32 = strchr ((char *)obuf, '\0') + 1;
      buf = (LPBYTE)strchr ((char *)win32, '\0') + 1;
      memcpy ((char *)&status, buf + FHSTATOFF, sizeof(DWORD));
      debug_printf ("fd %d, name %s, win32 name %s, status %p",
		    i, obuf, win32, status);
      len = build_fhandler (i, status, (const char *) NULL)->
	    de_linearize ((char *) buf, (char *) obuf, win32);
      set_std_handle (i);
      buf += len;
      debug_printf ("len %d", buf - obuf);
    }
  first_fd_for_open = 0;
  return buf;
}

void
dtable::fixup_after_fork (HANDLE parent)
{
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  for (size_t i = 0; i < size; i++)
    if (!not_open (i))
      {
	fhandler_base *fh = fds[i];
	if (fh->get_close_on_exec () || fh->get_need_fork_fixup ())
	  {
	    debug_printf ("fd %d(%s)", i, fh->get_name ());
	    fh->fixup_after_fork (parent);
	  }
      }
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
}

int
dtable::vfork_child_dup ()
{
  fhandler_base **newtable;
  newtable = (fhandler_base **) calloc (size, sizeof(fds[0]));
  int res = 1;

  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  for (size_t i = 0; i < size; i++)
    if (not_open (i))
      continue;
    else if ((newtable[i] = dup_worker (fds[i])) == NULL)
      {
	res = 0;
	set_errno (EBADF);
	goto out;
      }
  fds_on_hold = fds;
  fds = newtable;
out:
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  return 1;
}

void
dtable::vfork_parent_restore ()
{
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");

  close_all_files ();
  fhandler_base **deleteme = fds;
  fds = fds_on_hold;
  fds_on_hold = NULL;
  free (deleteme);

  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK,"dup");
  return;
}
