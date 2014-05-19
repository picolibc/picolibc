/* dtable.cc: file descriptor support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

#define USE_SYS_TYPES_FD_SET
#include <winsock.h>
#include "pinfo.h"
#include "cygerrno.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "select.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "ntdll.h"
#include "shared_info.h"

static const DWORD std_consts[] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE,
				   STD_ERROR_HANDLE};

static bool handle_to_fn (HANDLE, char *);

#define WCLEN(x) ((sizeof (x) / sizeof (WCHAR)) - 1)
static const char unknown_file[] = "some disk file";
static const WCHAR DEV_NULL[] = L"\\Device\\Null";
static const WCHAR DEV_SOCKET[] = L"\\Device\\Afd";

static const WCHAR DEVICE_PREFIX[] = L"\\device\\";
static const size_t DEVICE_PREFIX_LEN WCLEN (DEVICE_PREFIX);

static const WCHAR DEV_NAMED_PIPE[] = L"\\Device\\NamedPipe\\";
static const size_t DEV_NAMED_PIPE_LEN = WCLEN (DEV_NAMED_PIPE);

static const WCHAR DEV_REMOTE[] = L"\\Device\\LanmanRedirector\\";
static const size_t DEV_REMOTE_LEN = WCLEN (DEV_REMOTE);

static const WCHAR DEV_REMOTE1[] = L"\\Device\\WinDfs\\Root\\";
static const size_t DEV_REMOTE1_LEN = WCLEN (DEV_REMOTE1);

/* Set aside space for the table of fds */
void
dtable_init ()
{
  if (!cygheap->fdtab.size)
    cygheap->fdtab.extend (NOFILE_INCR, 0);
}

void __stdcall
set_std_handle (int fd)
{
  fhandler_base *fh = cygheap->fdtab[fd];
  if (fd == 0)
    SetStdHandle (std_consts[fd], fh ? fh->get_handle () : NULL);
  else if (fd <= 2)
    SetStdHandle (std_consts[fd], fh ? fh->get_output_handle () : NULL);
}

int
dtable::extend (size_t howmuch, size_t min)
{
  size_t new_size = size + howmuch;
  fhandler_base **newfds;

  if (new_size <= OPEN_MAX_MAX)
    /* ok */;
  else if (size < OPEN_MAX_MAX && min < OPEN_MAX_MAX)
    new_size = OPEN_MAX_MAX;
  else
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
  debug_printf ("size %ld, fds %p", size, fds);
  return 1;
}

void
dtable::get_debugger_info ()
{
  extern bool jit_debug;
  if (!jit_debug && being_debugged ())
    {
      char std[3][sizeof ("/dev/ptyNNNN")];
      std[0][0] = std[1][0] = std [2][0] = '\0';
      char buf[sizeof ("cYgstd %x") + 64];
      sprintf (buf, "cYgstd %p %zx %x", &std, sizeof (std[0]), 3);
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
	      {
		CloseHandle (h);
		/* Copy to Windows' idea of a standard handle, otherwise
		   we have invalid standard handles when calling Windows
		   functions (small_printf and strace might suffer, too). */
		SetStdHandle (std_consts[i], i ? fh->get_output_handle ()
					       : fh->get_handle ());
	      }
	  }
    }
}

/* Initialize the file descriptor/handle mapping table.
   This function should only be called when a cygwin function is invoked
   by a non-cygwin function, i.e., it should only happen very rarely. */

void
dtable::stdio_init ()
{
  if (myself->cygstarted || ISSTATE (myself, PID_CYGPARENT))
    {
      tty_min *t = cygwin_shared->tty.get_cttyp ();
      if (t && t->is_console)
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
  /* CV 2008-10-17: Under debugger control, std fd's have been potentially
     initialized in dtable::get_debugger_info ().  In this case
     init_std_file_from_handle is a no-op, so, even if out == err we don't
     want to duplicate the handle since it will be unused. */
  if (out == err && (!being_debugged () || not_open (2)))
    {
      /* Since this code is not invoked for forked tasks, we don't have
	 to worry about the close-on-exec flag here.  */
      if (!DuplicateHandle (GetCurrentProcess (), out,
			    GetCurrentProcess (), &err,
			    0, TRUE, DUPLICATE_SAME_ACCESS))
	{
	  /* If that fails, do this as a fall back.  */
	  err = out;
	  system_printf ("couldn't make stderr distinct from stdout, %E");
	}
    }

  init_std_file_from_handle (1, out);
  init_std_file_from_handle (2, err);
}

const int dtable::initial_archetype_size;

fhandler_base *
dtable::find_archetype (device& dev)
{
  for (unsigned i = 0; i < farchetype; i++)
    if (archetypes[i]->get_device () == (dev_t) dev)
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
	debug_printf ("deleting element %d for %s(%d/%d)", i, fh->get_name (),
		      fh->dev ().get_major (), fh->dev ().get_minor ());
	if (i < --farchetype)
	  archetypes[i] = archetypes[farchetype];
	break;
      }

  delete fh;
}

