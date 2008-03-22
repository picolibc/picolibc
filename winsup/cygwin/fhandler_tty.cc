/* fhandler_tty.cc

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wingdi.h>
#include <winuser.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <cygwin/kd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygheap.h"
#include "shared_info.h"
#include "cygserver.h"
#include "cygthread.h"
#include "child_info.h"

/* tty master stuff */

fhandler_tty_master NO_COPY *tty_master;

static DWORD WINAPI process_input (void *);		// Input queue thread
static DWORD WINAPI process_output (void *);		// Output queue thread
static DWORD WINAPI process_ioctl (void *);		// Ioctl requests thread

fhandler_tty_master::fhandler_tty_master ()
  : fhandler_pty_master (), console (NULL)
{
}

int
fhandler_tty_slave::get_unit ()
{
  return dev () == FH_TTY ? myself->ctty : dev ().minor;
}

void
fhandler_tty_master::set_winsize (bool sendSIGWINCH)
{
  winsize w;
  console->ioctl (TIOCGWINSZ, &w);
  get_ttyp ()->winsize = w;
  if (sendSIGWINCH)
    tc->kill_pgrp (SIGWINCH);
}

int
fhandler_tty_master::init ()
{
  termios_printf ("Creating master for tty%d", get_unit ());

  if (init_console ())
    {
      termios_printf ("can't create fhandler");
      return -1;
    }

  termios ti;
  memset (&ti, 0, sizeof (ti));
  console->tcsetattr (0, &ti);

  if (!setup (false))
    return 1;

  set_winsize (false);

  set_close_on_exec (true);

  cygthread *h;
  h = new cygthread (process_input, 0, cygself, "ttyin");
  h->SetThreadPriority (THREAD_PRIORITY_HIGHEST);
  h->zap_h ();

  h = new cygthread (process_ioctl, 0, cygself, "ttyioctl");
  h->SetThreadPriority (THREAD_PRIORITY_HIGHEST);
  h->zap_h ();

  h = new cygthread (process_output, 0, cygself, "ttyout");
  h->SetThreadPriority (THREAD_PRIORITY_HIGHEST);
  h->zap_h ();

  return 0;
}

#ifdef DEBUGGING
static class mutex_stack
{
public:
  const char *fn;
  int ln;
  const char *tname;
} ostack[100];

static int osi;
#endif /*DEBUGGING*/

DWORD
fhandler_tty_common::__acquire_output_mutex (const char *fn, int ln,
					   DWORD ms)
{
  if (strace.active ())
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): tty output_mutex: waiting %d ms", ln, ms);
  DWORD res = WaitForSingleObject (output_mutex, ms);
  if (res == WAIT_OBJECT_0)
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): tty output_mutex: acquired", ln, res);
#else
      ostack[osi].fn = fn;
      ostack[osi].ln = ln;
      ostack[osi].tname = cygthread::name ();
      termios_printf ("acquired for %s:%d, osi %d", fn, ln, osi);
      osi++;
#endif
    }
  return res;
}

void
fhandler_tty_common::__release_output_mutex (const char *fn, int ln)
{
  if (ReleaseMutex (output_mutex))
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): tty output_mutex released", ln);
#else
      if (osi > 0)
	osi--;
      termios_printf ("released at %s:%d, osi %d", fn, ln, osi);
      termios_printf ("  for %s:%d (%s)", ostack[osi].fn, ostack[osi].ln, ostack[osi].tname);
      ostack[osi].ln = -ln;
#endif
    }
#ifdef DEBUGGING
  else if (osi > 0)
    {
      system_printf ("couldn't release output mutex but we seem to own it, %E");
      try_to_debug ();
    }
#endif
}

/* Process tty input. */

void
fhandler_pty_master::doecho (const void *str, DWORD len)
{
  acquire_output_mutex (INFINITE);
  if (!WriteFile (to_master, str, len, &len, NULL))
    termios_printf ("Write to %p failed, %E", to_master);
//  WaitForSingleObject (output_done_event, INFINITE);
  release_output_mutex ();
}

int
fhandler_pty_master::accept_input ()
{
  DWORD bytes_left;
  int ret = 1;

  WaitForSingleObject (input_mutex, INFINITE);

  bytes_left = eat_readahead (-1);

  if (!bytes_left)
    {
      termios_printf ("sending EOF to slave");
      get_ttyp ()->read_retval = 0;
    }
  else
    {
      char *p = rabuf;
      DWORD rc;
      DWORD written = 0;

      termios_printf ("about to write %d chars to slave", bytes_left);
      rc = WriteFile (get_output_handle (), p, bytes_left, &written, NULL);
      if (!rc)
	{
	  debug_printf ("error writing to pipe %E");
	  get_ttyp ()->read_retval = -1;
	  ret = -1;
	}
      else
	{
	  get_ttyp ()->read_retval = 1;
	  p += written;
	  bytes_left -= written;
	  if (bytes_left > 0)
	    {
	      debug_printf ("to_slave pipe is full");
	      puts_readahead (p, bytes_left);
	      ret = 0;
	    }
	}
    }

  SetEvent (input_available_event);
  ReleaseMutex (input_mutex);
  return ret;
}

static DWORD WINAPI
process_input (void *)
{
  char rawbuf[INP_BUFFER_SIZE];

  while (1)
    {
      size_t nraw = INP_BUFFER_SIZE;
      tty_master->console->read ((void *) rawbuf, nraw);
      if (tty_master->line_edit (rawbuf, nraw, tty_master->get_ttyp ()->ti)
	  == line_edit_signalled)
	tty_master->console->eat_readahead (-1);
    }
}

