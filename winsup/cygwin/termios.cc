/* termios.cc: termios for WIN32.

   Written by Doug Evans and Steve Chamberlain of Cygnus Support
   dje@cygnus.com, sac@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygwin/version.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "perprocess.h"
#include "cygtls.h"

/* tcsendbreak: POSIX 7.2.2.1 */
extern "C" int
tcsendbreak (int fd, int duration)
{
  int res = -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    goto out;

  if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else if ((res = cfd->bg_check (-SIGTTOU)) > bg_eof)
    res = cfd->tcsendbreak (duration);

out:
  syscall_printf ("%R = tcsendbreak(%d, %d)", res, fd, duration);
  return res;
}

/* tcdrain: POSIX 7.2.2.1 */
extern "C" int
tcdrain (int fd)
{
  pthread_testcancel ();

  int res = -1;

  termios_printf ("tcdrain");

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    goto out;

  if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else if ((res = cfd->bg_check (-SIGTTOU)) > bg_eof)
    res = cfd->tcdrain ();

out:
  syscall_printf ("%R = tcdrain(%d)", res, fd);
  return res;
}

/* tcflush: POSIX 7.2.2.1 */
extern "C" int
tcflush (int fd, int queue)
{
  int res = -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    goto out;

  if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else if (queue != TCIFLUSH && queue != TCOFLUSH && queue != TCIOFLUSH)
      set_errno (EINVAL);
  else if ((res = cfd->bg_check (-SIGTTOU)) > bg_eof)
    res = cfd->tcflush (queue);

out:
  termios_printf ("%R = tcflush(%d, %d)", res, fd, queue);
  return res;
}

/* tcflow: POSIX 7.2.2.1 */
extern "C" int
tcflow (int fd, int action)
{
  int res = -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    goto out;

  if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else if ((res = cfd->bg_check (-SIGTTOU)) > bg_eof)
    res = cfd->tcflow (action);

out:
  syscall_printf ("%R = tcflow(%d, %d)", res, fd, action);
  return res;
}

/* tcsetattr: POSIX96 7.2.1.1 */
extern "C" int
tcsetattr (int fd, int a, const struct termios *t)
{
  int res;
  int e = get_errno ();

  while (1)
    {
      res = -1;
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	{
	  e = get_errno ();
	  break;
	}

      if (!cfd->is_tty ())
	{
	  e = ENOTTY;
	  break;
	}

      res = cfd->bg_check (-SIGTTOU);

      switch (res)
	{
	case bg_eof:
	  e = get_errno ();
	  break;
	case bg_ok:
	  if (cfd.isopen ())
	    res = cfd->tcsetattr (a, t);
	  e = get_errno ();
	  break;
	case bg_signalled:
	  if (_my_tls.call_signal_handler ())
	    continue;
	  res = -1;
	  fallthrough;
	default:
	  e = get_errno ();
	  break;
	}
      break;
    }

  set_errno (e);
  termios_printf ("iflag %y, oflag %y, cflag %y, lflag %y, VMIN %d, VTIME %d",
	t->c_iflag, t->c_oflag, t->c_cflag, t->c_lflag, t->c_cc[VMIN],
	t->c_cc[VTIME]);
  termios_printf ("%R = tcsetattr(%d, %d, %p)", res, fd, a, t);
  return res;
}

/* tcgetattr: POSIX 7.2.1.1 */
extern "C" int
tcgetattr (int fd, struct termios *t)
{
  int res = -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    /* saw an error */;
  else if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else
    res = cfd->tcgetattr (t);

  if (res)
    termios_printf ("%R = tcgetattr(%d, %p)", res, fd, t);
  else
    termios_printf ("iflag %y, oflag %y, cflag %y, lflag %y, VMIN %d, VTIME %d",
	  t->c_iflag, t->c_oflag, t->c_cflag, t->c_lflag, t->c_cc[VMIN],
	  t->c_cc[VTIME]);

  return res;
}

/* tcgetpgrp: POSIX 7.2.3.1 */
extern "C" int
tcgetpgrp (int fd)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->tcgetpgrp ();

  termios_printf ("%R = tcgetpgrp(%d)", res, fd);
  return res;
}

extern "C" pid_t
tcgetsid (int fd)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else
    res = cfd->tcgetsid ();

  termios_printf ("%R = tcgetsid(%d)", res, fd);
  return res;
}

