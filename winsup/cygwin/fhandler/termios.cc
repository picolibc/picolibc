/* fhandler_termios.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <ctype.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "sigproc.h"
#include "pinfo.h"
#include "tty.h"
#include "cygtls.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include "ntdll.h"

/* Wait time for some treminal mutexes. This is set to 0 when the
   process calls CreateProcess() with DEBUG_PROCESS flag, because
   the debuggie may be suspended while it grabs the mutex. Without
   this, GDB may cause deadlock in console or pty I/O. */
DWORD NO_COPY mutex_timeout = INFINITE;

/* Common functions shared by tty/console */

void
fhandler_termios::tcinit (bool is_pty_master)
{
  /* Initial termios values */

  if (is_pty_master || !tc ()->initialized ())
    {
      tc ()->ti.c_iflag = BRKINT | ICRNL | IXON | IUTF8;
      tc ()->ti.c_oflag = OPOST | ONLCR;
      tc ()->ti.c_cflag = B38400 | CS8 | CREAD;
      tc ()->ti.c_lflag = ISIG | ICANON | ECHO | IEXTEN
	| ECHOE | ECHOK | ECHOCTL | ECHOKE;

      tc ()->ti.c_cc[VDISCARD]	= CFLUSH;
      tc ()->ti.c_cc[VEOL]	= CEOL;
      tc ()->ti.c_cc[VEOL2]	= CEOL2;
      tc ()->ti.c_cc[VEOF]	= CEOF;
      tc ()->ti.c_cc[VERASE]	= CERASE;
      tc ()->ti.c_cc[VINTR]	= CINTR;
      tc ()->ti.c_cc[VKILL]	= CKILL;
      tc ()->ti.c_cc[VLNEXT]	= CLNEXT;
      tc ()->ti.c_cc[VMIN]	= CMIN;
      tc ()->ti.c_cc[VQUIT]	= CQUIT;
      tc ()->ti.c_cc[VREPRINT]	= CRPRNT;
      tc ()->ti.c_cc[VSTART]	= CSTART;
      tc ()->ti.c_cc[VSTOP]	= CSTOP;
      tc ()->ti.c_cc[VSUSP]	= CSUSP;
      tc ()->ti.c_cc[VSWTC]	= CSWTCH;
      tc ()->ti.c_cc[VTIME]	= CTIME;
      tc ()->ti.c_cc[VWERASE]	= CWERASE;

      tc ()->ti.c_ispeed = tc ()->ti.c_ospeed = B38400;
      tc ()->pgid = is_pty_master ? 0 : myself->pgid;
      tc ()->initialized (true);
    }
}

int
fhandler_termios::tcsetpgrp (const pid_t pgid)
{
  termios_printf ("%s, pgid %d, sid %d, tsid %d", tc ()->ttyname (), pgid,
		    myself->sid, tc ()->getsid ());
  if (myself->sid != tc ()->getsid ())
    {
      set_errno (EPERM);
      return -1;
    }
  else if (pgid < 0)
    {
      set_errno (EINVAL);
      return -1;
    }
  int res;
  while (1)
    {
      res = bg_check (-SIGTTOU);

      switch (res)
	{
	case bg_ok:
	  tc ()->setpgid (pgid);
	  if (tc ()->is_console && (strace.active () || !being_debugged ()))
	    tc ()->kill_pgrp (__SIGSETPGRP);
	  res = 0;
	  break;
	case bg_signalled:
	  if (_my_tls.call_signal_handler ())
	    continue;
	  set_errno (EINTR);
	  fallthrough;
	default:
	  res = -1;
	  break;
	}
      break;
    }
  return res;
}