int
dtable::find_unused_handle (size_t start)
{
  /* When extending, try to allocate a NOFILE_INCR chunk
     following the empty fd.  */
  size_t extendby = NOFILE_INCR + ((start >= size) ? 1 + start - size : 0);

  /* This do loop should only ever execute twice. */
  int res = -1;
  do
    {
      for (size_t i = start; i < size; i++)
	if (fds[i] == NULL)
	  {
	    res = (int) i;
	    goto out;
	  }
    }
  while (extend (extendby, start));
out:
  return res;
}

void
dtable::release (int fd)
{
  if (fds[fd]->need_fixup_before ())
    dec_need_fixup_before ();
  fds[fd]->dec_refcnt ();
  fds[fd] = NULL;
  if (fd <= 2)
    set_std_handle (fd);
}

extern "C" int
cygwin_attach_handle_to_fd (char *name, int fd, HANDLE handle, mode_t bin,
			    DWORD myaccess)
{
  cygheap->fdtab.lock ();
  if (fd == -1)
    fd = cygheap->fdtab.find_unused_handle ();
  fhandler_base *fh = build_fh_name (name);
  if (!fh)
    fd = -1;
  else
    {
      cygheap->fdtab[fd] = fh;
      cygheap->fdtab[fd]->inc_refcnt ();
      fh->init (handle, myaccess, bin ?: fh->pc_binmode ());
    }
  cygheap->fdtab.unlock ();
  return fd;
}