/* tcsetpgrp: POSIX 7.2.4.1 */
extern "C" int
tcsetpgrp (int fd, pid_t pgid)
{
  int res = -1;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    /* saw an error */;
  else if (!cfd->is_tty ())
    set_errno (ENOTTY);
  else
    res = cfd->tcsetpgrp (pgid);

  termios_printf ("%R = tcsetpgrp(%d, %d)", res, fd, pgid);
  return res;
}

/* NIST PCTS requires not macro-only implementation */
#undef cfgetospeed
#undef cfgetispeed
#undef cfsetospeed
#undef cfsetispeed

/* cfgetospeed: POSIX96 7.1.3.1 */
extern "C" speed_t
cfgetospeed (const struct termios *tp)
{
  return tp->c_ospeed;
}

/* cfgetispeed: POSIX96 7.1.3.1 */
extern "C" speed_t
cfgetispeed (const struct termios *tp)
{
  return tp->c_ispeed;
}

static inline int
setspeed (speed_t &set_speed, speed_t from_speed)
{
  int res;
  switch (from_speed)
    {
    case B0:
    case B50:
    case B75:
    case B110:
    case B134:
    case B150:
    case B200:
    case B300:
    case B600:
    case B1200:
    case B1800:
    case B2400:
    case B4800:
    case B9600:
    case B19200:
    case B38400:
    case B57600:
    case B115200:
    case B128000:
    case B230400:
    case B256000:
    case B460800:
    case B500000:
    case B576000:
    case B921600:
    case B1000000:
    case B1152000:
    case B1500000:
    case B2000000:
    case B2500000:
    case B3000000:
      set_speed = from_speed;
      res = 0;
      break;
    default:
      set_errno (EINVAL);
      res = -1;
      break;
    }
  return res;
}

/* cfsetospeed: POSIX96 7.1.3.1 */
extern "C" int
cfsetospeed (struct termios *tp, speed_t speed)
{
  return setspeed (tp->c_ospeed, speed);
}

/* cfsetispeed: POSIX96 7.1.3.1 */
extern "C" int
cfsetispeed (struct termios *tp, speed_t speed)
{
  return setspeed (tp->c_ispeed, speed);
}

struct speed_struct
{
  speed_t value;
  speed_t internal;
};

static const struct speed_struct speeds[] =
  {
    { 0, B0 },
    { 50, B50 },
    { 75, B75 },
    { 110, B110 },
    { 134, B134 },
    { 150, B150 },
    { 200, B200 },
    { 300, B300 },
    { 600, B600 },
    { 1200, B1200 },
    { 1800, B1800 },
    { 2400, B2400 },
    { 4800, B4800 },
    { 9600, B9600 },
    { 19200, B19200 },
    { 38400, B38400 },
    { 57600, B57600 },
    { 115200, B115200 },
    { 128000, B128000 },
    { 230400, B230400 },
    { 256000, B256000 },
    { 460800, B460800 },
    { 500000, B500000 },
    { 576000, B576000 },
    { 921600, B921600 },
    { 1000000, B1000000 },
    { 1152000, B1152000 },
    { 1500000, B1500000 },
    { 2000000, B2000000 },
    { 2500000, B2500000 },
    { 3000000, B3000000 },
  };

/* Given a numerical baud rate (e.g., 9600), convert it to a Bnnn
   constant (e.g., B9600). */
static speed_t
convert_speed (speed_t speed)
{
  for (size_t i = 0; i < sizeof speeds / sizeof speeds[0]; i++)
    {
      if (speed == speeds[i].internal)
	return speed;
      else if (speed == speeds[i].value)
	return speeds[i].internal;
    }
  return speed;
}

/* cfsetspeed: 4.4BSD */
/* Following Linux (undocumented), allow speed to be a numerical baud rate. */
extern "C" int
cfsetspeed (struct termios *tp, speed_t speed)
{
  int res;

  speed = convert_speed (speed);
  /* errors come only from unsupported baud rates, so setspeed() would return
     identical results in both calls */
  if ((res = setspeed (tp->c_ospeed, speed)) == 0)
    setspeed (tp->c_ispeed, speed);
  return res;
}

extern "C" void
cfmakeraw(struct termios *tp)
{
  tp->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
		 | INLCR | IGNCR | ICRNL | IXON);
  tp->c_oflag &= ~OPOST;
  tp->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tp->c_cflag &= ~(CSIZE | PARENB);
  tp->c_cflag |= CS8;
}
