/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * CV 2003-09-10: Cygwin specific changes applied.  Code simplified just
 *		  for Cygwin alone.
 */

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <err.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygtls.h"

extern "C" int
daemon (int nochdir, int noclose)
{
  int fd;

  switch (fork ())
    {
      case -1:
	return -1;
      case 0:
	break;
      default:
	/* This sleep avoids a race condition which kills the
	   child process if parent is started by a service process.
	   FIXME: Is that still true? */
	Sleep (1000L);
	_exit (0);
    }
  if (setsid () == -1)
    return -1;
  if (!nochdir)
    chdir ("/");
  if (!noclose && (fd = open (_PATH_DEVNULL, O_RDWR, 0)) >= 0)
    {
      dup2 (fd, STDIN_FILENO);
      dup2 (fd, STDOUT_FILENO);
      dup2 (fd, STDERR_FILENO);
      if (fd > 2)
	close (fd);
    }
  return 0;
}

extern "C" int
login_tty (int fd)
{
  /* If setsid fails, FreeBSD uses the current sid returned by getsid(0),
     then calls tcsetsid, which we don't provide (just as Linux doesn't).
     tcsetsid only uses the sid to check against the value returned by
     getsid(0).  So, either way, that check will not fail and we can
     simply ignore the return value from setsid and just perform the
     ioctl call tcsetsid does. */
  setsid ();
  if (ioctl (fd, TIOCSCTTY, NULL) == -1)
    return -1;
  dup2 (fd, STDIN_FILENO);
  dup2 (fd, STDOUT_FILENO);
  dup2 (fd, STDERR_FILENO);
  if (fd > 2)
    close (fd);
  return 0;
}

extern "C" int
openpty (int *amaster, int *aslave, char *name, const struct termios *termp,
	 const struct winsize *winp)
{
  int master, slave;
  char pts[TTY_NAME_MAX];

  if ((master = open ("/dev/ptmx", O_RDWR | O_NOCTTY)) >= 0)
    {
      grantpt (master);
      unlockpt (master);
      __ptsname (pts, cygheap->fdtab[master]->get_minor ());
      revoke (pts);
      if ((slave = open (pts, O_RDWR | O_NOCTTY)) >= 0)
	{
	  if (amaster)
	    *amaster = master;
	  if (name)
	    strcpy (name, pts);
	  if (termp)
	    tcsetattr (slave, TCSAFLUSH, termp);
	  if (winp)
	    ioctl (slave, TIOCSWINSZ, (char *) winp);
	  /* The man page doesn't say that aslave can be NULL but we have
	     allowed it for years.  As of 2011-11-08 we now avoid a handle
	     leak in this case.  */
	  if (aslave)
	    *aslave = slave;
	  else
	    close (slave);
	  return 0;
	}
      close (master);
    }
  set_errno (ENOENT);
  return -1;
}

extern "C" int
forkpty (int *amaster, char *name, const struct termios *termp,
	 const struct winsize *winp)
{
  int master, slave, pid;

  if (openpty (&master, &slave, name, termp, winp) == -1)
    return -1;
  switch (pid = fork ())
    {
      case -1:
	close (master);
	close (slave);
	return -1;
      case 0:
	close (master);
	login_tty (slave);
	return 0;
    }
  if (amaster)
    *amaster = master;
  close (slave);
  return pid;
}

extern "C" char *__progname;

static void
_vwarnx (const char *fmt, va_list ap)
{
  fprintf (stderr, "%s: ", __progname);
  vfprintf (stderr, fmt, ap);
}

extern "C" void
vwarn (const char *fmt, va_list ap)
{
  _vwarnx (fmt, ap);
  fprintf (stderr, ": %s", strerror (get_errno ()));
  fputc ('\n', stderr);
}

extern "C" void
vwarnx (const char *fmt, va_list ap)
{
  _vwarnx (fmt, ap);
  fputc ('\n', stderr);
}

extern "C" void
warn (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vwarn (fmt, ap);
  va_end (ap);
}

extern "C" void
warnx (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vwarnx (fmt, ap);
  va_end (ap);
}

extern "C" void
verr (int eval, const char *fmt, va_list ap)
{
  vwarn (fmt, ap);
  exit (eval);
}

extern "C" void
verrx (int eval, const char *fmt, va_list ap)
{
  vwarnx (fmt, ap);
  exit (eval);
}

extern "C" void
err (int eval, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vwarn (fmt, ap);
  va_end (ap);
  exit (eval);
}

extern "C" void
errx (int eval, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vwarnx (fmt, ap);
  va_end (ap);
  exit (eval);
}

extern "C" const char *
getprogname (void)
{
  return __progname;
}

extern "C" void
setprogname (const char *newprogname)
{
  __try
    {
      /* Per BSD man page, setprogname keeps a pointer to the last
	 path component of the argument.  It does *not* copy the
	 argument before. */
      __progname = strrchr (newprogname, '/');
      if (__progname)
	++__progname;
      else
	__progname = (char *)newprogname;
    }
  __except (EFAULT) {}
  __endtry
}

extern "C" void
logwtmp (const char *line, const char *user, const char *host)
{
  struct utmp ut;
  memset (&ut, 0, sizeof ut);
  ut.ut_type = USER_PROCESS;
  ut.ut_pid = getpid ();
  if (line)
    strncpy (ut.ut_line, line, sizeof ut.ut_line);
  time (&ut.ut_time);
  if (user)
    strncpy (ut.ut_user, user, sizeof ut.ut_user);
  if (host)
    strncpy (ut.ut_host, host, sizeof ut.ut_host);
  updwtmp (_PATH_WTMP, &ut);
}

extern "C" void
login (const struct utmp *ut)
{
  pututline (ut);
  endutent ();
  updwtmp (_PATH_WTMP, ut);
}

extern "C" int
logout (const char *line)
{
  struct utmp ut_buf, *ut;

  memset (&ut_buf, 0, sizeof ut_buf);
  strncpy (ut_buf.ut_line, line, sizeof ut_buf.ut_line);
  setutent ();
  ut = getutline (&ut_buf);

  if (ut)
    {
      ut->ut_type = DEAD_PROCESS;
      memset (ut->ut_user, 0, sizeof ut->ut_user);
      memset (ut->ut_host, 0, sizeof ut->ut_host);
      time (&ut->ut_time);
      debug_printf ("set logout time for %s", line);
      pututline (ut);
      endutent ();
      return 1;
    }
  return 0;
}