void
dtable::init_std_file_from_handle (int fd, HANDLE handle)
{
  tmp_pathbuf tp;
  CONSOLE_SCREEN_BUFFER_INFO buf;
  DCB dcb;
  unsigned bin = O_BINARY;
  device dev = {};

  first_fd_for_open = 0;

  if (!not_open (fd))
    return;

  SetLastError (0);
  DWORD access = 0;
  DWORD ft = GetFileType (handle);
  char *name = tp.c_get ();
  name[0] = '\0';
  if (ft == FILE_TYPE_UNKNOWN && GetLastError () == ERROR_INVALID_HANDLE)
    /* can't figure out what this is */;
  else if (ft == FILE_TYPE_PIPE)
    {
      int rcv = 0, len = sizeof (int);

      if (handle_to_fn (handle, name))
	dev.parse (name);
      else if (strcmp (name, ":sock:") == 0
	       /* NtQueryObject returns an error when called on an LSP socket
		  handle.  While fdsock now tries to fetch the underlying
		  base socket, this only works on Vista and later. */
	       || (strcmp (name, unknown_file) == 0
		   && !::getsockopt ((SOCKET) handle, SOL_SOCKET, SO_RCVBUF,
				     (char *) &rcv, &len)))
	{
	  /* socket */
	  dev = *tcp_dev;
	  name[0] = '\0';
	}
      else if (fd == 0)
	dev = *piper_dev;
      else
	dev = *pipew_dev;
    }
  else if (GetConsoleScreenBufferInfo (handle, &buf)
	   || GetNumberOfConsoleInputEvents (handle, (DWORD *) &buf))
    {
      /* Console I/O */
      if (myself->ctty > 0)
	dev.parse (myself->ctty);
      else
	{
	  dev.parse (FH_CONSOLE);
	  CloseHandle (handle);
	  handle = INVALID_HANDLE_VALUE;
	}
    }
  else if (GetCommState (handle, &dcb))
    /* FIXME: Not right - assumes ttyS0 */
    dev.parse (DEV_SERIAL_MAJOR, 0);
  else
    /* Try to figure it out from context - probably a disk file */
    handle_to_fn (handle, name);

  if (!name[0] && !dev)
    fds[fd] = NULL;
  else
    {
      fhandler_base *fh;

      if (dev)
	fh = build_fh_dev (dev);
      else
	fh = build_fh_name (name);

      if (!fh)
	return;

      if (name[0])
	{
	  bin = fh->pc_binmode ();
	  if (!bin)
	    {
	      bin = fh->get_default_fmode (O_RDWR);
	      if (!bin && dev)
		bin = O_BINARY;
	    }
	}

      IO_STATUS_BLOCK io;
      FILE_ACCESS_INFORMATION fai;
      int openflags = O_BINARY;

      /* Console windows are not kernel objects, so the access mask returned
	 by NtQueryInformationFile is meaningless.  CMD always hands down
	 stdin handles as R/O handles, but our tty slave sides are R/W. */
      if (fh->is_tty ())
	{
	  openflags |= O_RDWR;
	  access |= GENERIC_READ | GENERIC_WRITE;
	}
      else if (!iscons_dev (dev)
	       && NT_SUCCESS (NtQueryInformationFile (handle, &io, &fai,
						      sizeof fai,
						      FileAccessInformation)))
	{
	  if (fai.AccessFlags & FILE_WRITE_DATA)
	    {
	      openflags |= O_WRONLY;
	      access |= GENERIC_WRITE;
	    }
	  if (fai.AccessFlags & FILE_READ_DATA)
	    {
	      openflags |= openflags & O_WRONLY ? O_RDWR : O_RDONLY;
	      access |= GENERIC_READ;
	    }
	}
      else if (fd == 0)
	{
	  openflags |= O_RDONLY;
	  access |= GENERIC_READ;
	}
      else
	{
	  openflags |= O_WRONLY;
	  access |= GENERIC_WRITE;  /* Should be rdwr for stderr but not sure that's
				       possible for some versions of handles */
	}
      if (!fh->init (handle, access, bin))
	api_fatal ("couldn't initialize fd %d for %s", fd, fh->get_name ());

      fh->open_setup (openflags);
      fh->usecount = 0;
      cygheap->fdtab[fd] = fh;
      cygheap->fdtab[fd]->inc_refcnt ();
      set_std_handle (fd);
      paranoid_printf ("fd %d, handle %p", fd, handle);
    }
}

#define cnew(name, ...) \
  ({ \
    void* ptr = (void*) ccalloc (HEAP_FHANDLER, 1, sizeof (name)); \
    ptr ? new (ptr) name (__VA_ARGS__) : NULL; \
  })

#define cnew_no_ctor(name, ...) \
  ({ \
    void* ptr = (void*) ccalloc (HEAP_FHANDLER, 1, sizeof (name)); \
    ptr ? new (ptr) name (ptr) : NULL; \
  })

fhandler_base *
build_fh_name (const char *name, unsigned opt, suffix_info *si)
{
  path_conv pc (name, opt | PC_NULLEMPTY | PC_POSIX, si);
  if (pc.error)
    {
      fhandler_base *fh = cnew (fhandler_nodevice);
      if (fh)
	fh->set_error (pc.error);
      set_errno (fh ? pc.error : EMFILE);
      return fh;
    }

  return build_fh_pc (pc);
}

fhandler_base *
build_fh_dev (const device& dev, const char *unix_name)
{
  path_conv pc (dev);
  if (unix_name)
    pc.set_normalized_path (unix_name);
  else
    pc.set_normalized_path (dev.name);
  return build_fh_pc (pc);
}

#define fh_unset ((fhandler_base *) 1)
static device last_tty_dev;
#define fh_last_tty_dev ((fhandler_termios *) cygheap->fdtab.find_archetype (last_tty_dev))

