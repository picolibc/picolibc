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
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 *                for Cygwin alone.
 */

#include "winsup.h"
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "cygerrno.h"

extern "C" int
daemon (int nochdir, int noclose)
{
  int fd;

  switch (fork ())
    {
      case -1:
	return -1;
      case 0:
	if (!wincap.is_winnt ())
	  {
	    /* Register as service under 9x/Me which allows to close
	       the parent window with the daemon still running.
	       This function only exists on 9x/Me and is autoloaded
	       so it fails silently on NT. */
	    DWORD WINAPI RegisterServiceProcess (DWORD, DWORD);
	    RegisterServiceProcess (0, 1);
	  }
	break;
      default:
	/* This sleep avoids a race condition which kills the
	   child process if parent is started by a NT/W2K service.
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
  char *fdname;
  int newfd;

  if (setsid () == -1)
    return -1;
  if ((fdname = ttyname (fd)))
    {
      if (fd != STDIN_FILENO)
        close (STDIN_FILENO);
      if (fd != STDOUT_FILENO)
        close (STDOUT_FILENO);
      if (fd != STDERR_FILENO)
        close (STDERR_FILENO);
      newfd = open (fdname, O_RDWR);
      close (newfd);
    }
  dup2 (fd, STDIN_FILENO);
  dup2 (fd, STDOUT_FILENO);
  dup2 (fd, STDERR_FILENO);
  if (fd > 2)
    close (fd);
  return 0;
}

extern "C" int
openpty (int *amaster, int *aslave, char *name, struct termios *termp,
	 struct winsize *winp)
{
  int master, slave;
  char pts[MAX_PATH];

  if ((master = open ("/dev/ptmx", O_RDWR | O_NOCTTY)) >= 0)
    {
      grantpt (master);
      unlockpt (master);
      strcpy (pts, ptsname (master));
      revoke (pts);
      if ((slave = open (pts, O_RDWR | O_NOCTTY)) >= 0)
        {
	  if (amaster)
	    *amaster = master;
	  if (aslave)
	    *aslave = slave;
	  if (name)
	    strcpy (name, pts);
	  if (termp)
	    tcsetattr (slave, TCSAFLUSH, termp);
	  if (winp)
	    ioctl (slave, TIOCSWINSZ, (char *) winp);
	  return 0;
	}
      close (master);
    }
  set_errno (ENOENT);
  return -1;
}

extern "C" int
forkpty (int *amaster, char *name, struct termios *termp, struct winsize *winp)
{
  int master, slave, pid;

  if (openpty (&master, &slave, name, termp, winp) == -1)
    return -1;
  switch (pid = fork ())
    {
      case -1:
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