bool
fhandler_pty_master::hit_eof ()
{
  if (get_ttyp ()->was_opened && !get_ttyp ()->slave_alive ())
    {
      /* We have the only remaining open handle to this pty, and
	 the slave pty has been opened at least once.  We treat
	 this as EOF.  */
      termios_printf ("all other handles closed");
      return 1;
    }
  return 0;
}

/* Process tty output requests */

int
fhandler_pty_master::process_slave_output (char *buf, size_t len, int pktmode_on)
{
  size_t rlen;
  char outbuf[OUT_BUFFER_SIZE + 1];
  DWORD n;
  int column = 0;
  int rc = 0;

  if (len == 0)
    goto out;

  if (need_nl)
    {
      /* We need to return a left over \n character, resulting from
	 \r\n conversion.  Note that we already checked for FLUSHO and
	 output_stopped at the time that we read the character, so we
	 don't check again here.  */
      if (buf)
	buf[0] = '\n';
      need_nl = 0;
      rc = 1;
      goto out;
    }


  for (;;)
    {
      /* Set RLEN to the number of bytes to read from the pipe.  */
      rlen = len;
      if (get_ttyp ()->ti.c_oflag & OPOST && get_ttyp ()->ti.c_oflag & ONLCR)
	{
	  /* We are going to expand \n to \r\n, so don't read more than
	     half of the number of bytes requested.  */
	  rlen /= 2;
	  if (rlen == 0)
	    rlen = 1;
	}
      if (rlen > sizeof outbuf)
	rlen = sizeof outbuf;

      HANDLE handle = get_io_handle ();

      n = 0; // get_readahead_into_buffer (outbuf, len);
      if (!n)
	{
	  /* Doing a busy wait like this is quite inefficient, but nothing
	     else seems to work completely.  Windows should provide some sort
	     of overlapped I/O for pipes, or something, but it doesn't.  */
	  while (1)
	    {
	      if (!PeekNamedPipe (handle, NULL, 0, NULL, &n, NULL))
		goto err;
	      if (n > 0)
		break;
	      if (hit_eof ())
		goto out;
	      /* DISCARD (FLUSHO) and tcflush can finish here. */
	      if (n == 0 && (get_ttyp ()->ti.c_lflag & FLUSHO || !buf))
		goto out;
	      if (n == 0 && is_nonblocking ())
		{
		  set_errno (EAGAIN);
		  rc = -1;
		  break;
		}

	      Sleep (10);
	    }

	  if (ReadFile (handle, outbuf, rlen, &n, NULL) == FALSE)
	    goto err;
	}

      termios_printf ("bytes read %u", n);
      get_ttyp ()->write_error = 0;
      if (output_done_event != NULL)
	SetEvent (output_done_event);

      if (get_ttyp ()->ti.c_lflag & FLUSHO || !buf)
	continue;

      char *optr;
      optr = buf;
      if (pktmode_on)
	*optr++ = TIOCPKT_DATA;

      if (!(get_ttyp ()->ti.c_oflag & OPOST))	// post-process output
	{
	  memcpy (optr, outbuf, n);
	  optr += n;
	}
      else					// raw output mode
	{
	  char *iptr = outbuf;

	  while (n--)
	    {
	      switch (*iptr)
		{
		case '\r':
		  if ((get_ttyp ()->ti.c_oflag & ONOCR) && column == 0)
		    {
		      iptr++;
		      continue;
		    }
		  if (get_ttyp ()->ti.c_oflag & OCRNL)
		    *iptr = '\n';
		  else
		    column = 0;
		  break;
		case '\n':
		  if (get_ttyp ()->ti.c_oflag & ONLCR)
		    {
		      *optr++ = '\r';
		      column = 0;
		    }
		  if (get_ttyp ()->ti.c_oflag & ONLRET)
		    column = 0;
		  break;
		default:
		  column++;
		  break;
		}

	      /* Don't store data past the end of the user's buffer.  This
		 can happen if the user requests a read of 1 byte when
		 doing \r\n expansion.  */
	      if (optr - buf >= (int) len)
		{
		  if (*iptr != '\n' || n != 0)
		    system_printf ("internal error: %d unexpected characters", n);
		  need_nl = 1;
		  break;
		}

	      *optr++ = *iptr++;
	    }
	}
      rc = optr - buf;
      break;

    err:
      if (GetLastError () == ERROR_BROKEN_PIPE)
	rc = 0;
      else
	{
	  __seterrno ();
	  rc = -1;
	}
      break;
    }

out:
  termios_printf ("returning %d", rc);
  return rc;
}

static DWORD WINAPI
process_output (void *)
{
  char buf[OUT_BUFFER_SIZE * 2];

  for (;;)
    {
      int n = tty_master->process_slave_output (buf, OUT_BUFFER_SIZE, 0);
      if (n <= 0)
	{
	  if (n < 0)
	    termios_printf ("ReadFile %E");
	  ExitThread (0);
	}
      n = tty_master->console->write ((void *) buf, (size_t) n);
      tty_master->get_ttyp ()->write_error = n == -1 ? get_errno () : 0;
    }
}


/* Process tty ioctl requests */