static fhandler_base *
fh_alloc (path_conv& pc)
{
  fhandler_base *fh = fh_unset;
  fhandler_base *fhraw = NULL;

  switch (pc.dev.get_major ())
    {
    case DEV_PTYS_MAJOR:
      fh = cnew (fhandler_pty_slave, pc.dev.get_minor ());
      break;
    case DEV_PTYM_MAJOR:
      fh = cnew (fhandler_pty_master, pc.dev.get_minor ());
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
      fh = cnew (fhandler_dev_floppy);
      break;
    case DEV_TAPE_MAJOR:
      fh = cnew (fhandler_dev_tape);
      break;
    case DEV_SERIAL_MAJOR:
      fh = cnew (fhandler_serial);
      break;
    case DEV_CONS_MAJOR:
      fh = cnew (fhandler_console, pc.dev);
      break;
    default:
      switch ((dev_t) pc.dev)
	{
	case FH_CONSOLE:
	case FH_CONIN:
	case FH_CONOUT:
	  fh = cnew (fhandler_console, pc.dev);
	  break;
	case FH_PTMX:
	  if (pc.isopen ())
	    fh = cnew (fhandler_pty_master, -1);
	  else
	    fhraw = cnew_no_ctor (fhandler_pty_master, -1);
	  break;
	case FH_WINDOWS:
	  fh = cnew (fhandler_windows);
	  break;
	case FH_FIFO:
	  fh = cnew (fhandler_fifo);
	  break;
	case FH_PIPE:
	case FH_PIPER:
	case FH_PIPEW:
	  fh = cnew (fhandler_pipe);
	  break;
	case FH_TCP:
	case FH_UDP:
	case FH_ICMP:
	case FH_UNIX:
	case FH_STREAM:
	case FH_DGRAM:
	  fh = cnew (fhandler_socket);
	  break;
	case FH_FS:
	  fh = cnew (fhandler_disk_file);
	  break;
	case FH_NULL:
	  fh = cnew (fhandler_dev_null);
	  break;
	case FH_ZERO:
	case FH_FULL:
	  fh = cnew (fhandler_dev_zero);
	  break;
	case FH_RANDOM:
	case FH_URANDOM:
	  fh = cnew (fhandler_dev_random);
	  break;
	case FH_CLIPBOARD:
	  fh = cnew (fhandler_dev_clipboard);
	  break;
	case FH_OSS_DSP:
	  fh = cnew (fhandler_dev_dsp);
	  break;
	case FH_PROC:
	  fh = cnew (fhandler_proc);
	  break;
	case FH_REGISTRY:
	  fh = cnew (fhandler_registry);
	  break;
	case FH_PROCESS:
	case FH_PROCESSFD:
	  fh = cnew (fhandler_process);
	  break;
	case FH_PROCNET:
	  fh = cnew (fhandler_procnet);
	  break;
	case FH_PROCSYS:
	  fh = cnew (fhandler_procsys);
	  break;
	case FH_PROCSYSVIPC:
	  fh = cnew (fhandler_procsysvipc);
	  break;
	case FH_NETDRIVE:
	  fh = cnew (fhandler_netdrive);
	  break;
	case FH_DEV:
	  fh = cnew (fhandler_dev);
	  break;
	case FH_CYGDRIVE:
	  fh = cnew (fhandler_cygdrive);
	  break;
	case FH_TTY:
	  if (!pc.isopen ())
	    {
	      fhraw = cnew_no_ctor (fhandler_console, -1);
	      debug_printf ("not called from open for /dev/tty");
	    }
	  else if (myself->ctty <= 0 && last_tty_dev
		   && !myself->set_ctty (fh_last_tty_dev, 0))
	    debug_printf ("no /dev/tty assigned");
	  else if (myself->ctty > 0)
	    {
	      debug_printf ("determining /dev/tty assignment for ctty %p", myself->ctty);
	      if (iscons_dev (myself->ctty))
		fh = cnew (fhandler_console, pc.dev);
	      else
		fh = cnew (fhandler_pty_slave, myself->ctty);
	      if (fh->dev () != FH_NADA)
		fh->set_name ("/dev/tty");
	    }
	  break;
	case FH_KMSG:
	  fh = cnew (fhandler_mailslot);
	  break;
      }
    }

  /* If `fhraw' is set that means that this fhandler is just a dummy
     set up for stat().  Mock it up for use by stat without actually
     trying to do any real initialization.  */
  if (fhraw)
    {
      fh = fhraw;
      fh->set_name (pc);
      if (fh->use_archetype ())
	fh->archetype = fh;
    }
  if (fh == fh_unset)
    fh = cnew (fhandler_nodevice);
  else if (fh->dev () == FH_ERROR)
    {
      if (!pc.isopen () && pc.dev.isfs ())
	fh->dev () = pc.dev;	/* Special case: This file actually exists on
				   disk and we're not trying to open it so just
				   return the info from pc.  */
      else
	{
	  delete fh;
	  fh = NULL;
	}
    }
  return fh;
}

