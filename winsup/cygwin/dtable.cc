/* dtable.cc: file descriptor support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/cygwin.h>
#include <assert.h>
#include <ntdef.h>
#include <winnls.h>

#define USE_SYS_TYPES_FD_SET
#include <winsock.h>
#include "pinfo.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "ntdll.h"
#include "shared_info.h"

static const char NO_COPY unknown_file[] = "some disk file";

static const NO_COPY DWORD std_consts[] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE,
					   STD_ERROR_HANDLE};

static const char *handle_to_fn (HANDLE, char *);

#define DEVICE_PREFIX "\\device\\"
#define DEVICE_PREFIX_LEN sizeof (DEVICE_PREFIX) - 1
#define REMOTE "\\Device\\LanmanRedirector\\"
#define REMOTE_LEN sizeof (REMOTE) - 1
#define REMOTE1 "\\Device\\WinDfs\\Root\\"
#define REMOTE1_LEN sizeof (REMOTE1) - 1
#define NAMED_PIPE "\\Device\\NamedPipe\\"
#define NAMED_PIPE_LEN sizeof (NAMED_PIPE) - 1
#define POSIX_NAMED_PIPE "/Device/NamedPipe/"
#define POSIX_NAMED_PIPE_LEN sizeof (POSIX_NAMED_PIPE) - 1

/* Set aside space for the table of fds */
void
dtable_init ()
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

