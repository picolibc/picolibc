/* fhandler_termios.cc

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "tty.h"

/* Common functions shared by tty/console */

void
fhandler_termios::tcinit (tty_min *this_tc, int force)
{
  /* Initial termios values */

  tc = this_tc;

  if (force || !TTYISSETF (INITIALIZED))
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
      TTYSETF (INITIALIZED);
    }
}

int
fhandler_termios::tcsetpgrp (const pid_t pgid)
{
  termios_printf ("pgid %d, sid %d, tsid %d", pgid,
		    myself->sid, tc->getsid ());
  if (myself->sid != tc->getsid ())
    {
      set_errno (EPERM);
      return -1;
    }
  tc->setpgid (pgid);
  return 0;
}

int
fhandler_termios::tcgetpgrp ()
{
  return tc->pgid;
}

void
fhandler_termios::set_ctty (int ttynum, int flags)
{
  if ((myself->ctty < 0 || myself->ctty == ttynum) && !(flags & O_NOCTTY))
    {
      myself->ctty = ttynum;
      syscall_printf ("attached tty%d sid %d, pid %d, tty->pgid %d, tty->sid %d",
		      ttynum, myself->sid, myself->pid, tc->pgid, tc->getsid ());

      pinfo p (tc->getsid ());
      if (myself->sid == myself->pid &&
	  (p == myself || !proc_exists (p)))
	{
	  paranoid_printf ("resetting tty%d sid.  Was %d, now %d.  pgid was %d, now %d.",
			   ttynum, tc->getsid(), myself->sid, tc->getpgid (), myself->pgid);
	  /* We are the session leader */
	  tc->setsid (myself->sid);
	  tc->setpgid (myself->pgid);
	}
      else
	myself->sid = tc->getsid ();
      if (tc->getpgid () == 0)
	tc->setpgid (myself->pgid);
    }
}

int
fhandler_termios::bg_check (int sig)
{
  if (!myself->pgid || tc->getpgid () == myself->pgid ||
	myself->ctty != tc->ntty ||
	((sig == SIGTTOU) && !(tc->ti.c_lflag & TOSTOP)))
    return 1;

  if (sig < 0)
    sig = -sig;

  termios_printf("bg I/O pgid %d, tpgid %d, ctty %d",
		    myself->pgid, tc->getpgid (), myself->ctty);

  if (tc->getsid () == 0)
    {
      /* The pty has been closed by the master.  Return an EOF
	 indication.  FIXME: There is nothing to stop somebody
	 from reallocating this pty.  I think this is the case
	 which is handled by unlockpt on a Unix system.  */
      termios_printf ("closed by master");
      return 0;
    }

  /* If the process group is no more or if process is ignoring or blocks 'sig',
     return with error */
  int pgid_gone = !pid_exists (myself->pgid);
  int sigs_ignored =
    ((void *) myself->getsig(sig).sa_handler == (void *) SIG_IGN) ||
    (myself->getsigmask () & SIGTOMASK (sig));

  if (pgid_gone)
    goto setEIO;
  else if (!sigs_ignored)
    /* nothing */;
  else if (sig == SIGTTOU)
    return 1;		/* Just allow the output */
  else
    goto setEIO;	/* This is an output error */

  _raise (sig);
  return 1;

setEIO:
  set_errno (EIO);
  return -1;
}

#define set_input_done(x) input_done = input_done || (x)

int
fhandler_termios::line_edit (const char *rptr, int nread, int always_accept)
{
  char c;
  int input_done = 0;
  int iscanon = tc->ti.c_lflag & ICANON;

  while (nread-- > 0)
    {
      c = *rptr++;

      termios_printf ("char %c", c);

      /* Check for special chars */

      if (c == '\r')
	{
	  if (tc->ti.c_iflag & IGNCR)
	    continue;
	  if (tc->ti.c_iflag & ICRNL)
	    {
	      c = '\n';
	      set_input_done (iscanon);
	    }
	}
      else if (c == '\n')
	{
	  if (tc->ti.c_iflag & INLCR)
	    c = '\r';
	  else
	    set_input_done (iscanon);
	}

      if (tc->ti.c_iflag & ISTRIP)
	c &= 0x7f;
      if (tc->ti.c_lflag & ISIG)
	{
	  int sig;
	  if (c ==  tc->ti.c_cc[VINTR])
	    sig = SIGINT;
	  else if (c == tc->ti.c_cc[VQUIT])
	    sig = SIGQUIT;
	  else if (c == tc->ti.c_cc[VSUSP])
	    sig = SIGTSTP;
	  else
	    goto not_a_sig;

	  termios_printf ("got interrupt %d, sending signal %d", c, sig);
	  kill_pgrp (tc->getpgid (), sig);
	  tc->ti.c_lflag &= ~FLUSHO;
	  goto restart_output;
	}
    not_a_sig:
      if (tc->ti.c_iflag & IXON)
	{
	  if (c == tc->ti.c_cc[VSTOP])
	    {
	      if (!tc->OutputStopped)
		{
		  tc->OutputStopped = 1;
		  acquire_output_mutex (INFINITE);
		}
	      continue;
	    }
	  else if (c == tc->ti.c_cc[VSTART])
	    {
    restart_output:
	      tc->OutputStopped = 0;
	      release_output_mutex ();
	      continue;
	    }
	  else if ((tc->ti.c_iflag & IXANY) && tc->OutputStopped)
	    goto restart_output;
	}
      if (tc->ti.c_lflag & IEXTEN && c == tc->ti.c_cc[VDISCARD])
	{
	  tc->ti.c_lflag ^= FLUSHO;
	  continue;
	}
      if (!iscanon)
	/* nothing */;
      else if (c == tc->ti.c_cc[VERASE])
	{
	  if (eat_readahead (1))
	    doecho ("\b \b", 3);
	  continue;
	}
      else if (c == tc->ti.c_cc[VWERASE])
	{
	  int ch;
	  do
	    if (!eat_readahead (1))
	      break;
	    else
	      doecho ("\b \b", 3);
	  while ((ch = peek_readahead (1)) >= 0 && !isspace (ch));
	  continue;
	}
      else if (c == tc->ti.c_cc[VKILL])
	{
	  int nchars = eat_readahead (-1);
	  while (nchars--)
	    doecho ("\b \b", 3);
	  continue;
	}
      else if (c == tc->ti.c_cc[VREPRINT])
	{
	  doecho ("\n\r", 2);
	  doecho (rabuf, ralen);
	  continue;
	}
      else if (c == tc->ti.c_cc[VEOF])
	{
	  termios_printf ("EOF");
	  input_done = 1;
	  continue;
	}
      else if (c == tc->ti.c_cc[VEOL] ||
	       c == tc->ti.c_cc[VEOL2] ||
	       c == '\n')
	{
	  set_input_done (1);
	  termios_printf ("EOL");
	}

      if (tc->ti.c_iflag & IUCLC && isupper (c))
	c = cyg_tolower (c);

      if (tc->ti.c_lflag & ECHO)
	doecho (&c, 1);
      put_readahead (c);
    }

  if (!iscanon || always_accept)
    set_input_done (ralen > 0);

  if (input_done)
    (void) accept_input ();

  return input_done;
}
