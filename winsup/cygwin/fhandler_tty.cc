/* fhandler_tty.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/param.h>
#include <cygwin/acl.h>
#include <cygwin/kd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "ntdll.h"
#include "cygheap.h"
#include "shared_info.h"
#include "cygthread.h"
#include "child_info.h"
#include <asm/socket.h>
#include "cygwait.h"

#define close_maybe(h) \
  do { \
    if (h && h != INVALID_HANDLE_VALUE) \
      CloseHandle (h); \
  } while (0)

/* pty master control pipe messages */
struct pipe_request {
  DWORD pid;
};

struct pipe_reply {
  HANDLE from_master;
  HANDLE to_master;
  HANDLE to_master_cyg;
  DWORD error;
};

bool
bytes_available (DWORD& n, HANDLE h)
{
  DWORD navail, nleft;
  navail = nleft = 0;
  bool succeeded = PeekNamedPipe (h, NULL, 0, NULL, &navail, &nleft);
  if (succeeded)
    /* nleft should always be the right choice unless something has written 0
       bytes to the pipe.  In that pathological case we return the actual number
       of bytes available in the pipe. See cgf-000008 for more details.  */
    n = nleft ?: navail;
  else
    {
      termios_printf ("PeekNamedPipe(%p) failed, %E", h);
      n = 0;
    }
  debug_only_printf ("n %u, nleft %u, navail %u", n, nleft, navail);
  return succeeded;
}

bool
fhandler_pty_common::bytes_available (DWORD &n)
{
  return ::bytes_available (n, get_handle ());
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

void
fhandler_pty_master::flush_to_slave ()
{
  if (get_readahead_valid () && !(get_ttyp ()->ti.c_lflag & ICANON))
    accept_input ();
}

DWORD
fhandler_pty_common::__acquire_output_mutex (const char *fn, int ln,
					     DWORD ms)
{
  if (strace.active ())
    strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex (%p): waiting %d ms", ln, output_mutex, ms);
  if (ms == INFINITE)
    ms = 100;
  DWORD res = WaitForSingleObject (output_mutex, ms);
  if (res == WAIT_OBJECT_0)
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex: acquired", ln, res);
#else
      ostack[osi].fn = fn;
      ostack[osi].ln = ln;
      ostack[osi].tname = mythreadname ();
      termios_printf ("acquired for %s:%d, osi %d", fn, ln, osi);
      osi++;
#endif
    }
  return res;
}