static DWORD WINAPI
process_ioctl (void *)
{
  while (1)
    {
      WaitForSingleObject (tty_master->ioctl_request_event, INFINITE);
      termios_printf ("ioctl() request");
      tty *ttyp = tty_master->get_ttyp ();
      ttyp->ioctl_retval =
      tty_master->console->ioctl (ttyp->cmd,
				  (ttyp->cmd == KDSKBMETA)
				  ? (void *) ttyp->arg.value
				  : (void *) &ttyp->arg);
      SetEvent (tty_master->ioctl_done_event);
    }
}

/**********************************************************************/
/* Tty slave stuff */

fhandler_tty_slave::fhandler_tty_slave ()
  : fhandler_tty_common (), inuse (NULL)
{
  uninterruptible_io (true);
}

/* FIXME: This function needs to close handles when it has
   a failing condition. */
int
fhandler_tty_slave::open (int flags, mode_t)
{
  if (get_device () == FH_TTY)
    dev().tty_to_real_device ();
  fhandler_tty_slave *arch = (fhandler_tty_slave *) cygheap->fdtab.find_archetype (pc.dev);
  if (arch)
    {
      *this = *(fhandler_tty_slave *) arch;
      termios_printf ("copied fhandler_tty_slave archetype");
      set_flags ((flags & ~O_TEXT) | O_BINARY);
      cygheap->manage_console_count ("fhandler_tty_slave::open<arch>", 1);
      goto out;
    }

  tcinit (cygwin_shared->tty[get_unit ()]);

  cygwin_shared->tty.attach (get_unit ());

  set_flags ((flags & ~O_TEXT) | O_BINARY);
  /* Create synchronisation events */
  char buf[MAX_PATH];

  /* output_done_event may or may not exist.  It will exist if the tty
     was opened by fhandler_tty_master::init, normally called at
     startup if use_tty is non-zero.  It will not exist if this is a
     pty opened by fhandler_pty_master::open.  In the former case, tty
     output is handled by a separate thread which controls output.  */
  shared_name (buf, OUTPUT_DONE_EVENT, get_unit ());
  output_done_event = OpenEvent (EVENT_ALL_ACCESS, TRUE, buf);

  if (!(output_mutex = get_ttyp ()->open_output_mutex ()))
    {
      termios_printf ("open output mutex failed, %E");
      __seterrno ();
      return 0;
    }
  if (!(input_mutex = get_ttyp ()->open_input_mutex ()))
    {
      termios_printf ("open input mutex failed, %E");
      __seterrno ();
      return 0;
    }
  shared_name (buf, INPUT_AVAILABLE_EVENT, get_unit ());
  if (!(input_available_event = OpenEvent (EVENT_ALL_ACCESS, TRUE, buf)))
    {
      termios_printf ("open input event failed, %E");
      __seterrno ();
      return 0;
    }

  /* The ioctl events may or may not exist.  See output_done_event,
     above.  */
  shared_name (buf, IOCTL_REQUEST_EVENT, get_unit ());
  ioctl_request_event = OpenEvent (EVENT_ALL_ACCESS, TRUE, buf);
  shared_name (buf, IOCTL_DONE_EVENT, get_unit ());
  ioctl_done_event = OpenEvent (EVENT_ALL_ACCESS, TRUE, buf);

  /* FIXME: Needs a method to eliminate tty races */
  {
    acquire_output_mutex (500);
    inuse = get_ttyp ()->create_inuse (TTY_SLAVE_ALIVE);
    get_ttyp ()->was_opened = true;
    release_output_mutex ();
  }

  if (!get_ttyp ()->from_master || !get_ttyp ()->to_master)
    {
      termios_printf ("tty handles have been closed");
      set_errno (EACCES);
      return 0;
    }

  HANDLE from_master_local;
  HANDLE to_master_local;
  from_master_local = to_master_local = NULL;

#ifdef USE_SERVER
  if (cygserver_running == CYGSERVER_UNAVAIL
      || !cygserver_attach_tty (&from_master_local, &to_master_local))
#endif
    {
      if (get_ttyp ()->master_pid < 0)
	{
	  set_errno (EAGAIN);
	  termios_printf ("*** master is closed");
	  return 0;
	}
      pinfo p (get_ttyp ()->master_pid);
      if (!p)
	{
	  set_errno (EAGAIN);
	  termios_printf ("*** couldn't find tty master");
	  return 0;
	}
#ifdef USE_SERVER
      termios_printf ("cannot dup handles via server. using old method.");
#endif
      HANDLE tty_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE,
				      p->dwProcessId);
      if (tty_owner == NULL)
	{
	  termios_printf ("can't open tty (%d) handle process %d",
			  get_unit (), get_ttyp ()->master_pid);
	  __seterrno ();
	  return 0;
	}

      if (!DuplicateHandle (tty_owner, get_ttyp ()->from_master,
			    hMainProc, &from_master_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input, %E");
	  __seterrno ();
	  return 0;
	}

      VerifyHandle (from_master_local);
      if (!DuplicateHandle (tty_owner, get_ttyp ()->to_master,
			  hMainProc, &to_master_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate output, %E");
	  __seterrno ();
	  return 0;
	}
      VerifyHandle (to_master_local);
      CloseHandle (tty_owner);
    }

  termios_printf ("duplicated from_master %p->%p from tty_owner",
		  get_ttyp ()->from_master, from_master_local);
  termios_printf ("duplicated to_master %p->%p from tty_owner",
		  get_ttyp ()->to_master, to_master_local);

  set_io_handle (from_master_local);
  set_output_handle (to_master_local);

  set_open_status ();
  if (cygheap->manage_console_count ("fhandler_tty_slave::open", 1) == 1
      && !output_done_event)
    fhandler_console::need_invisible ();

  // FIXME: Do this better someday
  arch = (fhandler_tty_slave *) cmalloc_abort (HEAP_ARCHETYPES, sizeof (*this));
  *((fhandler_tty_slave **) cygheap->fdtab.add_archetype ()) = arch;
  archetype = arch;
  *arch = *this;

out:
  usecount = 0;
  arch->usecount++;
  report_tty_counts (this, "opened", "");
  myself->set_ctty (get_ttyp (), flags, arch);

  return 1;
}

int
fhandler_tty_slave::close ()
{
  /* This used to always call fhandler_tty_common::close when hExeced but that
     caused multiple closes of the handles associated with this tty.  Since
     close_all_files is not called until after the cygwin process has synced
     or before a non-cygwin process has exited, it should be safe to just
     close this normally.  cgf 2006-05-20 */
  cygheap->manage_console_count ("fhandler_tty_slave::close", -1);

  archetype->usecount--;
  report_tty_counts (this, "closed", "");

  if (archetype->usecount)
    {
#ifdef DEBUGGING
      if (archetype->usecount < 0)
	system_printf ("error: usecount %d", archetype->usecount);
#endif
      termios_printf ("just returning because archetype usecount is != 0");
      return 0;
    }

  termios_printf ("closing last open %s handle", ttyname ());
  if (inuse && !CloseHandle (inuse))
    termios_printf ("CloseHandle (inuse), %E");
  return fhandler_tty_common::close ();
}

int
fhandler_tty_slave::cygserver_attach_tty (LPHANDLE from_master_ptr,
					  LPHANDLE to_master_ptr)
{
#ifndef USE_SERVER
  return 0;
#else
  if (!from_master_ptr || !to_master_ptr)
    return 0;

  client_request_attach_tty req ((DWORD) get_ttyp ()->master_pid,
				 (HANDLE) get_ttyp ()->from_master,
				 (HANDLE) get_ttyp ()->to_master);

  if (req.make_request () == -1 || req.error_code ())
    return 0;

  *from_master_ptr = req.from_master ();
  *to_master_ptr = req.to_master ();

  return 1;
#endif
}

void
fhandler_tty_slave::init (HANDLE, DWORD a, mode_t)
{
  int flags = 0;

  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;

  open (flags);
}

int
fhandler_tty_slave::write (const void *ptr, size_t len)
{
  DWORD n, towrite = len;

  termios_printf ("tty%d, write(%x, %d)", get_unit (), ptr, len);

  acquire_output_mutex (INFINITE);

  while (len)
    {
      n = min (OUT_BUFFER_SIZE, len);
      char *buf = (char *)ptr;
      ptr = (char *) ptr + n;
      len -= n;

      /* Previous write may have set write_error to != 0.  Check it here.
	 This is less than optimal, but the alternative slows down tty
	 writes enormously. */
      if (get_ttyp ()->write_error)
	{
	  set_errno (get_ttyp ()->write_error);
	  towrite = (DWORD) -1;
	  break;
	}

      if (WriteFile (get_output_handle (), buf, n, &n, NULL) == FALSE)
	{
	  DWORD err = GetLastError ();
	  termios_printf ("WriteFile failed, %E");
	  switch (err)
	    {
	    case ERROR_NO_DATA:
	      err = ERROR_IO_DEVICE;
	    default:
	      __seterrno_from_win_error (err);
	    }
	  raise (SIGHUP);		/* FIXME: Should this be SIGTTOU? */
	  towrite = (DWORD) -1;
	  break;
	}

      if (output_done_event != NULL)
	{
	  DWORD rc;
	  DWORD x = n * 1000;
	  rc = WaitForSingleObject (output_done_event, x);
	  termios_printf ("waited %d ms for output_done_event, WFSO %d", x, rc);
	}
    }
  release_output_mutex ();
  return towrite;
}

void __stdcall
fhandler_tty_slave::read (void *ptr, size_t& len)
{
  int totalread = 0;
  int vmin = 0;
  int vtime = 0;	/* Initialized to prevent -Wuninitialized warning */
  size_t readlen;
  DWORD bytes_in_pipe;
  char buf[INP_BUFFER_SIZE];
  char peek_buf[INP_BUFFER_SIZE];
  DWORD time_to_wait;
  DWORD rc;
  HANDLE w4[2];

  termios_printf ("read(%x, %d) handle %p", ptr, len, get_handle ());

  if (!ptr) /* Indicating tcflush(). */
    time_to_wait = 0;
  else if ((get_ttyp ()->ti.c_lflag & ICANON))
    time_to_wait = INFINITE;
  else
    {
      vmin = get_ttyp ()->ti.c_cc[VMIN];
      if (vmin > INP_BUFFER_SIZE)
	vmin = INP_BUFFER_SIZE;
      vtime = get_ttyp ()->ti.c_cc[VTIME];
      if (vmin < 0)
	vmin = 0;
      if (vtime < 0)
	vtime = 0;
      if (!vmin && !vtime)
	time_to_wait = 0;
      else
	time_to_wait = !vtime ? INFINITE : 100 * vtime;
    }

  w4[0] = signal_arrived;
  w4[1] = input_available_event;

  DWORD waiter = time_to_wait;
  while (len)
    {
      rc = WaitForMultipleObjects (2, w4, FALSE, waiter);

      if (rc == WAIT_TIMEOUT)
	{
	  termios_printf ("wait timed out, waiter %u", waiter);
	  break;
	}

      if (rc == WAIT_FAILED)
	{
	  termios_printf ("wait for input event failed, %E");
	  break;
	}

      if (rc == WAIT_OBJECT_0)
	{
	  /* if we've received signal after successfully reading some data,
	     just return all data successfully read */
	  if (totalread > 0)
	    break;
	  set_sig_errno (EINTR);
	  len = (size_t) -1;
	  return;
	}

      rc = WaitForSingleObject (input_mutex, 1000);
      if (rc == WAIT_FAILED)
	{
	  termios_printf ("wait for input mutex failed, %E");
	  break;
	}
      else if (rc == WAIT_TIMEOUT)
	{
	  termios_printf ("failed to acquire input mutex after input event arrived");
	  break;
	}
      if (!PeekNamedPipe (get_handle (), peek_buf, sizeof (peek_buf), &bytes_in_pipe, NULL, NULL))
	{
	  termios_printf ("PeekNamedPipe failed, %E");
	  raise (SIGHUP);
	  bytes_in_pipe = 0;
	}

      /* On first peek determine no. of bytes to flush. */
      if (!ptr && len == UINT_MAX)
	len = (size_t) bytes_in_pipe;

      if (ptr && !bytes_in_pipe && !vmin && !time_to_wait)
	{
	  ReleaseMutex (input_mutex);
	  len = (size_t) bytes_in_pipe;
	  return;
	}

      readlen = min (bytes_in_pipe, min (len, sizeof (buf)));

      if (ptr && vmin && readlen > (unsigned) vmin)
	readlen = vmin;

      DWORD n = 0;
      if (readlen)
	{
	  termios_printf ("reading %d bytes (vtime %d)", readlen, vtime);
	  if (ReadFile (get_handle (), buf, readlen, &n, NULL) == FALSE)
	    {
	      termios_printf ("read failed, %E");
	      raise (SIGHUP);
	    }
	  /* MSDN states that 5th prameter can be used to determine total
	     number of bytes in pipe, but for some reason this number doesn't
	     change after successful read. So we have to peek into the pipe
	     again to see if input is still available */
	  if (!PeekNamedPipe (get_handle (), peek_buf, 1, &bytes_in_pipe, NULL, NULL))
	    {
	      termios_printf ("PeekNamedPipe failed, %E");
	      raise (SIGHUP);
	      bytes_in_pipe = 0;
	    }
	  if (n)
	    {
	      len -= n;
	      totalread += n;
	      if (ptr)
		{
		  memcpy (ptr, buf, n);
		  ptr = (char *) ptr + n;
		}
	    }
	}

      if (!bytes_in_pipe)
	ResetEvent (input_available_event);

      ReleaseMutex (input_mutex);

      if (!ptr)
	{
	  if (!bytes_in_pipe)
	    break;
	  continue;
	}

      if (get_ttyp ()->read_retval < 0)	// read error
	{
	  set_errno (-get_ttyp ()->read_retval);
	  totalread = -1;
	  break;
	}
      if (get_ttyp ()->read_retval == 0)	//EOF
	{
	  termios_printf ("saw EOF");
	  break;
	}
      if (get_ttyp ()->ti.c_lflag & ICANON || is_nonblocking ())
	break;
      if (vmin && totalread >= vmin)
	break;

      /* vmin == 0 && vtime == 0:
       *   we've already read all input, if any, so return immediately
       * vmin == 0 && vtime > 0:
       *   we've waited for input 10*vtime ms in WFSO(input_available_event),
       *   no matter whether any input arrived, we shouldn't wait any longer,
       *   so return immediately
       * vmin > 0 && vtime == 0:
       *   here, totalread < vmin, so continue waiting until more data
       *   arrive
       * vmin > 0 && vtime > 0:
       *   similar to the previous here, totalread < vmin, and timer
       *   hadn't expired -- WFSO(input_available_event) != WAIT_TIMEOUT,
       *   so "restart timer" and wait until more data arrive
       */

      if (vmin == 0)
	break;

      if (n)
	waiter = time_to_wait;
    }
  termios_printf ("%d=read(%x, %d)", totalread, ptr, len);
  len = (size_t) totalread;
}

int
fhandler_tty_slave::dup (fhandler_base *child)
{
  fhandler_tty_slave *arch = (fhandler_tty_slave *) archetype;
  *(fhandler_tty_slave *) child = *arch;
  child->set_flags (get_flags ());
  child->usecount = 0;
  arch->usecount++;
  cygheap->manage_console_count ("fhandler_tty_slave::dup", 1);
  report_tty_counts (child, "duped", "");
  return 0;
}

int
fhandler_pty_master::dup (fhandler_base *child)
{
  fhandler_tty_master *arch = (fhandler_tty_master *) archetype;
  *(fhandler_tty_master *) child = *arch;
  child->set_flags (get_flags ());
  child->usecount = 0;
  arch->usecount++;
  report_tty_counts (child, "duped master", "");
  return 0;
}

int
fhandler_tty_slave::tcgetattr (struct termios *t)
{
  *t = get_ttyp ()->ti;
  return 0;
}

int
fhandler_tty_slave::tcsetattr (int, const struct termios *t)
{
  acquire_output_mutex (INFINITE);
  get_ttyp ()->ti = *t;
  release_output_mutex ();
  return 0;
}

int
fhandler_tty_slave::tcflush (int queue)
{
  int ret = 0;

  termios_printf ("tcflush(%d) handle %p", queue, get_handle ());

  if (queue == TCIFLUSH || queue == TCIOFLUSH)
    {
      size_t len = UINT_MAX;
      read (NULL, len);
      ret = ((int) len) >= 0 ? 0 : -1;
    }
  if (queue == TCOFLUSH || queue == TCIOFLUSH)
    {
      /* do nothing for now. */
    }

  termios_printf ("%d=tcflush(%d)", ret, queue);
  return ret;
}

int
fhandler_tty_slave::ioctl (unsigned int cmd, void *arg)
{
  termios_printf ("ioctl (%x)", cmd);

  if (myself->pgid && get_ttyp ()->getpgid () != myself->pgid
      && myself->ctty == get_unit () && (get_ttyp ()->ti.c_lflag & TOSTOP))
    {
      /* background process */
      termios_printf ("bg ioctl pgid %d, tpgid %d, %s", myself->pgid,
		      get_ttyp ()->getpgid (), myctty ());
      raise (SIGTTOU);
    }

  int retval;
  switch (cmd)
    {
    case TIOCGWINSZ:
    case TIOCSWINSZ:
    case TIOCLINUX:
    case KDGKBMETA:
    case KDSKBMETA:
      break;
    case FIONBIO:
      set_nonblocking (*(int *) arg);
      retval = 0;
      goto out;
    default:
      set_errno (EINVAL);
      return -1;
    }

  acquire_output_mutex (INFINITE);

  get_ttyp ()->cmd = cmd;
  get_ttyp ()->ioctl_retval = 0;
  int val;
  switch (cmd)
    {
    case TIOCGWINSZ:
      get_ttyp ()->arg.winsize = get_ttyp ()->winsize;
      if (ioctl_request_event)
	SetEvent (ioctl_request_event);
      *(struct winsize *) arg = get_ttyp ()->arg.winsize;
      if (ioctl_done_event)
	WaitForSingleObject (ioctl_done_event, INFINITE);
      get_ttyp ()->winsize = get_ttyp ()->arg.winsize;
      break;
    case TIOCSWINSZ:
      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col)
	{
	  get_ttyp ()->arg.winsize = *(struct winsize *) arg;
	  if (ioctl_request_event)
	    {
	      get_ttyp ()->ioctl_retval = -EINVAL;
	      SetEvent (ioctl_request_event);
	    }
	  else
	    {
	      get_ttyp ()->winsize = *(struct winsize *) arg;
	      killsys (-get_ttyp ()->getpgid (), SIGWINCH);
	    }
	  if (ioctl_done_event)
	    WaitForSingleObject (ioctl_done_event, INFINITE);
	}
      break;
    case TIOCLINUX:
      val = *(unsigned char *) arg;
      if (val != 6 || !ioctl_request_event || !ioctl_done_event)
	  get_ttyp ()->ioctl_retval = -EINVAL;
      else
	{
	  get_ttyp ()->arg.value = val;
	  SetEvent (ioctl_request_event);
	  WaitForSingleObject (ioctl_done_event, INFINITE);
	  *(unsigned char *) arg = get_ttyp ()->arg.value & 0xFF;
	}
      break;
    case KDGKBMETA:
      if (ioctl_request_event)
	{
	  SetEvent (ioctl_request_event);
	  if (ioctl_done_event)
	    WaitForSingleObject (ioctl_done_event, INFINITE);
	  *(int *) arg = get_ttyp ()->arg.value;
	}
      else
	get_ttyp ()->ioctl_retval = -EINVAL;
      break;
    case KDSKBMETA:
      if (ioctl_request_event)
	{
	  get_ttyp ()->arg.value = (int) arg;
	  SetEvent (ioctl_request_event);
	  if (ioctl_done_event)
	    WaitForSingleObject (ioctl_done_event, INFINITE);
	}
      else
	get_ttyp ()->ioctl_retval = -EINVAL;
      break;
    }

  release_output_mutex ();
  retval = get_ttyp ()->ioctl_retval;
  if (retval < 0)
    {
      set_errno (-retval);
      retval = -1;
    }

out:
  termios_printf ("%d = ioctl (%x)", retval, cmd);
  return retval;
}

