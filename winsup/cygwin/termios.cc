/* termios.cc: termios for WIN32.

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

   Written by Doug Evans and Steve Chamberlain of Cygnus Support
   dje@cygnus.com, sac@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <signal.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "dtable.h"
#include <cygwin/version.h>
#include "perprocess.h"
#include <sys/termios.h>

/* tcsendbreak: POSIX 7.2.2.1 */
extern "C" int
tcsendbreak (int fd, int duration)
{
  int res = -1;

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      goto out;
    }

  fhandler_base *fh;
  fh = fdtab[fd];

  if (!fh->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fh->bg_check (-SIGTTOU)) > 0)
	res = fh->tcsendbreak (duration);
    }

out:
  syscall_printf ("%d = tcsendbreak (%d, %d)", res, fd, duration);
  return res;
}

/* tcdrain: POSIX 7.2.2.1 */
extern "C" int
tcdrain (int fd)
{
  int res = -1;

  termios_printf ("tcdrain");

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      goto out;
    }

  fhandler_base *fh;
  fh = fdtab[fd];

  if (!fh->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fh->bg_check (-SIGTTOU)) > 0)
	res = fh->tcdrain ();
    }

out:
  syscall_printf ("%d = tcdrain (%d)", res, fd);
  return res;
}

/* tcflush: POSIX 7.2.2.1 */
extern "C" int
tcflush (int fd, int queue)
{
  int res = -1;

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      goto out;
    }

  fhandler_base *fh;
  fh = fdtab[fd];

  if (!fh->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fh->bg_check (-SIGTTOU)) > 0)
	res = fh->tcflush (queue);
    }

out:
  termios_printf ("%d = tcflush (%d, %d)", res, fd, queue);
  return res;
}

/* tcflow: POSIX 7.2.2.1 */
extern "C" int
tcflow (int fd, int action)
{
  int res = -1;

  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      goto out;
    }

  fhandler_base *fh;
  fh = fdtab[fd];

  if (!fh->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fh->bg_check (-SIGTTOU)) > 0)
	res = fh->tcflow (action);
    }

out:
  syscall_printf ("%d = tcflow (%d, %d)", res, fd, action);
  return res;
}

/* tcsetattr: POSIX96 7.2.1.1 */
extern "C" int
tcsetattr (int fd, int a, const struct termios *t)
{
  int res = -1;

  t = __tonew_termios (t);
  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      goto out;
    }

  fhandler_base *fh;
  fh = fdtab[fd];

  if (!fh->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fh->bg_check (-SIGTTOU)) > 0)
	res = fh->tcsetattr (a, t);
    }

out:
  termios_printf ("iflag %x, oflag %x, cflag %x, lflag %x, VMIN %d, VTIME %d",
	t->c_iflag, t->c_oflag, t->c_cflag, t->c_lflag, t->c_cc[VMIN],
	t->c_cc[VTIME]);
  termios_printf ("%d = tcsetattr (%d, %d, %x)", res, fd, a, t);
  return res;
}

/* tcgetattr: POSIX 7.2.1.1 */
extern "C" int
tcgetattr (int fd, struct termios *in_t)
{
  int res = -1;
  struct termios *t = __makenew_termios (in_t);

  if (fdtab.not_open (fd))
    set_errno (EBADF);
  else if (!fdtab[fd]->is_tty ())
    set_errno (ENOTTY);
  else
    {
      if ((res = fdtab[fd]->tcgetattr (t)) == 0)
	(void) __toapp_termios (in_t, t);
    }

  if (res)
    termios_printf ("%d = tcgetattr (%d, %x)", res, fd, in_t);
  else
    termios_printf ("iflag %x, oflag %x, cflag %x, lflag %x, VMIN %d, VTIME %d",
	  t->c_iflag, t->c_oflag, t->c_cflag, t->c_lflag, t->c_cc[VMIN],
	  t->c_cc[VTIME]);

  return res;
}

/* tcgetpgrp: POSIX 7.2.3.1 */
extern "C" int
tcgetpgrp (int fd)
{
  int res = -1;

  if (fdtab.not_open (fd))
    set_errno (EBADF);
  else if (!fdtab[fd]->is_tty ())
    set_errno (ENOTTY);
  else
    res = fdtab[fd]->tcgetpgrp ();

  termios_printf ("%d = tcgetpgrp (%d)", res, fd);
  return res;
}

/* tcsetpgrp: POSIX 7.2.4.1 */
extern "C" int
tcsetpgrp (int fd, pid_t pgid)
{
  int res = -1;

  if (fdtab.not_open (fd))
    set_errno (EBADF);
  else if (!fdtab[fd]->is_tty ())
    set_errno (ENOTTY);
  else
    res = fdtab[fd]->tcsetpgrp (pgid);

  termios_printf ("%d = tcsetpgrp (%d, %x)", res, fd, pgid);
  return res;
}

/* NIST PCTS requires not macro-only implementation */
#undef cfgetospeed
#undef cfgetispeed
#undef cfsetospeed
#undef cfsetispeed

/* cfgetospeed: POSIX96 7.1.3.1 */
extern "C" speed_t
cfgetospeed (struct termios *tp)
{
  return __tonew_termios(tp)->c_ospeed;
}

/* cfgetispeed: POSIX96 7.1.3.1 */
extern "C" speed_t
cfgetispeed (struct termios *tp)
{
  return __tonew_termios(tp)->c_ispeed;
}

/* cfsetospeed: POSIX96 7.1.3.1 */
extern "C" int
cfsetospeed (struct termios *in_tp, speed_t speed)
{
  struct termios *tp = __tonew_termios (in_tp);
  tp->c_ospeed = speed;
  (void) __toapp_termios (in_tp, tp);
  return 0;
}

/* cfsetispeed: POSIX96 7.1.3.1 */
extern "C" int
cfsetispeed (struct termios *in_tp, speed_t speed)
{
  struct termios *tp = __tonew_termios (in_tp);
  tp->c_ispeed = speed;
  (void) __toapp_termios (in_tp, tp);
  return 0;
}
