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
      tc ()->ti.c_lflag = ISIG | ICANON | ECHO | IEXTEN;

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
	  /* fall through intentionally */
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
  if (myself->ctty > 0 && myself->ctty == tc ()->ntty)
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
tty_min::kill_pgrp (int sig)
{
  bool killself = false;
  if (is_flush_sig (sig) && cygheap->ctty)
    cygheap->ctty->sigflush ();
  winpids pids ((DWORD) PID_MAP_RW);
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;

  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      if (!p || !p->exists () || p->ctty != ntty || p->pgid != pgid)
	continue;
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
  int oralen = ralen;
  if (n < 0)
    n = ralen;
  if (n > 0 && ralen > 0)
    {
      if ((int) (ralen -= n) < 0)
	ralen = 0;
      /* If IUTF8 is set, the terminal is in UTF-8 mode.  If so, we erase
	 a complete UTF-8 multibyte sequence on VERASE/VWERASE.  Otherwise,
	 if we only erase a single byte, invalid unicode chars are left in
	 the input. */
      if (tc ()->ti.c_iflag & IUTF8)
	while (ralen > 0 && ((unsigned char) rabuf[ralen] & 0xc0) == 0x80)
	  --ralen;

      if (raixget >= ralen)
	raixget = raixput = ralen = 0;
      else if (raixput > ralen)
	raixput = ralen;
    }

  return oralen;
}

inline void
fhandler_termios::echo_erase (int force)
{
  if (force || tc ()->ti.c_lflag & ECHO)
    doecho ("\b \b", 3);
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

  if (bytes_read)
    *bytes_read = nread;
  while (nread-- > 0)
    {
      c = *rptr++;

      paranoid_printf ("char %0o", c);

      if (ti.c_iflag & ISTRIP)
	c &= 0x7f;
      if (ti.c_lflag & ISIG)
	{
	  int sig;
	  if (CCEQ (ti.c_cc[VINTR], c))
	    sig = SIGINT;
	  else if (CCEQ (ti.c_cc[VQUIT], c))
	    sig = SIGQUIT;
	  else if (CCEQ (ti.c_cc[VSUSP], c))
	    sig = SIGTSTP;
	  else
	    goto not_a_sig;

	  termios_printf ("got interrupt %d, sending signal %d", c, sig);
	  eat_readahead (-1);
	  tc ()->kill_pgrp (sig);
	  ti.c_lflag &= ~FLUSHO;
	  sawsig = true;
	  goto restart_output;
	}
    not_a_sig:
      if (ti.c_iflag & IXON)
	{
	  if (CCEQ (ti.c_cc[VSTOP], c))
	    {
	      if (!tc ()->output_stopped)
		tc ()->output_stopped = true;
	      continue;
	    }
	  else if (CCEQ (ti.c_cc[VSTART], c))
	    {
    restart_output:
	      tc ()->output_stopped = false;
	      continue;
	    }
	  else if ((ti.c_iflag & IXANY) && tc ()->output_stopped)
	    goto restart_output;
	}
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

      if (iscanon && ti.c_lflag & IEXTEN && CCEQ (ti.c_cc[VDISCARD], c))
	{
	  ti.c_lflag ^= FLUSHO;
	  continue;
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
	      doecho (rabuf, ralen);
	    }
	  continue;
	}
      else if (CCEQ (ti.c_cc[VEOF], c))
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
      if ((!iscanon && ralen >= 32) || input_done)
	{
	  int status = accept_input ();
	  if (status != 1)
	    {
	      ret = status ? line_edit_error : line_edit_pipe_full;
	      nread += ralen;
	      break;
	    }
	  ret = line_edit_input_done;
	  input_done = 0;
	}
    }

  /* If we didn't write all bytes in non-canonical mode, write them now. */
  if (!iscanon && ralen > 0
      && (ret == line_edit_ok || ret == line_edit_input_done))
    {
      int status = accept_input ();
      if (status != 1)
	{
	  ret = status ? line_edit_error : line_edit_pipe_full;
	  nread += ralen;
	}
      else
	ret = line_edit_input_done;
    }

  /* Adding one compensates for the postdecrement in the above loop. */
  if (bytes_read)
    *bytes_read -= (nread + 1);

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
  if (myself->ctty > 0 && myself->ctty == tc ()->ntty)
    return tc ()->getsid ();
  set_errno (ENOTTY);
  return -1;
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
  if (myself->ctty > 0 || myself->sid != myself->pid || (!arg && tc ()->getsid () > 0))
    {
      set_errno (EPERM);
      return -1;
    }

  myself->ctty = -1;
  myself->set_ctty (this, 0);
  return 0;
}