void
fhandler_pty_common::__release_output_mutex (const char *fn, int ln)
{
  if (ReleaseMutex (output_mutex))
    {
#ifndef DEBUGGING
      if (strace.active ())
	strace.prntf (_STRACE_TERMIOS, fn, "(%d): pty output_mutex(%p) released", ln, output_mutex);
#else
      if (osi > 0)
	osi--;
      termios_printf ("released(%p) at %s:%d, osi %d", output_mutex, fn, ln, osi);
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

/* Process pty input. */

void
fhandler_pty_master::doecho (const void *str, DWORD len)
{
  ssize_t towrite = len;
  if (!process_opost_output (echo_w, str, towrite, true))
    termios_printf ("Write to echo pipe failed, %E");
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

      paranoid_printf ("about to write %u chars to slave", bytes_left);
      rc = WriteFile (get_output_handle (), p, bytes_left, &written, NULL);
      if (!rc)
	{
	  debug_printf ("error writing to pipe %p %E", get_output_handle ());
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

/* Process pty output requests */

int
fhandler_pty_master::process_slave_output (char *buf, size_t len, int pktmode_on)
{
  size_t rlen;
  char outbuf[OUT_BUFFER_SIZE];
  DWORD n;
  DWORD echo_cnt;
  int rc = 0;

  flush_to_slave ();

  if (len == 0)
    goto out;

  for (;;)
    {
      n = echo_cnt = 0;
      for (;;)
	{
	  /* Check echo pipe first. */
	  if (::bytes_available (echo_cnt, echo_r) && echo_cnt > 0)
	    break;
	  if (!::bytes_available (n, get_io_handle_cyg ()))
	    goto err;
	  if (n)
	    break;
	  if (hit_eof ())
	    {
	      set_errno (EIO);
	      rc = -1;
	      goto out;
	    }
	  /* DISCARD (FLUSHO) and tcflush can finish here. */
	  if ((get_ttyp ()->ti.c_lflag & FLUSHO || !buf))
	    goto out;

	  if (is_nonblocking ())
	    {
	      set_errno (EAGAIN);
	      rc = -1;
	      goto out;
	    }
	  pthread_testcancel ();
	  if (cygwait (NULL, 10, cw_sig_eintr) == WAIT_SIGNALED
	      && !_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      rc = -1;
	      goto out;
	    }
	  flush_to_slave ();
	}

      /* Set RLEN to the number of bytes to read from the pipe.  */
      rlen = len;

      char *optr;
      optr = buf;
      if (pktmode_on && buf)
	{
	  *optr++ = TIOCPKT_DATA;
	  rlen -= 1;
	}

      if (rlen == 0)
	{
	  rc = optr - buf;
	  goto out;
	}

      if (rlen > sizeof outbuf)
	rlen = sizeof outbuf;

      /* If echo pipe has data (something has been typed or pasted), prefer
         it over slave output. */
      if (echo_cnt > 0)
      	{
	  if (!ReadFile (echo_r, outbuf, rlen, &n, NULL))
	    {
	      termios_printf ("ReadFile on echo pipe failed, %E");
	      goto err;
	    }
	}
      else if (!ReadFile (get_io_handle_cyg (), outbuf, rlen, &n, NULL))
	{
	  termios_printf ("ReadFile failed, %E");
	  goto err;
	}

      termios_printf ("bytes read %u", n);

      if (get_ttyp ()->ti.c_lflag & FLUSHO || !buf)
	continue;

      memcpy (optr, outbuf, n);
      optr += n;
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

/* pty slave stuff */

fhandler_pty_slave::fhandler_pty_slave (int unit)
  : fhandler_pty_common (), inuse (NULL), output_handle_cyg (NULL)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYS_MAJOR, unit);
}

int
fhandler_pty_slave::open (int flags, mode_t)
{
  HANDLE pty_owner, from_master_local, to_master_local, to_master_cyg_local;
  HANDLE *handles[] =
  {
    &from_master_local, &input_available_event, &input_mutex, &inuse,
    &output_mutex, &to_master_local, &pty_owner, &to_master_cyg_local,
    NULL
  };

  for (HANDLE **h = handles; *h; h++)
    **h = NULL;

  _tc = cygwin_shared->tty[get_minor ()];

  tcinit (false);

  cygwin_shared->tty.attach (get_minor ());

  /* Create synchronisation events */
  char buf[MAX_PATH];

  const char *errmsg = NULL;

  if (!(output_mutex = get_ttyp ()->open_output_mutex (MAXIMUM_ALLOWED)))
    {
      errmsg = "open output mutex failed, %E";
      goto err;
    }
  if (!(input_mutex = get_ttyp ()->open_input_mutex (MAXIMUM_ALLOWED)))
    {
      errmsg = "open input mutex failed, %E";
      goto err;
    }
  shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
  if (!(input_available_event = OpenEvent (MAXIMUM_ALLOWED, TRUE, buf)))
    {
      errmsg = "open input event failed, %E";
      goto err;
    }

  /* FIXME: Needs a method to eliminate tty races */
  {
    /* Create security attribute.  Default permissions are 0620. */
    security_descriptor sd;
    sd.malloc (sizeof (SECURITY_DESCRIPTOR));
    RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
    SECURITY_ATTRIBUTES sa = { sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };
    if (!create_object_sd_from_attribute (myself->uid, myself->gid,
					  S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP,
					  sd))
      sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) sd;
    acquire_output_mutex (INFINITE);
    inuse = get_ttyp ()->create_inuse (&sa);
    get_ttyp ()->was_opened = true;
    release_output_mutex ();
  }

  if (!get_ttyp ()->from_master () ||
      !get_ttyp ()->to_master () || !get_ttyp ()->to_master_cyg ())
    {
      errmsg = "pty handles have been closed";
      set_errno (EACCES);
      goto err_no_errno;
    }

  /* Three case for duplicating the pipe handles:
     - Either we're the master.  In this case, just duplicate the handles.
     - Or, we have the right to open the master process for handle duplication.
       In this case, just duplicate the handles.
     - Or, we have to ask the master process itself.  In this case, send our
       pid to the master process and check the reply.  The reply contains
       either the handles, or an error code which tells us why we didn't
       get the handles. */
  if (myself->pid == get_ttyp ()->master_pid)
    {
      /* This is the most common case, just calling openpty. */
      termios_printf ("dup handles within myself.");
      pty_owner = GetCurrentProcess ();
    }
  else
    {
      pinfo p (get_ttyp ()->master_pid);
      if (!p)
	termios_printf ("*** couldn't find pty master");
      else
	{
	  pty_owner = OpenProcess (PROCESS_DUP_HANDLE, FALSE, p->dwProcessId);
	  if (pty_owner)
	    termios_printf ("dup handles directly since I'm the owner");
	}
    }
  if (pty_owner)
    {
      if (!DuplicateHandle (pty_owner, get_ttyp ()->from_master (),
			    GetCurrentProcess (), &from_master_local, 0, TRUE,
			    DUPLICATE_SAME_ACCESS))
	{
	  termios_printf ("can't duplicate input from %u/%p, %E",
			  get_ttyp ()->master_pid, get_ttyp ()->from_master ());
	  __seterrno ();
	  goto err_no_msg;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master (),
			  GetCurrentProcess (), &to_master_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  errmsg = "can't duplicate output, %E";
	  goto err;
	}
      if (!DuplicateHandle (pty_owner, get_ttyp ()->to_master_cyg (),
			  GetCurrentProcess (), &to_master_cyg_local, 0, TRUE,
			  DUPLICATE_SAME_ACCESS))
	{
	  errmsg = "can't duplicate output for cygwin, %E";
	  goto err;
	}
      if (pty_owner != GetCurrentProcess ())
	CloseHandle (pty_owner);
    }
  else
    {
      pipe_request req = { GetCurrentProcessId () };
      pipe_reply repl;
      DWORD len;

      __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
		       &cygheap->installation_key, get_minor ());
      termios_printf ("dup handles via master control pipe %s", buf);
      if (!CallNamedPipe (buf, &req, sizeof req, &repl, sizeof repl,
			  &len, 500))
	{
	  errmsg = "can't call master, %E";
	  goto err;
	}
      from_master_local = repl.from_master;
      to_master_local = repl.to_master;
      to_master_cyg_local = repl.to_master_cyg;
      if (!from_master_local || !to_master_local || !to_master_cyg_local)
	{
	  SetLastError (repl.error);
	  errmsg = "error duplicating pipes, %E";
	  goto err;
	}
    }
  VerifyHandle (from_master_local);
  VerifyHandle (to_master_local);
  VerifyHandle (to_master_cyg_local);

  termios_printf ("duplicated from_master %p->%p from pty_owner",
		  get_ttyp ()->from_master (), from_master_local);
  termios_printf ("duplicated to_master %p->%p from pty_owner",
		  get_ttyp ()->to_master (), to_master_local);
  termios_printf ("duplicated to_master_cyg %p->%p from pty_owner",
		  get_ttyp ()->to_master_cyg (), to_master_cyg_local);

  set_io_handle (from_master_local);
  set_output_handle (to_master_local);
  set_output_handle_cyg (to_master_cyg_local);

  fhandler_console::need_invisible ();
  set_open_status ();
  return 1;

err:
  if (GetLastError () == ERROR_FILE_NOT_FOUND)
    set_errno (ENXIO);
  else
    __seterrno ();
err_no_errno:
  termios_printf (errmsg);
err_no_msg:
  for (HANDLE **h = handles; *h; h++)
    if (**h && **h != INVALID_HANDLE_VALUE)
      CloseHandle (**h);
  return 0;
}

void
fhandler_pty_slave::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  myself->set_ctty (this, flags);
  report_tty_counts (this, "opened", "");
  fhandler_base::open_setup (flags);
}

void
fhandler_pty_slave::cleanup ()
{
  /* This used to always call fhandler_pty_common::close when we were execing
     but that caused multiple closes of the handles associated with this pty.
     Since close_all_files is not called until after the cygwin process has
     synced or before a non-cygwin process has exited, it should be safe to
     just close this normally.  cgf 2006-05-20 */
  report_tty_counts (this, "closed", "");
  fhandler_base::cleanup ();
}

int
fhandler_pty_slave::close ()
{
  termios_printf ("closing last open %s handle", ttyname ());
  if (inuse && !CloseHandle (inuse))
    termios_printf ("CloseHandle (inuse), %E");
  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E", input_available_event);
  if (!ForceCloseHandle (get_output_handle_cyg ()))
    termios_printf ("CloseHandle (get_output_handle_cyg ()<%p>), %E",
	get_output_handle_cyg ());
  if ((unsigned) myself->ctty == FHDEV (DEV_PTYS_MAJOR, get_minor ()))
    fhandler_console::free_console ();	/* assumes that we are the last pty closer */
  fhandler_pty_common::close ();
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  return 0;
}

int
fhandler_pty_slave::init (HANDLE h, DWORD a, mode_t)
{
  int flags = 0;

  a &= GENERIC_READ | GENERIC_WRITE;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;

  int ret = open_with_arch (flags);

  if (ret && !cygwin_finished_initializing && !being_debugged ())
    {
      /* This only occurs when called from dtable::init_std_file_from_handle
	 We have been started from a non-Cygwin process.  So we should become
	 pty process group leader.
	 TODO: Investigate how SIGTTIN should be handled with pure-windows
	 programs. */
      pinfo p (tc ()->getpgid ());
      /* We should only grab this when the process group owner for this
	 pty is a non-cygwin process or we've been started directly
	 from a non-Cygwin process with no Cygwin ancestry.  */
      if (!p || ISSTATE (p, PID_NOTCYGWIN))
	{
	  termios_printf ("Setting process group leader to %d since %W(%d) is not a cygwin process",
			  myself->pgid, p->progname, p->pid);
	  tc ()->setpgid (myself->pgid);
	}
    }

  if (h != INVALID_HANDLE_VALUE)
    CloseHandle (h);	/* Reopened by open */

  return ret;
}

ssize_t __stdcall
fhandler_pty_slave::write (const void *ptr, size_t len)
{
  ssize_t towrite = len;

  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  termios_printf ("pty%d, write(%p, %lu)", get_minor (), ptr, len);

  push_process_state process_state (PID_TTYOU);

  if (!process_opost_output (get_output_handle_cyg (), ptr, towrite, false))
    {
      DWORD err = GetLastError ();
      termios_printf ("WriteFile failed, %E");
      switch (err)
	{
	case ERROR_NO_DATA:
	  err = ERROR_IO_DEVICE;
	  /*FALLTHRU*/
	default:
	  __seterrno_from_win_error (err);
	}
      towrite = -1;
    }
  return towrite;
}

void __reg3
fhandler_pty_slave::read (void *ptr, size_t& len)
{
  ssize_t totalread = 0;
  int vmin = 0;
  int vtime = 0;	/* Initialized to prevent -Wuninitialized warning */
  size_t readlen;
  DWORD bytes_in_pipe;
  char buf[INP_BUFFER_SIZE];
  DWORD time_to_wait;

  bg_check_types bg = bg_check (SIGTTIN);
  if (bg <= bg_eof)
    {
      len = (size_t) bg;
      return;
    }

  termios_printf ("read(%p, %lu) handle %p", ptr, len, get_handle ());

  push_process_state process_state (PID_TTYIN);

  if (is_nonblocking () || !ptr) /* Indicating tcflush(). */
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

  while (len)
    {
      switch (cygwait (input_available_event, time_to_wait))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_SIGNALED:
	  if (totalread > 0)
	    goto out;
	  termios_printf ("wait catched signal");
	  set_sig_errno (EINTR);
	  totalread = -1;
	  goto out;
	case WAIT_CANCELED:
	  process_state.pop ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	case WAIT_TIMEOUT:
	  termios_printf ("wait timed out, time_to_wait %u", time_to_wait);
	  /* No error condition when called from tcflush. */
	  if (!totalread && ptr)
	    {
	      set_sig_errno (EAGAIN);
	      totalread = -1;
	    }
	  goto out;
	default:
	  termios_printf ("wait for input event failed, %E");
	  if (!totalread)
	    {
	      __seterrno ();
	      totalread = -1;
	    }
	  goto out;
	}
      /* Now that we know that input is available we have to grab the
	 input mutex. */
      switch (cygwait (input_mutex, 1000))
	{
	case WAIT_OBJECT_0:
	case WAIT_ABANDONED_0:
	  break;
	case WAIT_SIGNALED:
	  if (totalread > 0)
	    goto out;
	  termios_printf ("wait for mutex caught signal");
	  set_sig_errno (EINTR);
	  totalread = -1;
	  goto out;
	case WAIT_CANCELED:
	  process_state.pop ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	case WAIT_TIMEOUT:
	  termios_printf ("failed to acquire input mutex after input event "
			  "arrived");
	  /* If we have a timeout, we can simply handle this failure to
	     grab the mutex as an EAGAIN situation.  Otherwise, if this
	     is an infinitely blocking read, restart the loop. */
	  if (time_to_wait != INFINITE)
	    {
	      if (!totalread)
		{
		  set_sig_errno (EAGAIN);
		  totalread = -1;
		}
	      goto out;
	    }
	  continue;
	default:
	  termios_printf ("wait for input mutex failed, %E");
	  if (!totalread)
	    {
	      __seterrno ();
	      totalread = -1;
	    }
	  goto out;
	}
      if (!bytes_available (bytes_in_pipe))
	{
	  ReleaseMutex (input_mutex);
	  set_errno (EIO);
	  totalread = -1;
	  goto out;
	}

      if (ptr && !bytes_in_pipe && !vmin && !time_to_wait)
	{
	  ReleaseMutex (input_mutex);
	  len = (size_t) bytes_in_pipe;
	  return;
	}

      readlen = bytes_in_pipe ? MIN (len, sizeof (buf)) : 0;
      if (get_ttyp ()->ti.c_lflag & ICANON && ptr)
	readlen = MIN (bytes_in_pipe, readlen);

#if 0
      /* Why on earth is the read length reduced to vmin, even if more bytes
	 are available *and* len is bigger *and* the local buf is big enough?
	 Disable this code for now, it looks like a remnant of old. */
      if (ptr && vmin && readlen > (unsigned) vmin)
	readlen = vmin;
#endif

      DWORD n = 0;
      if (readlen)
	{
	  termios_printf ("reading %lu bytes (vtime %d)", readlen, vtime);
	  if (!ReadFile (get_handle (), buf, readlen, &n, NULL))
	    {
	      termios_printf ("read failed, %E");
	      ReleaseMutex (input_mutex);
	      set_errno (EIO);
	      totalread = -1;
	      goto out;
	    }
	  else
	    {
	      /* MSDN states that 5th prameter can be used to determine total
		 number of bytes in pipe, but for some reason this number doesn't
		 change after successful read. So we have to peek into the pipe
		 again to see if input is still available */
	      if (!bytes_available (bytes_in_pipe))
		{
		  ReleaseMutex (input_mutex);
		  set_errno (EIO);
		  totalread = -1;
		  goto out;
		}
	      if (n)
		{
		  if (!(!ptr && len == UINT_MAX)) /* not tcflush() */
		    len -= n;
		  totalread += n;
		  if (ptr)
		    {
		      memcpy (ptr, buf, n);
		      ptr = (char *) ptr + n;
		    }
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
    }
out:
  termios_printf ("%d=read(%p, %lu)", totalread, ptr, len);
  len = (size_t) totalread;
}

int
fhandler_pty_slave::dup (fhandler_base *child, int flags)
{
  /* This code was added in Oct 2001 for some undisclosed reason.
     However, setting the controlling tty on a dup causes rxvt to
     hang when the parent does a dup since the controlling pgid changes.
     Specifically testing for -2 (ctty has been setsid'ed) works around
     this problem.  However, it's difficult to see scenarios in which you
     have a dup'able fd, no controlling tty, and not having run setsid.
     So, we might want to consider getting rid of the set_ctty in tty-like dup
     methods entirely at some point */
  if (myself->ctty != -2)
    myself->set_ctty (this, flags);
  report_tty_counts (child, "duped slave", "");
  return 0;
}

int
fhandler_pty_master::dup (fhandler_base *child, int)
{
  report_tty_counts (child, "duped master", "");
  return 0;
}

int
fhandler_pty_slave::tcgetattr (struct termios *t)
{
  *t = get_ttyp ()->ti;
  return 0;
}

int
fhandler_pty_slave::tcsetattr (int, const struct termios *t)
{
  acquire_output_mutex (INFINITE);
  get_ttyp ()->ti = *t;
  release_output_mutex ();
  return 0;
}

int
fhandler_pty_slave::tcflush (int queue)
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
fhandler_pty_slave::ioctl (unsigned int cmd, void *arg)
{
  termios_printf ("ioctl (%x)", cmd);
  int res = fhandler_termios::ioctl (cmd, arg);
  if (res <= 0)
    return res;

  if (myself->pgid && get_ttyp ()->getpgid () != myself->pgid
      && (unsigned) myself->ctty == FHDEV (DEV_PTYS_MAJOR, get_minor ())
      && (get_ttyp ()->ti.c_lflag & TOSTOP))
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
      break;
    case TIOCGPGRP:
      {
	pid_t pid = this->tcgetpgrp ();
	if (pid < 0)
	  retval = -1;
	else
	  {
	    *((pid_t *) arg) = pid;
	    retval = 0;
	  }
      }
      goto out;
    case TIOCSPGRP:
      retval = this->tcsetpgrp ((pid_t) (intptr_t) arg);
      goto out;
    case FIONREAD:
      {
	DWORD n;
	if (!bytes_available (n))
	  {
	    set_errno (EINVAL);
	    retval = -1;
	  }
	else
	  {
	    *(int *) arg = (int) n;
	    retval = 0;
	  }
      }
      goto out;
    default:
      return fhandler_base::ioctl (cmd, arg);
    }

  acquire_output_mutex (INFINITE);

  get_ttyp ()->cmd = cmd;
  get_ttyp ()->ioctl_retval = 0;
  switch (cmd)
    {
    case TIOCGWINSZ:
      get_ttyp ()->arg.winsize = get_ttyp ()->winsize;
      *(struct winsize *) arg = get_ttyp ()->arg.winsize;
      get_ttyp ()->winsize = get_ttyp ()->arg.winsize;
      break;
    case TIOCSWINSZ:
      if (get_ttyp ()->winsize.ws_row != ((struct winsize *) arg)->ws_row
	  || get_ttyp ()->winsize.ws_col != ((struct winsize *) arg)->ws_col)
	{
	  get_ttyp ()->arg.winsize = *(struct winsize *) arg;
	  get_ttyp ()->winsize = *(struct winsize *) arg;
	  get_ttyp ()->kill_pgrp (SIGWINCH);
	}
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
  termios_printf ("%d = ioctl(%x)", retval, cmd);
  return retval;
}

int __reg2
fhandler_pty_slave::fstat (struct stat *st)
{
  fhandler_base::fstat (st);

  bool to_close = false;
  if (!input_available_event)
    {
      char buf[MAX_PATH];
      shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
      input_available_event = OpenEvent (READ_CONTROL, TRUE, buf);
      if (input_available_event)
	to_close = true;
    }
  st->st_mode = S_IFCHR;
  if (!input_available_event
      || get_object_attribute (input_available_event, &st->st_uid, &st->st_gid,
			       &st->st_mode))
    {
      /* If we can't access the ACL, or if the tty doesn't actually exist,
	 then fake uid and gid to strict, system-like values. */
      st->st_mode = S_IFCHR | S_IRUSR | S_IWUSR;
      st->st_uid = 18;
      st->st_gid = 544;
    }
  if (to_close)
    CloseHandle (input_available_event);
  return 0;
}

int __reg3
fhandler_pty_slave::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  int res = -1;
  bool to_close = false;
  security_descriptor sd;
  mode_t attr = S_IFCHR;

  switch (cmd)
    {
      case SETACL:
	if (!aclsort32 (nentries, 0, aclbufp))
	  set_errno (ENOTSUP);
	break;
      case GETACL:
	if (!aclbufp)
	  {
	    set_errno (EFAULT);
	    break;
	  }
	/*FALLTHRU*/
      case GETACLCNT:
	if (!input_available_event)
	  {
	    char buf[MAX_PATH];
	    shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
	    input_available_event = OpenEvent (READ_CONTROL, TRUE, buf);
	    if (input_available_event)
	      to_close = true;
	  }
	if (!input_available_event
	    || get_object_sd (input_available_event, sd))
	  {
	    res = get_posix_access (NULL, &attr, NULL, NULL, aclbufp, nentries);
	    if (aclbufp && res == MIN_ACL_ENTRIES)
	      {
		aclbufp[0].a_perm = S_IROTH | S_IWOTH;
		aclbufp[0].a_id = 18;
		aclbufp[1].a_id = 544;
	      }
	    break;
	  }
	if (cmd == GETACL)
	  res = get_posix_access (sd, &attr, NULL, NULL, aclbufp, nentries);
	else
	  res = get_posix_access (sd, &attr, NULL, NULL, NULL, 0);
	break;
      default:
	set_errno (EINVAL);
	break;
    }
  if (to_close)
    CloseHandle (input_available_event);
  return res;
}

/* Helper function for fchmod and fchown, which just opens all handles
   and signals success via bool return. */
bool
fhandler_pty_slave::fch_open_handles (bool chown)
{
  char buf[MAX_PATH];
  DWORD write_access = WRITE_DAC | (chown ? WRITE_OWNER : 0);

  _tc = cygwin_shared->tty[get_minor ()];
  shared_name (buf, INPUT_AVAILABLE_EVENT, get_minor ());
  input_available_event = OpenEvent (READ_CONTROL | write_access,
				     TRUE, buf);
  output_mutex = get_ttyp ()->open_output_mutex (write_access);
  input_mutex = get_ttyp ()->open_input_mutex (write_access);
  inuse = get_ttyp ()->open_inuse (write_access);
  if (!input_available_event || !output_mutex || !input_mutex || !inuse)
    {
      __seterrno ();
      return false;
    }
  return true;
}

/* Helper function for fchmod and fchown, which sets the new security
   descriptor on all objects representing the pty. */
int
fhandler_pty_slave::fch_set_sd (security_descriptor &sd, bool chown)
{
  security_descriptor sd_old;

  get_object_sd (input_available_event, sd_old);
  if (!set_object_sd (input_available_event, sd, chown)
      && !set_object_sd (output_mutex, sd, chown)
      && !set_object_sd (input_mutex, sd, chown)
      && !set_object_sd (inuse, sd, chown))
    return 0;
  set_object_sd (input_available_event, sd_old, chown);
  set_object_sd (output_mutex, sd_old, chown);
  set_object_sd (input_mutex, sd_old, chown);
  set_object_sd (inuse, sd_old, chown);
  return -1;
}

/* Helper function for fchmod and fchown, which closes all object handles in
   the pty. */
void
fhandler_pty_slave::fch_close_handles ()
{
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (inuse);
}

int __reg1
fhandler_pty_slave::fchmod (mode_t mode)
{
  int ret = -1;
  bool to_close = false;
  security_descriptor sd;
  uid_t uid;
  gid_t gid;
  mode_t orig_mode = S_IFCHR;

  if (!input_available_event)
    {
      to_close = true;
      if (!fch_open_handles (false))
	goto errout;
    }
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!get_object_attribute (input_available_event, &uid, &gid, &orig_mode)
      && !create_object_sd_from_attribute (uid, gid, S_IFCHR | mode, sd))
    ret = fch_set_sd (sd, false);
errout:
  if (to_close)
    fch_close_handles ();
  return ret;
}

int __reg2
fhandler_pty_slave::fchown (uid_t uid, gid_t gid)
{
  int ret = -1;
  bool to_close = false;
  security_descriptor sd;
  uid_t o_uid;
  gid_t o_gid;
  mode_t mode = S_IFCHR;

  if (uid == ILLEGAL_UID && gid == ILLEGAL_GID)
    return 0;
  if (!input_available_event)
    {
      to_close = true;
      if (!fch_open_handles (true))
	goto errout;
    }
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!get_object_attribute (input_available_event, &o_uid, &o_gid, &mode))
    {
      if (uid == ILLEGAL_UID)
	uid = o_uid;
      if (gid == ILLEGAL_GID)
	gid = o_gid;
      if (uid == o_uid && gid == o_gid)
	ret = 0;
      else if (!create_object_sd_from_attribute (uid, gid, mode, sd))
	ret = fch_set_sd (sd, true);
    }
errout:
  if (to_close)
    fch_close_handles ();
  return ret;
}