int
fhandler_termios::tcgetpgrp ()
{
  if (CTTY_IS_VALID (myself->ctty) && myself->ctty == tc ()->ntty)
    return tc ()->pgid;
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_pty_master::tcgetpgrp ()
{
  return tc ()->pgid;
}

static inline bool
is_flush_sig (int sig)
{
  return sig == SIGINT || sig == SIGQUIT || sig == SIGTSTP;
}

void
tty_min::kill_pgrp (int sig, pid_t target_pgid)
{
  target_pgid = target_pgid ?: pgid;
  bool killself = false;
  if (is_flush_sig (sig) && cygheap->ctty)
    cygheap->ctty->sigflush ();
  winpids pids ((DWORD) PID_MAP_RW);
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;
  if (sig > 0 && sig < _NSIG)
    last_sig = sig;

  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      if (!p || !p->exists () || p->ctty != ntty || p->pgid != target_pgid)
	continue;
      if (p->process_state & PID_NOTCYGWIN)
	continue; /* Do not send signal to non-cygwin process to prevent
		     cmd.exe from crash. */
      if (p == myself)
	killself = sig != __SIGSETPGRP && !exit_state;
      else
	sig_send (p, si);
    }
  if (killself)
    sig_send (myself, si);
}

int
tty_min::is_orphaned_process_group (int pgid)
{
  /* An orphaned process group is a process group in which the parent
     of every member is either itself a member of the group or is not
     a member of the group's session. */
  termios_printf ("checking pgid %d, my sid %d, my parent %d", pgid, myself->sid, myself->ppid);
  winpids pids ((DWORD) 0);
  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      termios_printf ("checking pid %d - has pgid %d\n", p->pid, p->pgid);
      if (!p || !p->exists () || p->pgid != pgid)
	continue;
      pinfo ppid (p->ppid);
      if (!ppid)
	continue;
      termios_printf ("ppid->pgid %d, ppid->sid %d", ppid->pgid, ppid->sid);
      if (ppid->pgid != pgid && ppid->sid == myself->sid)
	return 0;
    }
  return 1;
}

/*
  bg_check: check that this process is either in the foreground process group,
  or if the terminal operation is allowed for processes which are in a
  background process group.

  If the operation is not permitted by the terminal configuration for processes
  which are a member of a background process group, return an error or raise a
  signal as appropriate.

  This handles the following terminal operations:

  write:                             sig = SIGTTOU
  read:                              sig = SIGTTIN
  change terminal settings:          sig = -SIGTTOU
  (tcsetattr, tcsetpgrp, etc.)
  peek (poll, select):               sig = SIGTTIN, dontsignal = TRUE
*/
bg_check_types
fhandler_termios::bg_check (int sig, bool dontsignal)
{
  /* Ignore errors:
     - this process isn't in a process group
     - tty is invalid

     Everything is ok if:
     - this process is in the foreground process group, or
     - this tty is not the controlling tty for this process (???), or
     - writing, when TOSTOP TTY mode is not set on this tty
  */
  if (!myself->pgid || !tc () || tc ()->getpgid () == myself->pgid ||
	myself->ctty != tc ()->ntty ||
	((sig == SIGTTOU) && !(tc ()->ti.c_lflag & TOSTOP)))
    return bg_ok;

  /* sig -SIGTTOU is used to indicate a change to terminal settings, where
     TOSTOP TTY mode isn't considered when determining if we need to send a
     signal. */
  if (sig < 0)
    sig = -sig;

  termios_printf ("%s, bg I/O pgid %d, tpgid %d, myctty %s", tc ()->ttyname (),
		  myself->pgid, tc ()->getpgid (), myctty ());

  if (tc ()->getsid () == 0)
    {
      /* The pty has been closed by the master.  Return an EOF
	 indication.  FIXME: There is nothing to stop somebody
	 from reallocating this pty.  I think this is the case
	 which is handled by unlockpt on a Unix system.  */
      termios_printf ("closed by master");
      return bg_eof;
    }

  threadlist_t *tl_entry;
  tl_entry = cygheap->find_tls (_main_tls);
  int sigs_ignored =
    ((void *) global_sigs[sig].sa_handler == (void *) SIG_IGN) ||
    (_main_tls->sigmask & SIGTOMASK (sig));
  cygheap->unlock_tls (tl_entry);

  /* If the process is blocking or ignoring SIGTT*, then signals are not sent
     and background IO is allowed */
  if (sigs_ignored)
    return bg_ok;   /* Just allow the IO */
  /* If the process group of the process is orphaned, return EIO */
  else if (tc ()->is_orphaned_process_group (myself->pgid))
    {
      termios_printf ("process group is orphaned");
      set_errno (EIO);   /* This is an IO error */
      return bg_error;
    }
  /* Otherwise, if signalling is desired, the signal is sent to all processes in
     the process group */
  else if (!dontsignal)
    {
      /* Don't raise a SIGTT* signal if we have already been
	 interrupted by another signal. */
      if (cygwait ((DWORD) 0) != WAIT_SIGNALED)
	{
	  siginfo_t si = {0};
	  si.si_signo = sig;
	  si.si_code = SI_KERNEL;
	  kill_pgrp (myself->pgid, si);
	}
      return bg_signalled;
    }
  return bg_ok;
}

