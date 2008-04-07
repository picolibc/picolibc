/* fhandler_termios.cc

   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <ctype.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "sigproc.h"
#include "pinfo.h"
#include "tty.h"
#include "sys/cygwin.h"
#include "cygtls.h"

/* Common functions shared by tty/console */

void
fhandler_termios::tcinit (tty_min *this_tc, bool force)
{
  /* Initial termios values */

  tc = this_tc;

  if (force || !tc->initialized ())
    {
      tc->ti.c_iflag = BRKINT | ICRNL | IXON;
      tc->ti.c_oflag = OPOST | ONLCR;
      tc->ti.c_cflag = B38400 | CS8 | CREAD;
      tc->ti.c_lflag = ISIG | ICANON | ECHO | IEXTEN;

      tc->ti.c_cc[VDISCARD]	= CFLUSH;
      tc->ti.c_cc[VEOL]		= CEOL;
      tc->ti.c_cc[VEOL2]	= CEOL2;
      tc->ti.c_cc[VEOF]		= CEOF;
      tc->ti.c_cc[VERASE]	= CERASE;
      tc->ti.c_cc[VINTR]	= CINTR;
      tc->ti.c_cc[VKILL]	= CKILL;
      tc->ti.c_cc[VLNEXT]	= CLNEXT;
      tc->ti.c_cc[VMIN]		= 1;
      tc->ti.c_cc[VQUIT]	= CQUIT;
      tc->ti.c_cc[VREPRINT]	= CRPRNT;
      tc->ti.c_cc[VSTART]	= CSTART;
      tc->ti.c_cc[VSTOP]	= CSTOP;
      tc->ti.c_cc[VSUSP]	= CSUSP;
      tc->ti.c_cc[VSWTC]	= CSWTCH;
      tc->ti.c_cc[VTIME]	= 0;
      tc->ti.c_cc[VWERASE]	= CWERASE;

      tc->ti.c_ispeed = tc->ti.c_ospeed = B38400;
      tc->pgid = myself->pgid;
      tc->initialized (true);
    }
}

int
fhandler_termios::tcsetpgrp (const pid_t pgid)
{
  termios_printf ("tty %d pgid %d, sid %d, tsid %d", tc->ntty, pgid,
		    myself->sid, tc->getsid ());
  if (myself->sid != tc->getsid ())
    {
      set_errno (EPERM);
      return -1;
    }
  int res;
  while (1)
    {
      res = bg_check (-SIGTTOU);

      switch (res)
	{
	case bg_ok:
	  tc->setpgid (pgid);
	  init_console_handler (tc->gethwnd ());
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
  return tc->pgid;
}

void
tty_min::kill_pgrp (int sig)
{
  int killself = 0;
  winpids pids ((DWORD) PID_MAP_RW);
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;
  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      if (!p->exists () || p->ctty != ntty || p->pgid != pgid)
	continue;
      if (p == myself)
	killself++;
      else
	sig_send (p, si);
    }
  if (killself)
    sig_send (myself, si);
}

bg_check_types
fhandler_termios::bg_check (int sig)
{
  if (!myself->pgid || tc->getpgid () == myself->pgid ||
	myself->ctty != tc->ntty ||
	((sig == SIGTTOU) && !(tc->ti.c_lflag & TOSTOP)))
    return bg_ok;

  if (sig < 0)
    sig = -sig;

  termios_printf ("bg I/O pgid %d, tpgid %d, %s, ntty tty%d", myself->pgid, tc->getpgid (),
		  myctty (), tc->ntty);

  if (tc->getsid () == 0)
    {
      /* The pty has been closed by the master.  Return an EOF
	 indication.  FIXME: There is nothing to stop somebody
	 from reallocating this pty.  I think this is the case
	 which is handled by unlockpt on a Unix system.  */
      termios_printf ("closed by master");
      return bg_eof;
    }

  /* If the process group is no more or if process is ignoring or blocks 'sig',
     return with error */
  int pgid_gone = !pid_exists (myself->pgid);
  int sigs_ignored =
    ((void *) global_sigs[sig].sa_handler == (void *) SIG_IGN) ||
    (_main_tls->sigmask & SIGTOMASK (sig));

  if (pgid_gone)
    goto setEIO;
  else if (!sigs_ignored)
    /* nothing */;
  else if (sig == SIGTTOU)
    return bg_ok;		/* Just allow the output */
  else
    goto setEIO;	/* This is an output error */

  /* Don't raise a SIGTT* signal if we have already been interrupted
     by another signal. */
  if (WaitForSingleObject (signal_arrived, 0) != WAIT_OBJECT_0)
    {
      siginfo_t si = {0};
      si.si_signo = sig;
      si.si_code = SI_KERNEL;
      kill_pgrp (myself->pgid, si);
    }
  return bg_signalled;

setEIO:
  set_errno (EIO);
  return bg_error;
}

#define set_input_done(x) input_done = input_done || (x)

inline void
fhandler_termios::echo_erase (int force)
{
  if (force || tc->ti.c_lflag & ECHO)
    doecho ("\b \b", 3);
}

line_edit_status
fhandler_termios::line_edit (const char *rptr, int nread, termios& ti)
{
  line_edit_status ret = line_edit_ok;
  char c;
  int input_done = 0;
  bool sawsig = false;
  int iscanon = ti.c_lflag & ICANON;

  while (nread-- > 0)
    {
      c = *rptr++;

      termios_printf ("char %c", c);

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
	  tc->kill_pgrp (sig);
	  ti.c_lflag &= ~FLUSHO;
	  sawsig = true;
	  goto restart_output;
	}
    not_a_sig:
      if (ti.c_iflag & IXON)
	{
	  if (CCEQ (ti.c_cc[VSTOP], c))
	    {
	      if (!tc->output_stopped)
		{
		  tc->output_stopped = 1;
		  acquire_output_mutex (INFINITE);
		}
	      continue;
	    }
	  else if (CCEQ (ti.c_cc[VSTART], c))
	    {
    restart_output:
	      tc->output_stopped = 0;
	      release_output_mutex ();
	      continue;
	    }
	  else if ((ti.c_iflag & IXANY) && tc->output_stopped)
	    goto restart_output;
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
      if (!iscanon || input_done)
	{
	  int status = accept_input ();
	  if (status != 1)
	    {
	      ret = status ? line_edit_error : line_edit_pipe_full;
	      eat_readahead (1);
	      break;
	    }
	  ret = line_edit_input_done;
	  input_done = 0;
	}
    }

  if (!iscanon && ralen > 0)
    ret = line_edit_input_done;

  if (sawsig)
    ret = line_edit_signalled;

  return ret;
}

_off64_t
fhandler_termios::lseek (_off64_t, int)
{
  set_errno (ESPIPE);
  return -1;
}