/*******************************************************
 fhandler_pty_master
*/
fhandler_pty_master::fhandler_pty_master (int unit)
  : fhandler_pty_common (), pktmode (0), master_ctl (NULL),
    master_thread (NULL), from_master (NULL), to_master (NULL),
    echo_r (NULL), echo_w (NULL), dwProcessId (0),
    io_handle_cyg (NULL), to_master_cyg (NULL), master_fwd_thread (NULL)
{
  if (unit >= 0)
    dev ().parse (DEV_PTYM_MAJOR, unit);
  else if (!setup ())
    {
      dev ().parse (FH_ERROR);
      return;
    }
  set_name ("/dev/ptmx");
}

int
fhandler_pty_master::open (int flags, mode_t)
{
  set_open_status ();
  dwProcessId = GetCurrentProcessId ();
  return 1;
}

void
fhandler_pty_master::open_setup (int flags)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  char buf[sizeof ("opened pty master for ptyNNNNNNNNNNN")];
  __small_sprintf (buf, "opened pty master for pty%d", get_minor ());
  report_tty_counts (this, buf, "");
  fhandler_base::open_setup (flags);
}

off_t
fhandler_pty_common::lseek (off_t, int)
{
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_pty_common::close ()
{
  termios_printf ("pty%d <%p,%p> closing", get_minor (), get_handle (), get_output_handle ());
  if (!ForceCloseHandle (input_mutex))
    termios_printf ("CloseHandle (input_mutex<%p>), %E", input_mutex);
  if (!ForceCloseHandle1 (get_handle (), from_pty))
    termios_printf ("CloseHandle (get_handle ()<%p>), %E", get_handle ());
  if (!ForceCloseHandle1 (get_output_handle (), to_pty))
    termios_printf ("CloseHandle (get_output_handle ()<%p>), %E", get_output_handle ());

  return 0;
}

void
fhandler_pty_master::cleanup ()
{
  report_tty_counts (this, "closing master", "");
  if (archetype)
    from_master = to_master = to_master_cyg = NULL;
  fhandler_base::cleanup ();
}

int
fhandler_pty_master::close ()
{
  OBJECT_BASIC_INFORMATION obi;
  NTSTATUS status;

  termios_printf ("closing from_master(%p)/to_master(%p)/to_master_cyg(%p) since we own them(%u)",
		  from_master, to_master, to_master_cyg, dwProcessId);
  if (cygwin_finished_initializing)
    {
      if (master_ctl && get_ttyp ()->master_pid == myself->pid)
	{
	  char buf[MAX_PATH];
	  pipe_request req = { (DWORD) -1 };
	  pipe_reply repl;
	  DWORD len;

	  __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
			   &cygheap->installation_key, get_minor ());
	  acquire_output_mutex (INFINITE);
	  if (master_ctl)
	    {
	      CallNamedPipe (buf, &req, sizeof req, &repl, sizeof repl, &len,
			     500);
	      CloseHandle (master_ctl);
	      master_thread->detach ();
	      get_ttyp ()->set_master_ctl_closed ();
	      master_ctl = NULL;
	    }
	  release_output_mutex ();
	  master_fwd_thread->terminate_thread ();
	}
    }

  /* Check if the last master handle has been closed.  If so, set
     input_available_event to wake up potentially waiting slaves. */
  acquire_output_mutex (INFINITE);
  status = NtQueryObject (get_output_handle (), ObjectBasicInformation,
			  &obi, sizeof obi, NULL);
  fhandler_pty_common::close ();
  release_output_mutex ();
  if (!ForceCloseHandle (output_mutex))
    termios_printf ("CloseHandle (output_mutex<%p>), %E", output_mutex);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryObject: %y", status);
  else if (obi.HandleCount == 1)
    {
      termios_printf("Closing last master of pty%d", get_minor ());
      if (get_ttyp ()->getsid () > 0)
	kill (get_ttyp ()->getsid (), SIGHUP);
      SetEvent (input_available_event);
    }

  if (!ForceCloseHandle (from_master))
    termios_printf ("error closing from_master %p, %E", from_master);
  if (!ForceCloseHandle (to_master))
    termios_printf ("error closing to_master %p, %E", to_master);
  from_master = to_master = NULL;
  if (!ForceCloseHandle (get_io_handle_cyg ()))
    termios_printf ("error closing io_handle_cyg %p, %E", get_io_handle_cyg ());
  if (!ForceCloseHandle (to_master_cyg))
    termios_printf ("error closing to_master_cyg %p, %E", to_master_cyg);
  get_io_handle_cyg () = to_master_cyg = NULL;
  ForceCloseHandle (echo_r);
  ForceCloseHandle (echo_w);
  echo_r = echo_w = NULL;

  if (have_execed || get_ttyp ()->master_pid != myself->pid)
    termios_printf ("not clearing: %d, master_pid %d", have_execed, get_ttyp ()->master_pid);
  if (!ForceCloseHandle (input_available_event))
    termios_printf ("CloseHandle (input_available_event<%p>), %E", input_available_event);

  return 0;
}