#define set_input_done(x) input_done = input_done || (x)

int
fhandler_termios::eat_readahead (int n)
{
  int oralen = ralen ();
  if (n < 0)
    n = ralen () - raixget ();
  if (n > 0 && ralen () > raixget ())
    {
      if ((int) (ralen () -= n) < (int) raixget ())
	ralen () = raixget ();
      /* If IUTF8 is set, the terminal is in UTF-8 mode.  If so, we erase
	 a complete UTF-8 multibyte sequence on VERASE/VWERASE.  Otherwise,
	 if we only erase a single byte, invalid unicode chars are left in
	 the input. */
      if (tc ()->ti.c_iflag & IUTF8)
	while (ralen () > raixget () &&
	       ((unsigned char) rabuf ()[ralen ()] & 0xc0) == 0x80)
	  --ralen ();
    }
  oralen = oralen - ralen ();
  if (raixget () >= ralen ())
    raixget () = raixput () = ralen () = 0;
  else if (raixput () > ralen ())
    raixput () = ralen ();

  return oralen;
}

inline void
fhandler_termios::echo_erase (int force)
{
  if (force || tc ()->ti.c_lflag & ECHO)
    doecho ("\b \b", 3);
}

/* The basic policy is as follows:
   - The signal generated by key press will be sent only to cygwin process.
   - For non-cygwin process, CTRL_C_EVENT will be sent on Ctrl-C. */
/* The inferior of GDB is an exception. GDB does not support to hook signal
   even if the inferior is a cygwin app. As a result, inferior cannot be
   continued after interruption by Ctrl-C if SIGINT was sent. Therefore,
   CTRL_C_EVENT rather than SIGINT is sent to the inferior of GDB. */