/*******************************************************
 fhandler_pty_master
*/
fhandler_pty_master::fhandler_pty_master ()
  : fhandler_tty_common (), pktmode (0), need_nl (0), dwProcessId (0)
{
}

int
fhandler_pty_master::open (int flags, mode_t)
{
  int ntty;
  ntty = cygwin_shared->tty.allocate (false);
  if (ntty < 0)
    return 0;

  dev().devn = FHDEV (DEV_TTYM_MAJOR, ntty);
  if (!setup (true))
    {
      lock_ttys::release ();
      return 0;
    }
  lock_ttys::release ();
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  set_open_status ();
  //
  // FIXME: Do this better someday
  fhandler_pty_master *arch = (fhandler_tty_master *) cmalloc_abort (HEAP_ARCHETYPES, sizeof (*this));
  *((fhandler_pty_master **) cygheap->fdtab.add_archetype ()) = arch;
  archetype = arch;
  *arch = *this;
  arch->dwProcessId = GetCurrentProcessId ();

  usecount = 0;
  arch->usecount++;
  char buf[sizeof ("opened pty master for ttyNNNNNNNNNNN")];
  __small_sprintf (buf, "opened pty master for tty%d", get_unit ());
  report_tty_counts (this, buf, "");
  return 1;
}

_off64_t
fhandler_tty_common::lseek (_off64_t, int)
{
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_tty_common::close ()
{
  termios_printf ("tty%d <%p,%p> closing", get_unit (), get_handle (), get_output_handle ());
  if (output_done_event && !CloseHandle (output_done_event))
    termios_printf ("CloseHandle (output_done_event), %E");
  if (ioctl_done_event && !CloseHandle (ioctl_done_event))
    termios_printf ("CloseHandle (ioctl_done_event), %E");
  if (ioctl_request_event && !CloseHandle (ioctl_request_event))
    termios_printf ("CloseHandle (ioctl_request_event), %E");
  if (!ForceCloseHandle (input_mutex))
    termios_printf ("CloseHandle (input_mutex<%p>), %E", input_mutex);
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  if (!ForceCloseHandle1 (get_handle (), from_pty))
    termios_printf ("CloseHandle (get_handle ()<%p>), %E", get_handle ());
  if (!ForceCloseHandle1 (get_output_handle (), to_pty))
    termios_printf ("CloseHandle (get_output_handle ()<%p>), %E", get_output_handle ());

  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E", input_available_event);

  return 0;
}

int
fhandler_pty_master::close ()
{
#if 0
  while (accept_input () > 0)
    continue;
#endif
  archetype->usecount--;
  report_tty_counts (this, "closing master", "");

  if (archetype->usecount)
    {
#ifdef DEBUGGING
      if (archetype->usecount < 0)
	system_printf ("error: usecount %d", archetype->usecount);
#endif
      termios_printf ("just returning because archetype usecount is != 0");
      return 0;
    }

  fhandler_tty_master *arch = (fhandler_tty_master *) archetype;
  termios_printf ("closing from_master(%p)/to_master(%p) since we own them(%d)",
		  arch->from_master, arch->to_master, arch->dwProcessId);
  if (!ForceCloseHandle (arch->from_master))
    termios_printf ("error closing from_master %p, %E", arch->from_master);
  if (!ForceCloseHandle (arch->to_master))
    termios_printf ("error closing from_master %p, %E", arch->to_master);
  fhandler_tty_common::close ();

  if (hExeced || get_ttyp ()->master_pid != myself->pid)
    termios_printf ("not clearing: %d, master_pid %d", hExeced, get_ttyp ()->master_pid);
  else
    get_ttyp ()->set_master_closed ();

  return 0;
}

int
fhandler_pty_master::write (const void *ptr, size_t len)
{
  int i;
  char *p = (char *) ptr;
  termios ti = tc->ti;

  for (i = 0; i < (int) len; i++)
    {
      line_edit_status status = line_edit (p++, 1, ti);
      if (status > line_edit_signalled)
	{
	  if (status != line_edit_pipe_full)
	    i = -1;
	  break;
	}
    }
  return i;
}

void __stdcall
fhandler_pty_master::read (void *ptr, size_t& len)
{
  len = (size_t) process_slave_output ((char *) ptr, len, pktmode);
}

int
fhandler_pty_master::tcgetattr (struct termios *t)
{
  *t = cygwin_shared->tty[get_unit ()]->ti;
  return 0;
}

int
fhandler_pty_master::tcsetattr (int, const struct termios *t)
{
  cygwin_shared->tty[get_unit ()]->ti = *t;
  return 0;
}

int
fhandler_pty_master::tcflush (int queue)
{
  int ret = 0;

  termios_printf ("tcflush(%d) handle %p", queue, get_handle ());

  if (queue == TCIFLUSH || queue == TCIOFLUSH)
    ret = process_slave_output (NULL, OUT_BUFFER_SIZE, 0);
  else if (queue == TCIFLUSH || queue == TCIOFLUSH)
    {
      /* do nothing for now. */
    }

  termios_printf ("%d=tcflush(%d)", ret, queue);
  return ret;
}

int
fhandler_pty_master::ioctl (unsigned int cmd, void *arg)
{
  switch (cmd)
    {
      case TIOCPKT:
	pktmode = *(int *) arg;
	break;
      case TIOCGWINSZ:
	*(struct winsize *) arg = get_ttyp ()->winsize;
	break;
      case TIOCSWINSZ:
	if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	    || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col)
	  {
	    get_ttyp ()->winsize = *(struct winsize *) arg;
	    killsys (-get_ttyp ()->getpgid (), SIGWINCH);
	  }
	break;
      case FIONBIO:
	set_nonblocking (*(int *) arg);
	break;
      default:
	set_errno (EINVAL);
	return -1;
    }
  return 0;
}