ssize_t __stdcall
fhandler_pty_master::write (const void *ptr, size_t len)
{
  ssize_t ret;
  char *p = (char *) ptr;
  termios ti = tc ()->ti;

  bg_check_types bg = bg_check (SIGTTOU);
  if (bg <= bg_eof)
    return (ssize_t) bg;

  push_process_state process_state (PID_TTYOU);
  line_edit_status status = line_edit (p++, len, ti, &ret);
  if (status > line_edit_signalled && status != line_edit_pipe_full)
    ret = -1;
  return ret;
}

void __reg3
fhandler_pty_master::read (void *ptr, size_t& len)
{
  bg_check_types bg = bg_check (SIGTTIN);
  if (bg <= bg_eof)
    {
      len = (size_t) bg;
      return;
    }
  push_process_state process_state (PID_TTYIN);
  len = (size_t) process_slave_output ((char *) ptr, len, pktmode);
}

int
fhandler_pty_master::tcgetattr (struct termios *t)
{
  *t = cygwin_shared->tty[get_minor ()]->ti;
  return 0;
}

int
fhandler_pty_master::tcsetattr (int, const struct termios *t)
{
  cygwin_shared->tty[get_minor ()]->ti = *t;
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
  int res = fhandler_termios::ioctl (cmd, arg);
  if (res <= 0)
    return res;

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
	  get_ttyp ()->kill_pgrp (SIGWINCH);
	}
      break;
    case TIOCGPGRP:
      *((pid_t *) arg) = this->tcgetpgrp ();
      break;
    case TIOCSPGRP:
      return this->tcsetpgrp ((pid_t) (intptr_t) arg);
    case FIONREAD:
      {
	DWORD n;
	if (!::bytes_available (n, get_io_handle_cyg ()))
	  {
	    set_errno (EINVAL);
	    return -1;
	  }
	*(int *) arg = (int) n;
      }
      break;
    default:
      return fhandler_base::ioctl (cmd, arg);
    }
  return 0;
}