fhandler_termios::process_sig_state
fhandler_termios::process_sigs (char c, tty* ttyp, fhandler_termios *fh)
{
  termios &ti = ttyp->ti;
  pid_t pgid = ttyp->pgid;

  /* The name *_nat stands for 'native' which means non-cygwin apps. */
  bool ctrl_c_event_sent = false;
  bool need_discard_input = false;
  bool pg_with_nat = false; /* The process group has non-cygwin processes. */
  bool need_send_sig = false; /* There is process which need the signal. */
  bool nat_shell = false; /* The shell seems to be a non-cygwin process. */
  bool cyg_reader = false; /* Cygwin process is reading the tty. */
  bool with_debugger = false; /* GDB is debugging cygwin app. */
  bool with_debugger_nat = false; /* GDB is debugging non-cygwin app. */

  winpids pids ((DWORD) 0);
  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      /* PID_NOTCYGWIN: check this for non-cygwin process.
	 exec_dwProcessId == dwProcessId:
		     check this for GDB with non-cygwin inferior in pty
		     without pcon enabled. In this case, the inferior is not
		     cygwin process list. This condition is set true as
		     a marker for GDB with non-cygwin inferior in pty code.
	 !PID_CYGPARENT: check this for GDB with cygwin inferior or
			 cygwin apps started from non-cygwin shell. */
      if (c == '\003' && p && p->ctty == ttyp->ntty && p->pgid == pgid
	  && ((p->process_state & PID_NOTCYGWIN)
	      || ((p->exec_dwProcessId == p->dwProcessId)
		  && ttyp->pty_input_state_eq (tty::to_nat))
	      || !(p->process_state & PID_CYGPARENT)))
	{
	  /* Ctrl-C event will be sent only to the processes attaching
	     to the same console. Therefore, attach to the console to
	     which the target process is attaching before sending the
	     CTRL_C_EVENT. After sending the event, reattach to the
	     console to which the process was previously attached.  */
	  DWORD resume_pid = 0;
	  if (fh && !fh->is_console ())
	    resume_pid =
	      fhandler_pty_common::attach_console_temporarily (p->dwProcessId);
	  if (fh && p == myself && being_debugged ())
	    { /* Avoid deadlock in gdb on console. */
	      fh->tcflush(TCIFLUSH);
	      fh->release_input_mutex_if_necessary ();
	    }
	  /* CTRL_C_EVENT does not work for the process started with
	     CREATE_NEW_PROCESS_GROUP flag, so send CTRL_BREAK_EVENT
	     instead. */
	  if (p->process_state & PID_NEW_PG)
	    GenerateConsoleCtrlEvent (CTRL_BREAK_EVENT, p->dwProcessId);
	  else if ((!fh || fh->need_send_ctrl_c_event ()
		    || p->exec_dwProcessId == p->dwProcessId)
		   && !ctrl_c_event_sent)
	    {
	      GenerateConsoleCtrlEvent (CTRL_C_EVENT, 0);
	      ctrl_c_event_sent = true;
	    }
	  if (fh && !fh->is_console ())
	    {
	      /* If a process on pseudo console is killed by Ctrl-C,
		 this process may take over the ownership of the
		 pseudo console because this process attached to it
		 before sending CTRL_C_EVENT. In this case, closing
		 pseudo console is necessary. */
	      fhandler_pty_slave::release_ownership_of_nat_pipe (ttyp, fh);
	      fhandler_pty_common::resume_from_temporarily_attach (resume_pid);
	    }
	  need_discard_input = true;
	}
      if (p && p->ctty == ttyp->ntty && p->pgid == pgid)
	{
	  if (p->process_state & PID_NOTCYGWIN)
	    pg_with_nat = true; /* The process group has non-cygwin process */
	  if (!(p->process_state & PID_NOTCYGWIN))
	    need_send_sig = true; /* Process which needs signal exists */
	  if (!p->cygstarted)
	    nat_shell = true; /* The shell seems to a non-cygwin shell */
	  if (p->process_state & PID_TTYIN)
	    cyg_reader = true; /* Theh process is reading the tty */
	  if (!p->cygstarted && !(p->process_state & PID_NOTCYGWIN)
	      && (p->process_state & PID_DEBUGGED))
	    with_debugger = true; /* inferior is cygwin app */
	  if (!(p->process_state & PID_NOTCYGWIN)
	      && (p->exec_dwProcessId == p->dwProcessId) /* Check marker */
	      && ttyp->pty_input_state_eq (tty::to_nat)
	      && p->pid == pgid)
	    with_debugger_nat = true; /* inferior is non-cygwin app */
	}
    }
  if ((with_debugger || with_debugger_nat) && need_discard_input)
    {
      if (!(ti.c_lflag & NOFLSH) && fh)
	{
	  fh->eat_readahead (-1);
	  fh->discard_input ();
	}
      ti.c_lflag &= ~FLUSHO;
      return done_with_debugger;
    }
  if (with_debugger_nat)
    return not_signalled; /* Do not process slgnal keys further.
			     The non-cygwin inferior of GDB cannot receive
			     the signals. */
  /* Send SIGQUIT to non-cygwin process. Non-cygwin app will not be alerted
     by kill_pgrp(), however, QUIT key should quit the non-cygwin app
     if it is started along with cygwin process from cygwin shell. */
  if ((ti.c_lflag & ISIG) && CCEQ (ti.c_cc[VQUIT], c)
      && pg_with_nat && need_send_sig && !nat_shell)
    {
      for (unsigned i = 0; i < pids.npids; i++)
	{
	  _pinfo *p = pids[i];
	  if (p && p->ctty == ttyp->ntty && p->pgid == pgid
	      && (p->process_state & PID_NOTCYGWIN))
	    sig_send (p, SIGQUIT);
	}
    }
  if ((ti.c_lflag & ISIG) && need_send_sig)
    {
      int sig;
      if (CCEQ (ti.c_cc[VINTR], c))
	sig = SIGINT;
      else if (CCEQ (ti.c_cc[VQUIT], c))
	sig = SIGQUIT;
      else if (pg_with_nat) /* If the process group has a non-cygwin process,
			       it cannot be suspended correctly. Therefore,
			       do not send SIGTSTP. */
	goto not_a_sig;
      else if (CCEQ (ti.c_cc[VSUSP], c))
	sig = SIGTSTP;
      else
	goto not_a_sig;

      termios_printf ("got interrupt %d, sending signal %d", c, sig);
      if (!(ti.c_lflag & NOFLSH) && fh)
	{
	  fh->eat_readahead (-1);
	  fh->discard_input ();
	}
      if (fh)
	fh->release_input_mutex_if_necessary ();
      ttyp->kill_pgrp (sig, pgid);
      if (fh)
	fh->acquire_input_mutex_if_necessary (mutex_timeout);
      ti.c_lflag &= ~FLUSHO;
      return signalled;
    }