fhandler_base *
build_fh_pc (path_conv& pc)
{
  fhandler_base *fh = fh_alloc (pc);

  if (!fh)
    {
      set_errno (ENXIO);
      goto out;
    }

  if (!fh->use_archetype ())
    fh->set_name (pc);
  else if ((fh->archetype = cygheap->fdtab.find_archetype (fh->dev ())))
    {
      debug_printf ("found an archetype for %s(%d/%d) io_handle %p", fh->get_name (), fh->dev ().get_major (), fh->dev ().get_minor (),
		    fh->archetype->get_io_handle ());
      if (!fh->get_name ())
	fh->set_name (fh->archetype->dev ().name);
    }
  else if (cygwin_finished_initializing && !pc.isopen ())
    fh->set_name (pc);
  else
    {
      if (!fh->get_name ())
	fh->set_name (fh->dev ().native);
      fh->archetype = fh->clone ();
      debug_printf ("created an archetype (%p) for %s(%d/%d)", fh->archetype, fh->get_name (), fh->dev ().get_major (), fh->dev ().get_minor ());
      fh->archetype->archetype = NULL;
      *cygheap->fdtab.add_archetype () = fh->archetype;
    }


  /* Keep track of the last tty-like thing opened.  We could potentially want
     to open it if /dev/tty is referenced. */
  if (myself->ctty > 0 || !fh->is_tty () || !pc.isctty_capable ())
    last_tty_dev = FH_NADA;
  else
    last_tty_dev = fh->dev ();

out:
  debug_printf ("fh %p, dev %08x", fh, fh ? (dev_t) fh->dev () : 0);
  return fh;
}

fhandler_base *
dtable::dup_worker (fhandler_base *oldfh, int flags)
{
  /* Don't call set_name in build_fh_pc.  It will be called in
     fhandler_base::operator= below.  Calling it twice will result
     in double allocation. */
  fhandler_base *newfh = oldfh->clone ();
  if (!newfh)
    debug_printf ("build_fh_pc failed");
  else
    {
      if (!oldfh->archetype)
	newfh->set_io_handle (NULL);

      newfh->pc.reset_conv_handle ();
      if (oldfh->dup (newfh, flags))
	{
	  delete newfh;
	  newfh = NULL;
	  debug_printf ("oldfh->dup failed");
	}
      else
	{
	  /* Don't increment refcnt here since we don't know if this is a
	     allocated fd.  So we leave this chore to the caller. */

	  newfh->usecount = 0;
	  newfh->archetype_usecount (1);

	  /* The O_CLOEXEC flag enforces close-on-exec behaviour. */
	  newfh->set_close_on_exec (!!(flags & O_CLOEXEC));
	  debug_printf ("duped '%s' old %p, new %p", oldfh->get_name (),
			oldfh->get_io_handle (), newfh->get_io_handle ());
	}
    }
  return newfh;
}

int
dtable::dup3 (int oldfd, int newfd, int flags)
{
  int res = -1;
  fhandler_base *newfh = NULL;	// = NULL to avoid an incorrect warning

  MALLOC_CHECK;
  debug_printf ("dup3 (%d, %d, %y)", oldfd, newfd, flags);
  lock ();
  bool do_unlock = true;
  bool unlock_on_return;
  if (!(flags & O_EXCL))
    unlock_on_return = true;	/* Relinquish lock on return */
  else
    {
      flags &= ~O_EXCL;
      unlock_on_return = false;	/* Return with lock set on success */
    }

  if (not_open (oldfd))
    {
      syscall_printf ("fd %d not open", oldfd);
      set_errno (EBADF);
      goto done;
    }
  if (newfd >= OPEN_MAX_MAX || newfd < 0)
    {
      syscall_printf ("new fd out of bounds: %d", newfd);
      set_errno (EBADF);
      goto done;
    }
  if ((flags & ~O_CLOEXEC) != 0)
    {
      syscall_printf ("invalid flags value %y", flags);
      set_errno (EINVAL);
      return -1;
    }

  /* This is a temporary kludge until all utilities can catch up with
     a change in behavior that implements linux functionality:  opening
     a tty should not automatically cause it to become the controlling
     tty for the process.  */
  if (newfd > 2)
    flags |= O_NOCTTY;

  if ((newfh = dup_worker (fds[oldfd], flags)) == NULL)
    {
      res = -1;
      goto done;
    }

  debug_printf ("newfh->io_handle %p, oldfh->io_handle %p, new win32_name %p, old win32_name %p",
		newfh->get_io_handle (), fds[oldfd]->get_io_handle (), newfh->get_win32_name (), fds[oldfd]->get_win32_name ());

  if (!not_open (newfd))
    close (newfd);
  else if ((size_t) newfd >= size
	   && find_unused_handle (newfd) < 0)
    /* couldn't extend fdtab */
    {
      newfh->close ();
      res = -1;
      goto done;
    }

  fds[newfd] = newfh;

  if ((res = newfd) <= 2)
    set_std_handle (res);
  do_unlock = unlock_on_return;

done:
  MALLOC_CHECK;
  if (do_unlock)
    unlock ();
  syscall_printf ("%R = dup3(%d, %d, %y)", res, oldfd, newfd, flags);

  return res;
}