int
fhandler_pty_master::ptsname_r (char *buf, size_t buflen)
{
  char tmpbuf[TTY_NAME_MAX];

  __ptsname (tmpbuf, get_minor ());
  if (buflen <= strlen (tmpbuf))
    {
      set_errno (ERANGE);
      return ERANGE;
    }
  strcpy (buf, tmpbuf);
  return 0;
}

void
fhandler_pty_common::set_close_on_exec (bool val)
{
  // Cygwin processes will handle this specially on exec.
  close_on_exec (val);
}

void
fhandler_pty_slave::fixup_after_fork (HANDLE parent)
{
  // fork_fixup (parent, inuse, "inuse");
  // fhandler_pty_common::fixup_after_fork (parent);
  report_tty_counts (this, "inherited", "");
}

void
fhandler_pty_slave::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (NULL);
}

/* This thread function handles the master control pipe.  It waits for a
   client to connect.  Then it checks if the client process has permissions
   to access the tty handles.  If so, it opens the client process and
   duplicates the handles into that process.  If that fails, it sends a reply
   with at least one handle set to NULL and an error code.  Last but not
   least, the client is disconnected and the thread waits for the next client.

   A special case is when the master side of the tty is about to be closed.
   The client side is the fhandler_pty_master::close function and it sends
   a PID -1 in that case.  A check is performed that the request to leave
   really comes from the master process itself.

   Since there's always only one pipe instance, there's a chance that clients
   have to wait to connect to the master control pipe.  Therefore the client
   calls to CallNamedPipe should have a big enough timeout value.  For now this
   is 500ms.  Hope that's enough. */