not_a_sig:
  if ((ti.c_lflag & ISIG) && need_discard_input)
    {
      if (!(ti.c_lflag & NOFLSH) && fh)
	{
	  fh->eat_readahead (-1);
	  fh->discard_input ();
	}
      ti.c_lflag &= ~FLUSHO;
      return not_signalled_but_done;
    }
  bool to_nat = !cyg_reader && pg_with_nat;
  return to_nat ? not_signalled_with_nat_reader : not_signalled;
}

bool
fhandler_termios::process_stop_start (char c, tty *ttyp)
{
  termios &ti = ttyp->ti;
  if (ti.c_iflag & IXON)
    {
      if (CCEQ (ti.c_cc[VSTOP], c))
	{
	  ttyp->output_stopped = true;
	  return true;
	}
      else if (CCEQ (ti.c_cc[VSTART], c))
	{
restart_output:
	  ttyp->output_stopped = false;
	  return true;
	}
      else if ((ti.c_iflag & IXANY) && ttyp->output_stopped)
	goto restart_output;
    }
  if ((ti.c_lflag & ICANON) && (ti.c_lflag & IEXTEN)
      && CCEQ (ti.c_cc[VDISCARD], c))
    {
      ti.c_lflag ^= FLUSHO;
      return true;
    }
  return false;
}