bool
dtable::select_read (int fd, select_stuff *ss)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return false;
    }
  fhandler_base *fh = fds[fd];
  select_record *s = fh->select_read (ss);
  s->fd = fd;
  if (!s->fh)
    s->fh = fh;
  s->thread_errno = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return true;
}

bool
dtable::select_write (int fd, select_stuff *ss)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fds[fd];
  select_record *s = fh->select_write (ss);
  s->fd = fd;
  s->fh = fh;
  s->thread_errno = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return true;
}

bool
dtable::select_except (int fd, select_stuff *ss)
{
  if (not_open (fd))
    {
      set_errno (EBADF);
      return NULL;
    }
  fhandler_base *fh = fds[fd];
  select_record *s = fh->select_except (ss);
  s->fd = fd;
  s->fh = fh;
  s->thread_errno = 0;
  debug_printf ("%s fd %d", fh->get_name (), fd);
  return true;
}

void
dtable::move_fd (int from, int to)
{
  // close (to); /* It is assumed that this is close-on-exec */
  fds[to] = fds[from];
  fds[from] = NULL;
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
dtable::fixup_close (size_t i, fhandler_base *fh)
{
  if (fh->archetype)
    {
      debug_printf ("closing fd %d since it is an archetype", i);
      fh->close_with_arch ();
    }
  release (i);
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
	/* Close the handle if it's close-on-exec or if an error was detected
	   (typically with opening a console in a gui app) by fixup_after_exec.
	 */
	if (fh->close_on_exec () || (!fh->nohandle () && !fh->get_io_handle ()))
	  fixup_close (i, fh);
	else if (fh->get_popen_pid ())
	  close (i);
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
	    if (!fh->nohandle () && !fh->get_io_handle ())
	      {
		/* This should actually never happen but it's here to make sure
		   we don't crash due to access of an unopened file handle.  */
		fixup_close (i, fh);
		continue;
	      }
	  }
	if (i == 0)
	  SetStdHandle (std_consts[i], fh->get_io_handle ());
	else if (i <= 2)
	  SetStdHandle (std_consts[i], fh->get_output_handle ());
      }
}

static void
decode_tty (char *buf, WCHAR *w32)
{
  int ttyn = wcstol (w32, NULL, 10);
  __ptsname (buf, ttyn);
}

/* Try to derive posix filename from given handle.  Return true if
   the handle is associated with a cygwin tty. */