char *
fhandler_pty_master::ptsname ()
{
  static char buf[TTY_NAME_MAX];

  __small_sprintf (buf, "/dev/tty%d", get_unit ());
  return buf;
}

void
fhandler_tty_common::set_close_on_exec (bool val)
{
  // Cygwin processes will handle this specially on exec.
  close_on_exec (val);
}

void
fhandler_tty_slave::fixup_after_fork (HANDLE parent)
{
  // fork_fixup (parent, inuse, "inuse");
  // fhandler_tty_common::fixup_after_fork (parent);
  report_tty_counts (this, "inherited", "");
}

void
fhandler_tty_slave::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (NULL);
}

int
fhandler_tty_master::init_console ()
{
  console = (fhandler_console *) build_fh_dev (*console_dev, "/dev/ttym");
  if (console == NULL)
    return -1;

  console->init (INVALID_HANDLE_VALUE, GENERIC_READ | GENERIC_WRITE, O_BINARY);
  cygheap->manage_console_count ("fhandler_tty_master::init_console", -1, true);
  console->uninterruptible_io (true);
  return 0;
}

#define close_maybe(h) \
  do { \
    if (h) \
      CloseHandle (h); \
  } while (0)

bool
fhandler_pty_master::setup (bool ispty)
{
  int res;
  tty& t = *cygwin_shared->tty[get_unit ()];

  tcinit (&t, true);		/* Set termios information.  Force initialization. */

  const char *errstr = NULL;
  DWORD pipe_mode = PIPE_NOWAIT;

  /* Create communication pipes */

  /* FIXME: should this be sec_none_nih? */

  /* Create communication pipes */

  char pipename[sizeof("ttyNNNN-from-master")];
  __small_sprintf (pipename, "tty%d-from-master", get_unit ());
  res = fhandler_pipe::create_selectable (&sec_all_nih, from_master,
					  get_output_handle (), 128 * 1024,
					  pipename);
  if (res)
    {
      errstr = "input pipe";
      goto err;
    }

  __small_sprintf (pipename, "tty%d-to-master", get_unit ());
  res = fhandler_pipe::create_selectable (&sec_all_nih, get_io_handle (),
					  to_master, 128 * 1024, pipename);
  if (res)
    {
      errstr = "output pipe";
      goto err;
    }

  if (!SetNamedPipeHandleState (get_output_handle (), &pipe_mode, NULL, NULL))
    termios_printf ("can't set output_handle(%p) to non-blocking mode",
		    get_output_handle ());

  need_nl = 0;

  /* We do not allow others to open us (for handle duplication)
     but rely on cygheap->inherited_ctty for descendant processes.
     In the future the cygserver may allow access by others. */

#ifdef USE_SERVER
  if (cygserver_running == CYGSERVER_UNKNOWN)
    cygserver_init ();
#endif

  /* Create synchronisation events */

  if (!ispty)
    {
      if (!(output_done_event = t.get_event (errstr = OUTPUT_DONE_EVENT)))
	goto err;
      if (!(ioctl_done_event = t.get_event (errstr = IOCTL_DONE_EVENT)))
	goto err;
      if (!(ioctl_request_event = t.get_event (errstr = IOCTL_REQUEST_EVENT)))
	goto err;
    }

  if (!(input_available_event = t.get_event (errstr = INPUT_AVAILABLE_EVENT, TRUE)))
    goto err;

  char buf[MAX_PATH];
  errstr = shared_name (buf, OUTPUT_MUTEX, t.ntty);
  if (!(output_mutex = CreateMutex (&sec_all, FALSE, buf)))
    goto err;

  errstr = shared_name (buf, INPUT_MUTEX, t.ntty);
  if (!(input_mutex = CreateMutex (&sec_all, FALSE, buf)))
    goto err;

  if (!DuplicateHandle (hMainProc, from_master, hMainProc, &from_master, 0, false,
			DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    {
      errstr = "non-inheritable from_master";
      goto err;
    }

  if (!DuplicateHandle (hMainProc, to_master, hMainProc, &to_master, 0, false,
			DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    {
      errstr = "non-inheritable to_master";
      goto err;
    }

  t.from_master = from_master;
  t.to_master = to_master;
  // /* screws up tty master */ ProtectHandle1INH (output_mutex, output_mutex);
  // /* screws up tty master */ ProtectHandle1INH (input_mutex, input_mutex);
  t.winsize.ws_col = 80;
  t.winsize.ws_row = 25;
  t.master_pid = myself->pid;

  termios_printf ("tty%d opened - from_slave %p, to_slave %p", t.ntty,
		  get_io_handle (), get_output_handle ());
  return true;

err:
  __seterrno ();
  close_maybe (get_io_handle ());
  close_maybe (get_output_handle ());
  close_maybe (output_done_event);
  close_maybe (ioctl_done_event);
  close_maybe (ioctl_request_event);
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (from_master);
  close_maybe (to_master);
  termios_printf ("tty%d open failed - failed to create %s", errstr);
  return false;
}

void
fhandler_pty_master::fixup_after_fork (HANDLE parent)
{
  DWORD wpid = GetCurrentProcessId ();
  fhandler_tty_master *arch = (fhandler_tty_master *) archetype;
  if (arch->dwProcessId != wpid)
    {
      tty& t = *get_ttyp ();
      if (!DuplicateHandle (parent, arch->from_master, hMainProc,
			    &arch->from_master, 0, false, DUPLICATE_SAME_ACCESS))
	system_printf ("couldn't duplicate from_parent(%p), %E", arch->from_master);
      if (!DuplicateHandle (parent, arch->to_master, hMainProc,
			    &arch->to_master, 0, false, DUPLICATE_SAME_ACCESS))
	system_printf ("couldn't duplicate to_parent(%p), %E", arch->from_master);
      if (myself->pid == t.master_pid)
	{
	  t.from_master = arch->from_master;
	  t.to_master = arch->to_master;
	}
      arch->dwProcessId = wpid;
    }
  from_master = arch->from_master;
  to_master = arch->to_master;
  report_tty_counts (this, "inherited master", "");
}

void
fhandler_pty_master::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (spawn_info->parent);
  else
    from_master = to_master = NULL;
}