line_edit_status
fhandler_termios::line_edit (const char *rptr, size_t nread, termios& ti,
			     ssize_t *bytes_read)
{
  line_edit_status ret = line_edit_ok;
  char c;
  int input_done = 0;
  bool sawsig = false;
  int iscanon = ti.c_lflag & ICANON;
  size_t read_cnt = 0;

  while (read_cnt < nread)
    {
      c = *rptr++;
      read_cnt++;

      paranoid_printf ("char %0o", c);

      if (ti.c_iflag & ISTRIP)
	c &= 0x7f;
      bool disable_eof_key = false;
      switch (process_sigs (c, get_ttyp (), this))
	{
	case signalled:
	  sawsig = true;
	  fallthrough;
	case not_signalled_but_done:
	case done_with_debugger:
	  get_ttyp ()->output_stopped = false;
	  continue;
	case not_signalled_with_nat_reader:
	  disable_eof_key = true;
	  break;
	default: /* Not signalled */
	  break;
	}
      if (process_stop_start (c, get_ttyp ()))
	continue;
      /* Check for special chars */
      if (c == '\r')
	{
	  if (ti.c_iflag & IGNCR)
	    continue;
	  if (ti.c_iflag & ICRNL)
	    {
	      c = '\n';
	      set_input_done (iscanon);
	    }
	}
      else if (c == '\n')
	{
	  if (ti.c_iflag & INLCR)
	    c = '\r';
	  else
	    set_input_done (iscanon);
	}

      if (!iscanon)
	/* nothing */;
      else if (CCEQ (ti.c_cc[VERASE], c))
	{
	  if (eat_readahead (1))
	    echo_erase ();
	  continue;
	}
      else if (CCEQ (ti.c_cc[VWERASE], c))
	{
	  int ch;
	  do
	    if (!eat_readahead (1))
	      break;
	    else
	      echo_erase ();
	  while ((ch = peek_readahead (1)) >= 0 && !isspace (ch));
	  continue;
	}
      else if (CCEQ (ti.c_cc[VKILL], c))
	{
	  int nchars = eat_readahead (-1);
	  if (ti.c_lflag & ECHO)
	    while (nchars--)
	      echo_erase (1);
	  continue;
	}
      else if (CCEQ (ti.c_cc[VREPRINT], c))
	{
	  if (ti.c_lflag & ECHO)
	    {
	      doecho ("\n\r", 2);
	      doecho (rabuf (), ralen ());
	    }
	  continue;
	}
      else if (CCEQ (ti.c_cc[VEOF], c) && !disable_eof_key)
	{
	  termios_printf ("EOF");
	  accept_input ();
	  ret = line_edit_input_done;
	  continue;
	}
      else if (CCEQ (ti.c_cc[VEOL], c) ||
	       CCEQ (ti.c_cc[VEOL2], c) ||
	       c == '\n')
	{
	  set_input_done (1);
	  termios_printf ("EOL");
	}

      if (ti.c_iflag & IUCLC && isupper (c))
	c = cyg_tolower (c);

      put_readahead (c);
      if (ti.c_lflag & ECHO)
	doecho (&c, 1);
      /* Write in chunks of 32 bytes to reduce the number of WriteFile calls
	in non-canonical mode. */
      if ((!iscanon && ralen () >= 32) || input_done)
	{
	  int status = accept_input ();
	  if (status != 1)
	    {
	      ret = status ? line_edit_error : line_edit_pipe_full;
	      break;
	    }
	  ret = line_edit_input_done;
	  input_done = 0;
	}
    }

  /* If we didn't write all bytes in non-canonical mode, write them now. */
  if ((input_done || !iscanon) && ralen () > 0 && ret != line_edit_error)
    {
      int status;
      int retry_count = 3;
      while ((status = accept_input ()) != 1 &&
	     ralen () > 0 && --retry_count > 0)
	cygwait ((DWORD) 10);
      if (status != 1)
	ret = status ? line_edit_error : line_edit_pipe_full;
      else
	ret = line_edit_input_done;
    }

  if (bytes_read)
    *bytes_read = read_cnt;

  if (sawsig)
    ret = line_edit_signalled;

  return ret;
}

off_t
fhandler_termios::lseek (off_t, int)
{
  set_errno (ESPIPE);
  return -1;
}

void
fhandler_termios::sigflush ()
{
  /* FIXME: Checking get_ttyp() for NULL is not right since it should not
     be NULL while this is alive.  However, we can conceivably close a
     ctty while exiting and that will zero this. */
  if ((!have_execed || have_execed_cygwin) && tc ()
      && (tc ()->getpgid () == myself->pgid)
      && !(tc ()->ti.c_lflag & NOFLSH))
    tcflush (TCIFLUSH);
}

pid_t
fhandler_termios::tcgetsid ()
{
  if (CTTY_IS_VALID (myself->ctty) && myself->ctty == tc ()->ntty)
    return tc ()->getsid ();
  set_errno (ENOTTY);
  return -1;
}

static bool
is_console_app (const WCHAR *filename)
{
  HANDLE h;
  const int id_offset = 92;
  h = CreateFileW (filename, GENERIC_READ, FILE_SHARE_READ,
		   NULL, OPEN_EXISTING, 0, NULL);
  char buf[1024];
  DWORD n;
  ReadFile (h, buf, sizeof (buf), &n, 0);
  CloseHandle (h);
  char *p = (char *) memmem (buf, n, "PE\0\0", 4);
  if (p && p + id_offset < buf + n)
    return p[id_offset] == '\003'; /* 02: GUI, 03: console */
  else
    {
      wchar_t *e = wcsrchr (filename, L'.');
      if (e && (wcscasecmp (e, L".bat") == 0 || wcscasecmp (e, L".cmd") == 0))
	return true;
    }
  return false;
}