int
dtable::extend (int howmuch)
{
  int new_size = size + howmuch;
  fhandler_base **newfds;

  if (howmuch <= 0)
    return 0;

  if (new_size > (100 * NOFILE_INCR))
    {
      set_errno (EMFILE);
      return 0;
    }

  /* Try to allocate more space for fd table. We can't call realloc ()
     here to preserve old table if memory allocation fails */

  if (!(newfds = (fhandler_base **) ccalloc (HEAP_ARGV, new_size, sizeof newfds[0])))
    {
      debug_printf ("calloc failed");
      set_errno (ENOMEM);
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

void
dtable::get_debugger_info ()
{
  if (being_debugged ())
    {
      char std[3][sizeof ("/dev/ttyNNNN")];
      std[0][0] = std[1][0] = std [2][0] = '\0';
      char buf[sizeof ("cYgstd %x") + 32];
      sprintf (buf, "cYgstd %x %x %x", (unsigned) &std, sizeof (std[0]), 3);
      OutputDebugString (buf);
      for (int i = 0; i < 3; i++)
	if (std[i][0])
	  {
	    HANDLE h = GetStdHandle (std_consts[i]);
	    fhandler_base *fh = build_fh_name (std[i]);
	    if (!fh)
	      continue;
	    fds[i] = fh;
	    if (!fh->open ((i ? (i == 2 ? O_RDWR : O_WRONLY) : O_RDONLY)
			   | O_BINARY, 0777))
	      release (i);
	    else
	      CloseHandle (h);
	  }
    }
}

/* Initialize the file descriptor/handle mapping table.
   This function should only be called when a cygwin function is invoked
   by a non-cygwin function, i.e., it should only happen very rarely. */

void
dtable::stdio_init ()
{
  extern void set_console_ctty ();
  /* Set these before trying to output anything from strace.
     Also, always set them even if we're to pick up our parent's fds
     in case they're missed.  */

  if (myself->cygstarted || ISSTATE (myself, PID_CYGPARENT))
    {
      tty_min *t = cygwin_shared->tty.get_tty (myself->ctty);
      if (t && t->getpgid () == myself->pid && t->gethwnd ())
	init_console_handler (true);
      return;
    }

  HANDLE in = GetStdHandle (STD_INPUT_HANDLE);
  HANDLE out = GetStdHandle (STD_OUTPUT_HANDLE);
  HANDLE err = GetStdHandle (STD_ERROR_HANDLE);

  init_std_file_from_handle (0, in);

  /* STD_ERROR_HANDLE has been observed to be the same as
     STD_OUTPUT_HANDLE.  We need separate handles (e.g. using pipes
     to pass data from child to parent).  */
  if (out == err)
    {
      /* Since this code is not invoked for forked tasks, we don't have
	 to worry about the close-on-exec flag here.  */
      if (!DuplicateHandle (hMainProc, out, hMainProc, &err, 0, true,
			    DUPLICATE_SAME_ACCESS))
	{
	  /* If that fails, do this as a fall back.  */
	  err = out;
	  system_printf ("couldn't make stderr distinct from stdout");
	}
    }

  init_std_file_from_handle (1, out);
  init_std_file_from_handle (2, err);

  /* Assign the console as the controlling tty for this process if we actually
     have a console and no other controlling tty has been assigned. */
  if (!fhandler_console::need_invisible () && myself->ctty < 0)
    set_console_ctty ();
}

const int dtable::initial_archetype_size;

fhandler_base *
dtable::find_archetype (device& dev)
{
  for (unsigned i = 0; i < farchetype; i++)
    if (archetypes[i]->get_device () == (unsigned) dev)
      return archetypes[i];
  return NULL;
}

fhandler_base **
dtable::add_archetype ()
{
  if (farchetype++ >= narchetypes)
    archetypes = (fhandler_base **) crealloc_abort (archetypes, (narchetypes += initial_archetype_size) * sizeof archetypes[0]);
  return archetypes + farchetype - 1;
}

void
dtable::delete_archetype (fhandler_base *fh)
{
  for (unsigned i = 0; i < farchetype; i++)
    if (fh == archetypes[i])
      {
	debug_printf ("deleting element %d for %s", i, fh->get_name ());
	if (i < --farchetype)
	  archetypes[i] = archetypes[farchetype];
	break;
      }

  delete fh;
}

int
dtable::find_unused_handle (int start)
{
  do
    {
      for (size_t i = start; i < size; i++)
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
      if (fds[fd]->need_fixup_before ())
	dec_need_fixup_before ();
      fhandler_base *arch = fds[fd]->archetype;
      delete fds[fd];
      if (arch && !arch->usecount)
	cygheap->fdtab.delete_archetype (arch);
      fds[fd] = NULL;
    }
}

extern "C" int
cygwin_attach_handle_to_fd (char *name, int fd, HANDLE handle, mode_t bin,
			    DWORD myaccess)
{
  if (fd == -1)
    fd = cygheap->fdtab.find_unused_handle ();
  fhandler_base *fh = build_fh_name (name);
  cygheap->fdtab[fd] = fh;
  fh->init (handle, myaccess, bin ?: fh->pc_binmode ());
  return fd;
}

void
dtable::init_std_file_from_handle (int fd, HANDLE handle)
{
  const char *name = NULL;
  CONSOLE_SCREEN_BUFFER_INFO buf;
  struct sockaddr sa;
  int sal = sizeof (sa);
  DCB dcb;
  unsigned bin = O_BINARY;
  device dev;
  tmp_pathbuf tp;

  dev.devn = 0;		/* FIXME: device */
  first_fd_for_open = 0;

  if (!not_open (fd))
    return;

  SetLastError (0);
  DWORD ft = GetFileType (handle);
  if (ft != FILE_TYPE_UNKNOWN || GetLastError () != ERROR_INVALID_HANDLE)
    {
      /* See if we can consoleify it */
      if (GetConsoleScreenBufferInfo (handle, &buf))
	{
	  if (ISSTATE (myself, PID_USETTY))
	    dev.parse (FH_TTY);
	  else
	    dev = *console_dev;
	}
      else if (GetNumberOfConsoleInputEvents (handle, (DWORD *) &buf))
	{
	  if (ISSTATE (myself, PID_USETTY))
	    dev.parse (FH_TTY);
	  else
	    dev = *console_dev;
	}
      else if (wsock_started && getpeername ((SOCKET) handle, &sa, &sal) == 0)
	dev = *tcp_dev;
      else if (GetCommState (handle, &dcb))
	dev.parse (DEV_TTYS_MAJOR, 0);
      else
	{
	  name = handle_to_fn (handle, tp.c_get ());
	  if (!strncasematch (name, POSIX_NAMED_PIPE, POSIX_NAMED_PIPE_LEN))
	    /* nothing */;
	  else if (fd == 0)
	    dev = *piper_dev;
	  else
	    dev = *pipew_dev;
	}
    }

  if (!name && !dev)
    fds[fd] = NULL;
  else
    {
      fhandler_base *fh;

      if (dev)
	fh = build_fh_dev (dev, name);
      else
	fh = build_fh_name (name);

      if (fh)
	cygheap->fdtab[fd] = fh;

      if (name)
	{
	  bin = fh->pc_binmode ();
	  if (!bin)
	    {
	      bin = fh->get_default_fmode (O_RDWR);
	      if (!bin && dev)
		bin = O_BINARY;
	    }
	}

      DWORD access;
      if (dev == FH_TTY || dev == FH_CONSOLE)
      	access = GENERIC_READ | GENERIC_WRITE;
      else if (fd == 0)
	access = GENERIC_READ;
      else
	access = GENERIC_WRITE;  /* Should be rdwr for stderr but not sure that's
				    possible for some versions of handles */
      fh->init (handle, access, bin);
      set_std_handle (fd);
      paranoid_printf ("fd %d, handle %p", fd, handle);
    }
}

#define cnew(name) new ((void *) ccalloc (HEAP_FHANDLER, 1, sizeof (name))) name
fhandler_base *
build_fh_name (const char *name, HANDLE h, unsigned opt, suffix_info *si)
{
  path_conv pc (name, opt | PC_NULLEMPTY | PC_POSIX, si);
  if (pc.error)
    {
      fhandler_base *fh = cnew (fhandler_nodevice) ();
      if (fh)
	fh->set_error (pc.error);
      set_errno (fh ? pc.error : EMFILE);
      return fh;
    }

  if (!pc.exists () && h)
    pc.fillin (h);

  return build_fh_pc (pc);
}

fhandler_base *
build_fh_dev (const device& dev, const char *unix_name)
{
  path_conv pc (dev);
  if (unix_name)
    pc.set_normalized_path (unix_name, false);
  else
    pc.set_normalized_path (dev.name, false);
  return build_fh_pc (pc);
}

#define fh_unset ((fhandler_base *) 1)
fhandler_base *
build_fh_pc (path_conv& pc)
{
  fhandler_base *fh = fh_unset;

  switch (pc.dev.major)
    {
    case DEV_TTYS_MAJOR:
      fh = cnew (fhandler_tty_slave) ();
      break;
    case DEV_TTYM_MAJOR:
      fh = cnew (fhandler_tty_master) ();
      break;
    case DEV_CYGDRIVE_MAJOR:
      fh = cnew (fhandler_cygdrive) ();
      break;
    case DEV_FLOPPY_MAJOR:
    case DEV_CDROM_MAJOR:
    case DEV_SD_MAJOR:
    case DEV_SD1_MAJOR:
    case DEV_SD2_MAJOR:
    case DEV_SD3_MAJOR:
    case DEV_SD4_MAJOR:
    case DEV_SD5_MAJOR:
    case DEV_SD6_MAJOR:
    case DEV_SD7_MAJOR:
      fh = cnew (fhandler_dev_floppy) ();
      break;
    case DEV_TAPE_MAJOR:
      fh = cnew (fhandler_dev_tape) ();
      break;
    case DEV_SERIAL_MAJOR:
      fh = cnew (fhandler_serial) ();
      break;
    default:
      switch (pc.dev)
	{
	case FH_CONSOLE:
	case FH_CONIN:
	case FH_CONOUT:
	  fh = cnew (fhandler_console) ();
	  break;
	case FH_PTYM:
	  fh = cnew (fhandler_pty_master) ();
	  break;
	case FH_WINDOWS:
	  fh = cnew (fhandler_windows) ();
	  break;
	case FH_FIFO:
	  fh = cnew (fhandler_fifo) ();
	  break;
	case FH_PIPE:
	case FH_PIPER:
	case FH_PIPEW:
	  fh = cnew (fhandler_pipe) ();
	  break;
	case FH_TCP:
	case FH_UDP:
	case FH_ICMP:
	case FH_UNIX:
	case FH_STREAM:
	case FH_DGRAM:
	  fh = cnew (fhandler_socket) ();
	  break;
	case FH_FS:
	  fh = cnew (fhandler_disk_file) ();
	  break;
	case FH_NULL:
	  fh = cnew (fhandler_dev_null) ();
	  break;
	case FH_ZERO:
	case FH_FULL:
	  fh = cnew (fhandler_dev_zero) ();
	  break;
	case FH_RANDOM:
	case FH_URANDOM:
	  fh = cnew (fhandler_dev_random) ();
	  break;
	case FH_MEM:
	case FH_PORT:
	  fh = cnew (fhandler_dev_mem) ();
	  break;
	case FH_CLIPBOARD:
	  fh = cnew (fhandler_dev_clipboard) ();
	  break;
	case FH_OSS_DSP:
	  fh = cnew (fhandler_dev_dsp) ();
	  break;
	case FH_PROC:
	  fh = cnew (fhandler_proc) ();
	  break;
	case FH_REGISTRY:
	  fh = cnew (fhandler_registry) ();
	  break;
	case FH_PROCESS:
	  fh = cnew (fhandler_process) ();
	  break;
	case FH_PROCNET:
	  fh = cnew (fhandler_procnet) ();
	  break;
	case FH_NETDRIVE:
	  fh = cnew (fhandler_netdrive) ();
	  break;
	case FH_TTY:
	  {
	    if (myself->ctty == TTY_CONSOLE)
	      fh = cnew (fhandler_console) ();
	    else if (myself->ctty >= 0)
	      fh = cnew (fhandler_tty_slave) ();
	    break;
	  }
	case FH_KMSG:
	  fh = cnew (fhandler_mailslot) ();
	  break;
      }
    }

  if (fh == fh_unset)
    fh = cnew (fhandler_nodevice) ();

  if (fh)
    fh->set_name (pc);
  else
    set_errno (EMFILE);

  debug_printf ("fh %p", fh);
  return fh;
}

fhandler_base *
dtable::dup_worker (fhandler_base *oldfh)
{
  fhandler_base *newfh = build_fh_pc (oldfh->pc);
  if (!newfh)
    debug_printf ("build_fh_pc failed");
  else
    {
      *newfh = *oldfh;
      newfh->set_io_handle (NULL);
      if (oldfh->dup (newfh))
	{
	  cfree (newfh);
	  debug_printf ("oldfh->dup failed");
	}
      else
	{
	  newfh->close_on_exec (false);
	  debug_printf ("duped '%s' old %p, new %p", oldfh->get_name (), oldfh->get_io_handle (), newfh->get_io_handle ());
	}
    }
  return newfh;
}

int
dtable::dup2 (int oldfd, int newfd)
{
  int res = -1;
  fhandler_base *newfh = NULL;	// = NULL to avoid an incorrect warning

  MALLOC_CHECK;
  debug_printf ("dup2 (%d, %d)", oldfd, newfd);
  lock ();

  if (not_open (oldfd))
    {
      syscall_printf ("fd %d not open", oldfd);
      set_errno (EBADF);
      goto done;
    }

  if (newfd < 0)
    {
      syscall_printf ("new fd out of bounds: %d", newfd);
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

  debug_printf ("newfh->io_handle %p, oldfh->io_handle %p",
		newfh->get_io_handle (), fds[oldfd]->get_io_handle ());

  if (!not_open (newfd))
    close (newfd);
  else if ((size_t) newfd < size)
    /* nothing to do */;
  else if (find_unused_handle (newfd) < 0)
    {
      newfh->close ();
      res = -1;
      goto done;
    }

  fds[newfd] = newfh;

  if ((res = newfd) <= 2)
    set_std_handle (res);

done:
  MALLOC_CHECK;
  unlock ();
  syscall_printf ("%d = dup2 (%d, %d)", res, oldfd, newfd);

  return res;
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
  s->thread_errno = 0;
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
  s->thread_errno = 0;
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
  s->thread_errno = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return s;
}

/* Function to walk the fd table after an exec and perform
   per-fhandler type fixups. */
void
dtable::fixup_before_fork (DWORD target_proc_id)
{
  lock ();
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	debug_printf ("fd %d (%s)", i, fh->get_name ());
	fh->fixup_before_fork_exec (target_proc_id);
      }
  unlock ();
}

void
dtable::move_fd (int from, int to)
{
  // close (to); /* It is assumed that this is close-on-exec */
  fds[to] = fds[from];
  fds[from] = NULL;
}

void
dtable::fixup_before_exec (DWORD target_proc_id)
{
  lock ();
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL && !fh->close_on_exec ())
      {
	debug_printf ("fd %d (%s)", i, fh->get_name ());
	fh->fixup_before_fork_exec (target_proc_id);
      }
  unlock ();
}

void
dtable::set_file_pointers_for_exec ()
{
/* This is not POSIX-compliant so the function is only called for
   non-Cygwin processes. */
  LONG off_high = 0;
  lock ();
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL && fh->get_flags () & O_APPEND)
      SetFilePointer (fh->get_handle (), 0, &off_high, FILE_END);
  unlock ();
}