DWORD
fhandler_pty_master::pty_master_thread ()
{
  bool exit = false;
  GENERIC_MAPPING map = { EVENT_QUERY_STATE, EVENT_MODIFY_STATE, 0,
			  EVENT_QUERY_STATE | EVENT_MODIFY_STATE };
  pipe_request req;
  DWORD len;
  security_descriptor sd;
  HANDLE token;
  PRIVILEGE_SET ps;
  DWORD pid;
  NTSTATUS status;

  termios_printf ("Entered");
  while (!exit && (ConnectNamedPipe (master_ctl, NULL)
		   || GetLastError () == ERROR_PIPE_CONNECTED))
    {
      pipe_reply repl = { NULL, NULL, 0 };
      bool deimp = false;
      NTSTATUS allow = STATUS_ACCESS_DENIED;
      ACCESS_MASK access = EVENT_MODIFY_STATE;
      HANDLE client = NULL;

      if (!ReadFile (master_ctl, &req, sizeof req, &len, NULL))
	{
	  termios_printf ("ReadFile, %E");
	  goto reply;
	}
      if (!GetNamedPipeClientProcessId (master_ctl, &pid))
	pid = req.pid;
      if (get_object_sd (input_available_event, sd))
	{
	  termios_printf ("get_object_sd, %E");
	  goto reply;
	}
      cygheap->user.deimpersonate ();
      deimp = true;
      if (!ImpersonateNamedPipeClient (master_ctl))
	{
	  termios_printf ("ImpersonateNamedPipeClient, %E");
	  goto reply;
	}
      status = NtOpenThreadToken (GetCurrentThread (), TOKEN_QUERY, TRUE,
				  &token);
      if (!NT_SUCCESS (status))
	{
	  termios_printf ("NtOpenThreadToken, %y", status);
	  SetLastError (RtlNtStatusToDosError (status));
	  goto reply;
	}
      len = sizeof ps;
      status = NtAccessCheck (sd, token, access, &map, &ps, &len, &access,
			      &allow);
      NtClose (token);
      if (!NT_SUCCESS (status))
	{
	  termios_printf ("NtAccessCheck, %y", status);
	  SetLastError (RtlNtStatusToDosError (status));
	  goto reply;
	}
      if (!RevertToSelf ())
	{
	  termios_printf ("RevertToSelf, %E");
	  goto reply;
	}
      if (req.pid == (DWORD) -1)	/* Request to finish thread. */
	{
	  /* Check if the requesting process is the master process itself. */
	  if (pid == GetCurrentProcessId ())
	    exit = true;
	  goto reply;
	}
      if (NT_SUCCESS (allow))
	{
	  client = OpenProcess (PROCESS_DUP_HANDLE, FALSE, pid);
	  if (!client)
	    {
	      termios_printf ("OpenProcess, %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), from_master,
			       client, &repl.from_master,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (from_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), to_master,
				client, &repl.to_master,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master), %E");
	      goto reply;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), to_master_cyg,
				client, &repl.to_master_cyg,
				0, TRUE, DUPLICATE_SAME_ACCESS))
	    {
	      termios_printf ("DuplicateHandle (to_master_cyg), %E");
	      goto reply;
	    }
	}