int
fhandler_termios::ioctl (int cmd, void *varg)
{
  if (cmd != TIOCSCTTY)
    return 1;		/* Not handled by this function */

  int arg = (int) (intptr_t) varg;

  if (arg != 0 && arg != 1)
    {
      set_errno (EINVAL);
      return -1;
    }

  termios_printf ("myself->ctty %d, myself->sid %d, myself->pid %d, arg %d, tc()->getsid () %d\n",
		  myself->ctty, myself->sid, myself->pid, arg, tc ()->getsid ());
  if (CTTY_IS_VALID (myself->ctty) || myself->sid != myself->pid
      || (!arg && tc ()->getsid () > 0))
    {
      set_errno (EPERM);
      return -1;
    }

  if (!myself->set_ctty (this, 0))
    {
      set_errno (EPERM);
      return -1;
    }
  return 0;
}

void
fhandler_termios::spawn_worker::setup (bool iscygwin, HANDLE h_stdin,
				       const WCHAR *runpath, bool nopcon,
				       bool reset_sendsig,
				       const WCHAR *envblock)
{
  fhandler_pty_slave *ptys_primary = NULL;
  fhandler_console *cons_native = NULL;

  for (int i = 0; i < 3; i ++)
    {
      const int chk_order[] = {1, 0, 2};
      int fd = chk_order[i];
      fhandler_base *fh = ::cygheap->fdtab[fd];
      if (fh && fh->get_major () == DEV_PTYS_MAJOR && ptys_primary == NULL)
	ptys_primary = (fhandler_pty_slave *) fh;
      else if (fh && fh->get_major () == DEV_CONS_MAJOR
	       && !iscygwin && cons_native == NULL)
	cons_native = (fhandler_console *) fh;
    }
  if (cons_native)
    {
      cons_native->setup_for_non_cygwin_app ();
      /* Console handles will be already closed by close_all_files()
	 when cleaning up, therefore, duplicate them here. */
      cons_native->get_duplicated_handle_set (&cons_handle_set);
      cons_need_cleanup = true;
    }
  if (!iscygwin)
    {
      int fd;
      cygheap_fdenum cfd (false);
      while ((fd = cfd.next ()) >= 0)
	if (cfd->get_major () == DEV_PTYS_MAJOR)
	  {
	    fhandler_pty_slave *ptys
	      = (fhandler_pty_slave *)(fhandler_base *) cfd;
	    ptys->create_invisible_console ();
	    ptys->setup_locale ();
	  }
    }
  if (!iscygwin && ptys_primary && is_console_app (runpath))
    {
      if (h_stdin == ptys_primary->get_handle_nat ())
	stdin_is_ptys = true;
      if (reset_sendsig)
	myself->sendsig = myself->exec_sendsig;
      ptys_primary->setup_for_non_cygwin_app (nopcon, envblock, stdin_is_ptys);
      if (reset_sendsig)
	myself->sendsig = NULL;
      ptys_primary->get_duplicated_handle_set (&ptys_handle_set);
      ptys_ttyp = (tty *) ptys_primary->tc ();
      ptys_need_cleanup = true;
    }
}

void
fhandler_termios::spawn_worker::cleanup ()
{
  if (ptys_need_cleanup)
    fhandler_pty_slave::cleanup_for_non_cygwin_app (&ptys_handle_set,
						    ptys_ttyp, stdin_is_ptys);
  if (cons_need_cleanup)
    fhandler_console::cleanup_for_non_cygwin_app (&cons_handle_set);
  close_handle_set ();
}

void
fhandler_termios::spawn_worker::close_handle_set ()
{
  if (ptys_need_cleanup)
    fhandler_pty_slave::close_handle_set (&ptys_handle_set);
  if (cons_need_cleanup)
    fhandler_console::close_handle_set (&cons_handle_set);
}