void
dtable::fixup_after_exec ()
{
  first_fd_for_open = 0;
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	fh->clear_readahead ();
	fh->fixup_after_exec ();
	if (fh->close_on_exec ())
	  {
	    if (fh->archetype)
	      {
		debug_printf ("closing fd %d since it is an archetype", i);
		fh->close ();
	      }
	    release (i);
	  }
	else if (i == 0)
	  SetStdHandle (std_consts[i], fh->get_io_handle ());
	else if (i <= 2)
	  SetStdHandle (std_consts[i], fh->get_output_handle ());
      }
}

void
dtable::fixup_after_fork (HANDLE parent)
{
  fhandler_base *fh;
  for (size_t i = 0; i < size; i++)
    if ((fh = fds[i]) != NULL)
      {
	if (fh->close_on_exec () || fh->need_fork_fixup ())
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

#ifdef NEWVFORK
int
dtable::vfork_child_dup ()
{
  fhandler_base **newtable;
  lock ();
  newtable = (fhandler_base **) ccalloc (HEAP_ARGV, size, sizeof (fds[0]));
  int res = 1;

  /* Remove impersonation */
  cygheap->user.deimpersonate ();
  if (cygheap->ctty)
    {
      cygheap->ctty->usecount++;
      cygheap->console_count++;
      report_tty_counts (cygheap->ctty, "vfork dup", "incremented ", "");
    }

  for (size_t i = 0; i < size; i++)
    if (not_open (i))
      continue;
    else if ((newtable[i] = dup_worker (fds[i])) != NULL)
      newtable[i]->set_close_on_exec (fds[i]->close_on_exec ());
    else
      {
	res = 0;
	goto out;
      }

  fds_on_hold = fds;
  fds = newtable;

out:
  /* Restore impersonation */
  cygheap->user.reimpersonate ();

  unlock ();
  return 1;
}

void
dtable::vfork_parent_restore ()
{
  lock ();

  fhandler_tty_slave *ctty_on_hold = cygheap->ctty_on_hold;
  close_all_files ();
  fhandler_base **deleteme = fds;
  fds = fds_on_hold;
  fds_on_hold = NULL;
  cfree (deleteme);
  unlock ();

  if (cygheap->ctty != ctty_on_hold)
    {
      cygheap->ctty = ctty_on_hold;		// revert
      cygheap->ctty->close ();			// Undo previous bump of this archetype
    }
  cygheap->ctty_on_hold = NULL;
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
  for (int i = 0; i < (int) size; i++)
    if ((fh = fds[i]) != NULL)
      {
	fh->clear_readahead ();
	if (!fh->archetype && fh->close_on_exec ())
	  release (i);
	else
	  {
	    fh->close ();
	    release (i);
	  }
      }

  fds = saveme;
  cfree (fds_on_hold);
  fds_on_hold = NULL;

  if (cygheap->ctty_on_hold)
    {
      cygheap->ctty_on_hold->close ();
      cygheap->ctty_on_hold = NULL;
    }
}
#endif /*NEWVFORK*/

static const char *
handle_to_fn (HANDLE h, char *posix_fn)
{
  tmp_pathbuf tp;
  OBJECT_NAME_INFORMATION *ntfn;
  const size_t len = sizeof (OBJECT_NAME_INFORMATION)
		     + NT_MAX_PATH * sizeof (WCHAR);
  char *fnbuf = (char *) alloca (len);

  memset (fnbuf, 0, len);
  ntfn = (OBJECT_NAME_INFORMATION *) fnbuf;
  ntfn->Name.MaximumLength = (NT_MAX_PATH - 1) * sizeof (WCHAR);
  ntfn->Name.Buffer = (WCHAR *) (ntfn + 1);

  NTSTATUS res = NtQueryObject (h, ObjectNameInformation, ntfn, len, NULL);

  if (!NT_SUCCESS (res))
    {
      strcpy (posix_fn, unknown_file);
      debug_printf ("NtQueryObject failed");
      return unknown_file;
    }

  // NT seems to do this on an unopened file
  if (!ntfn->Name.Buffer)
    {
      debug_printf ("nt->Name.Buffer == NULL");
      return NULL;
    }

  ntfn->Name.Buffer[ntfn->Name.Length / sizeof (WCHAR)] = 0;

  char *win32_fn = tp.c_get ();
  sys_wcstombs (win32_fn, NT_MAX_PATH, ntfn->Name.Buffer,
  		ntfn->Name.Length / sizeof (WCHAR));
  debug_printf ("nt name '%s'", win32_fn);
  if (!strncasematch (win32_fn, DEVICE_PREFIX, DEVICE_PREFIX_LEN)
      || !QueryDosDevice (NULL, fnbuf, len))
    return strcpy (posix_fn, win32_fn);

  char *p = strechr (win32_fn + DEVICE_PREFIX_LEN, '\\');

  int n = p - win32_fn;
  int maxmatchlen = 0;
  char *maxmatchdos = NULL;
  char *device = tp.c_get ();
  for (char *s = fnbuf; *s; s = strchr (s, '\0') + 1)
    {
      device[NT_MAX_PATH - 1] = '\0';
      if (strchr (s, ':') == NULL)
	continue;
      if (!QueryDosDevice (s, device, NT_MAX_PATH - 1))
	continue;
      char *q = strrchr (device, ';');
      if (q)
	{
	  char *r = strchr (q, '\\');
	  if (r)
	    strcpy (q, r + 1);
	}
      int devlen = strlen (device);
      if (device[devlen - 1] == '\\')
	device[--devlen] = '\0';
      if (devlen < maxmatchlen)
	continue;
      if (!strncasematch (device, win32_fn, devlen) ||
	  (win32_fn[devlen] != '\0' && win32_fn[devlen] != '\\'))
	continue;
      maxmatchlen = devlen;
      maxmatchdos = s;
      debug_printf ("current match '%s'", device);
    }

  char *w32 = win32_fn;
  bool justslash = false;
  if (maxmatchlen)
    {
      n = strlen (maxmatchdos);
      if (maxmatchdos[n - 1] == '\\')
	n--;
      w32 += maxmatchlen - n;
      memcpy (w32, maxmatchdos, n);
      w32[n] = '\\';
    }
  else if (strncasematch (w32, NAMED_PIPE, NAMED_PIPE_LEN))
    {
      debug_printf ("pipe");
      justslash = true;
    }
  else if (strncasematch (w32, REMOTE, REMOTE_LEN))
    {
      w32 += REMOTE_LEN - 2;
      *w32 = '\\';
      debug_printf ("remote drive");
      justslash = true;
    }
  else if (strncasematch (w32, REMOTE1, REMOTE1_LEN))
    {
      w32 += REMOTE1_LEN - 2;
      *w32 = '\\';
      debug_printf ("remote drive");
      justslash = true;
    }

  if (!justslash)
    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, w32, posix_fn,
		      NT_MAX_PATH);
  else
    {
      char *s, *d;
      for (s = w32, d = posix_fn; *s; s++, d++)
	if (*s == '\\')
	  *d = '/';
	else
	  *d = *s;
      *d = 0;
    }

  debug_printf ("derived path '%s', posix '%s'", w32, posix_fn);
  return posix_fn;
}