reply:
      repl.error = GetLastError ();
      if (client)
	CloseHandle (client);
      if (deimp)
	cygheap->user.reimpersonate ();
      sd.free ();
      termios_printf ("Reply: from %p, to %p, error %u",
		      repl.from_master, repl.to_master, repl.error );
      if (!WriteFile (master_ctl, &repl, sizeof repl, &len, NULL))
	termios_printf ("WriteFile, %E");
      if (!DisconnectNamedPipe (master_ctl))
	termios_printf ("DisconnectNamedPipe, %E");
    }
  termios_printf ("Leaving");
  return 0;
}

static DWORD WINAPI
pty_master_thread (VOID *arg)
{
  return ((fhandler_pty_master *) arg)->pty_master_thread ();
}

DWORD
fhandler_pty_master::pty_master_fwd_thread ()
{
  DWORD rlen;
  char outbuf[OUT_BUFFER_SIZE];

  termios_printf("Started.");
  for (;;)
    {
      if (!ReadFile (get_io_handle (), outbuf, sizeof outbuf, &rlen, NULL))
	{
	  termios_printf ("ReadFile for forwarding failed, %E");
	  break;
	}
      ssize_t wlen = rlen;
      while (rlen>0)
	{
	  if (!process_opost_output (to_master_cyg, outbuf, wlen, false))
	    {
	      termios_printf ("WriteFile for forwarding failed, %E");
	      break;
	    }
	  rlen -= wlen;
	}
    }
  return 0;
}

static DWORD WINAPI
pty_master_fwd_thread (VOID *arg)
{
  return ((fhandler_pty_master *) arg)->pty_master_fwd_thread ();
}