static bool
handle_to_fn (HANDLE h, char *posix_fn)
{
  tmp_pathbuf tp;
  ULONG len = 0;
  WCHAR *maxmatchdos = NULL;
  PWCHAR device = tp.w_get ();
  int maxmatchlen = 0;
  OBJECT_NAME_INFORMATION *ntfn = (OBJECT_NAME_INFORMATION *) tp.w_get ();

  NTSTATUS status = NtQueryObject (h, ObjectNameInformation, ntfn, 65536, &len);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryObject failed, %y", status);
  // NT seems to do this on an unopened file
  else if (!ntfn->Name.Buffer)
    debug_printf ("nt->Name.Buffer == NULL");
  else
    {
      WCHAR *w32 = ntfn->Name.Buffer;
      size_t w32len = ntfn->Name.Length / sizeof (WCHAR);
      w32[w32len] = L'\0';

      if (wcscasecmp (w32, DEV_NULL) == 0)
	{
	  strcpy (posix_fn, "/dev/null");
	  return false;
	}

      if (wcscasecmp (w32, DEV_SOCKET) == 0)
	{
	  strcpy (posix_fn, ":sock:");
	  return false;
	}

      if (wcsncasecmp (w32, DEV_NAMED_PIPE, DEV_NAMED_PIPE_LEN) == 0)
	{
	  w32 += DEV_NAMED_PIPE_LEN;
	  if (wcsncmp (w32, L"cygwin-", WCLEN (L"cygwin-")) != 0)
	    return false;
	  w32 += WCLEN (L"cygwin-");
	  /* Check for installation key and trailing dash. */
	  w32len = cygheap->installation_key.Length / sizeof (WCHAR);
	  if (w32len
	      && wcsncmp (w32, cygheap->installation_key.Buffer, w32len) != 0)
	    return false;
	  w32 += w32len;
	  if (*w32 != L'-')
	    return false;
	  ++w32;
	  bool istty = wcsncmp (w32, L"pty", WCLEN (L"pty")) == 0;
	  if (istty)
	    decode_tty (posix_fn, w32 + WCLEN (L"pty"));
	  else if (wcsncmp (w32, L"pipe", WCLEN (L"pipe")) == 0)
	    strcpy (posix_fn, "/dev/pipe");
	  return istty;
	}

      WCHAR fnbuf[64 * 1024];
      if (wcsncasecmp (w32, DEVICE_PREFIX, DEVICE_PREFIX_LEN) != 0
	  || !QueryDosDeviceW (NULL, fnbuf, sizeof (fnbuf) / sizeof (WCHAR)))
	{
	  sys_wcstombs (posix_fn, NT_MAX_PATH, w32, w32len);
	  return false;
	}

      for (WCHAR *s = fnbuf; *s; s = wcschr (s, '\0') + 1)
	{
	  if (!QueryDosDeviceW (s, device, NT_MAX_PATH))
	    continue;
	  if (wcschr (s, ':') == NULL)
	    continue;
	  WCHAR *q = wcsrchr (device, ';');
	  if (q)
	    {
	      WCHAR *r = wcschr (q, '\\');
	      if (r)
		wcscpy (q, r + 1);
	    }
	  int devlen = wcslen (device);
	  if (device[devlen - 1] == L'\\')
	    device[--devlen] = L'\0';
	  if (devlen < maxmatchlen)
	    continue;
	  if (wcsncmp (device, w32, devlen) != 0||
	      (w32[devlen] != L'\0' && w32[devlen] != L'\\'))
	    continue;
	  maxmatchlen = devlen;
	  maxmatchdos = s;
	  debug_printf ("current match '%W' = '%W'\n", s, device);
	}

      if (maxmatchlen)
	{
	  WCHAR *p = wcschr (w32 + DEVICE_PREFIX_LEN, L'\\');
	  size_t n = wcslen (maxmatchdos);
	  WCHAR ch;
	  if (!p)
	    ch = L'\0';
	  else
	    {
	      if (maxmatchdos[n - 1] == L'\\')
		n--;
	      w32 += maxmatchlen - n;
	      ch = L'\\';
	    }
	  memcpy (w32, maxmatchdos, n * sizeof (WCHAR));
	  w32[n] = ch;
	}
      else if (wcsncmp (w32, DEV_REMOTE, DEV_REMOTE_LEN) == 0)
	{
	  w32 += DEV_REMOTE_LEN - 2;
	  *w32 = L'\\';
	  debug_printf ("remote drive");
	}
      else if (wcsncmp (w32, DEV_REMOTE1, DEV_REMOTE1_LEN) == 0)
	{
	  w32 += DEV_REMOTE1_LEN - 2;
	  *w32 = L'\\';
	  debug_printf ("remote drive");
	}

      cygwin_conv_path (CCP_WIN_W_TO_POSIX | CCP_ABSOLUTE, w32, posix_fn,
			NT_MAX_PATH);

      debug_printf ("derived path '%W', posix '%s'", w32, posix_fn);
      return false;
    }

  strcpy (posix_fn,  unknown_file);
  return false;
}

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