bool
fhandler_pty_master::setup ()
{
  int res;
  security_descriptor sd;
  SECURITY_ATTRIBUTES sa = { sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };

  /* Find an unallocated pty to use. */
  int unit = cygwin_shared->tty.allocate (from_master, get_output_handle ());
  if (unit < 0)
    return false;

  ProtectHandle1 (get_output_handle (), to_pty);

  tty& t = *cygwin_shared->tty[unit];
  _tc = (tty_min *) &t;

  tcinit (true);		/* Set termios information.  Force initialization. */

  const char *errstr = NULL;
  DWORD pipe_mode = PIPE_NOWAIT;

  if (!SetNamedPipeHandleState (get_output_handle (), &pipe_mode, NULL, NULL))
    termios_printf ("can't set output_handle(%p) to non-blocking mode",
		    get_output_handle ());

  char pipename[sizeof("ptyNNNN-to-master-cyg")];
  __small_sprintf (pipename, "pty%d-to-master", unit);
  res = fhandler_pipe::create (&sec_none, &get_io_handle (), &to_master,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe";
      goto err;
    }

  __small_sprintf (pipename, "pty%d-to-master-cyg", unit);
  res = fhandler_pipe::create (&sec_none, &get_io_handle_cyg (), &to_master_cyg,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "output pipe for cygwin";
      goto err;
    }

  ProtectHandle1 (get_io_handle (), from_pty);

  __small_sprintf (pipename, "pty%d-echoloop", unit);
  res = fhandler_pipe::create (&sec_none, &echo_r, &echo_w,
			       fhandler_pty_common::pipesize, pipename, 0);
  if (res)
    {
      errstr = "echo pipe";
      goto err;
    }

  /* Create security attribute.  Default permissions are 0620. */
  sd.malloc (sizeof (SECURITY_DESCRIPTOR));
  RtlCreateSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION);
  if (!create_object_sd_from_attribute (myself->uid, myself->gid,
					S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP,
					sd))
    sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) sd;

  /* Carefully check that the input_available_event didn't already exist.
     This is a measure to make sure that the event security descriptor
     isn't occupied by a malicious process.  We must make sure that the
     event's security descriptor is what we expect it to be. */
  if (!(input_available_event = t.get_event (errstr = INPUT_AVAILABLE_EVENT,
					     &sa, TRUE))
      || GetLastError () == ERROR_ALREADY_EXISTS)
    goto err;

  char buf[MAX_PATH];
  errstr = shared_name (buf, OUTPUT_MUTEX, unit);
  if (!(output_mutex = CreateMutex (&sa, FALSE, buf)))
    goto err;

  errstr = shared_name (buf, INPUT_MUTEX, unit);
  if (!(input_mutex = CreateMutex (&sa, FALSE, buf)))
    goto err;

  /* Create master control pipe which allows the master to duplicate
     the pty pipe handles to processes which deserve it. */
  __small_sprintf (buf, "\\\\.\\pipe\\cygwin-%S-pty%d-master-ctl",
		   &cygheap->installation_key, unit);
  master_ctl = CreateNamedPipe (buf, PIPE_ACCESS_DUPLEX
				     | FILE_FLAG_FIRST_PIPE_INSTANCE,
				PIPE_WAIT | PIPE_TYPE_MESSAGE
				| PIPE_READMODE_MESSAGE
				| PIPE_REJECT_REMOTE_CLIENTS,
				1, 4096, 4096, 0, &sec_all_nih);
  if (master_ctl == INVALID_HANDLE_VALUE)
    {
      errstr = "pty master control pipe";
      goto err;
    }
  master_thread = new cygthread (::pty_master_thread, this, "ptym");
  if (!master_thread)
    {
      errstr = "pty master control thread";
      goto err;
    }
  master_fwd_thread = new cygthread (::pty_master_fwd_thread, this, "ptymf");
  if (!master_fwd_thread)
    {
      errstr = "pty master forwarding thread";
      goto err;
    }

  t.set_from_master (from_master);
  t.set_to_master (to_master);
  t.set_to_master_cyg (to_master_cyg);
  t.winsize.ws_col = 80;
  t.winsize.ws_row = 25;
  t.master_pid = myself->pid;

  dev ().parse (DEV_PTYM_MAJOR, unit);

  termios_printf ("this %p, pty%d opened - from_pty <%p,%p>, to_pty %p",
	this, unit, get_io_handle (), get_io_handle_cyg (),
	get_output_handle ());
  return true;

err:
  __seterrno ();
  close_maybe (get_io_handle ());
  close_maybe (get_io_handle_cyg ());
  close_maybe (get_output_handle ());
  close_maybe (input_available_event);
  close_maybe (output_mutex);
  close_maybe (input_mutex);
  close_maybe (from_master);
  close_maybe (to_master);
  close_maybe (to_master_cyg);
  close_maybe (echo_r);
  close_maybe (echo_w);
  close_maybe (master_ctl);
  termios_printf ("pty%d open failed - failed to create %s", unit, errstr);
  return false;
}

void
fhandler_pty_master::fixup_after_fork (HANDLE parent)
{
  DWORD wpid = GetCurrentProcessId ();
  fhandler_pty_master *arch = (fhandler_pty_master *) archetype;
  if (arch->dwProcessId != wpid)
    {
      tty& t = *get_ttyp ();
      if (myself->pid == t.master_pid)
	{
	  t.set_from_master (arch->from_master);
	  t.set_to_master (arch->to_master);
	  t.set_to_master_cyg (arch->to_master_cyg);
	}
      arch->dwProcessId = wpid;
    }
  from_master = arch->from_master;
  to_master = arch->to_master;
  to_master_cyg = arch->to_master_cyg;
  report_tty_counts (this, "inherited master", "");
}

void
fhandler_pty_master::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (spawn_info->parent);
  else
    from_master = to_master = to_master_cyg = NULL;
}

BOOL
fhandler_pty_common::process_opost_output (HANDLE h, const void *ptr, ssize_t& len, bool is_echo)
{
  ssize_t towrite = len;
  BOOL res = TRUE;
  while (towrite)
    {
      if (!is_echo)
	{
	  if (tc ()->output_stopped && is_nonblocking ())
	    {
	      if (towrite < len)
		break;
	      else
		{
		  set_errno(EAGAIN);
		  len = -1;
		  return TRUE;
		}
	    }
	  while (tc ()->output_stopped)
	    cygwait (10);
	}

      if (!(get_ttyp ()->ti.c_oflag & OPOST))	// raw output mode
	{
	  DWORD n = MIN (OUT_BUFFER_SIZE, towrite);
	  res = WriteFile (h, ptr, n, &n, NULL);
	  if (!res)
	    break;
	  ptr = (char *) ptr + n;
	  towrite -= n;
	}
      else					// post-process output
	{
	  char outbuf[OUT_BUFFER_SIZE + 1];
	  char *buf = (char *)ptr;
	  DWORD n = 0;
	  ssize_t rc = 0;
	  acquire_output_mutex (INFINITE);
	  while (n < OUT_BUFFER_SIZE && rc < towrite)
	    {
	      switch (buf[rc])
		{
		case '\r':
		  if ((get_ttyp ()->ti.c_oflag & ONOCR)
		      && get_ttyp ()->column == 0)
		    {
		      rc++;
		      continue;
		    }
		  if (get_ttyp ()->ti.c_oflag & OCRNL)
		    {
		      outbuf[n++] = '\n';
		      rc++;
		    }
		  else
		    {
		      outbuf[n++] = buf[rc++];
		      get_ttyp ()->column = 0;
		    }
		  break;
		case '\n':
		  if (get_ttyp ()->ti.c_oflag & ONLCR)
		    {
		      outbuf[n++] = '\r';
		      get_ttyp ()->column = 0;
		    }
		  if (get_ttyp ()->ti.c_oflag & ONLRET)
		    get_ttyp ()->column = 0;
		  outbuf[n++] = buf[rc++];
		  break;
		default:
		  outbuf[n++] = buf[rc++];
		  get_ttyp ()->column++;
		  break;
		}
	    }
	  release_output_mutex ();
	  res = WriteFile (h, outbuf, n, &n, NULL);
	  if (!res)
	    break;
	  ptr = (char *) ptr + rc;
	  towrite -= rc;
	}
    }
  len -= towrite;
  return res;
}
