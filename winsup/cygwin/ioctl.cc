/* ioctl.cc: ioctl routines.

   Copyright 1996, 1998 Cygnus Solutions.

   Written by Doug Evans of Cygnus Support
   dje@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/ioctl.h>
#include <errno.h>
#include "dtable.h"

extern "C"
int
ioctl (int fd, int cmd, void *buf)
{
  if (fdtab.not_open (fd))
    {
      set_errno (EBADF);
      return -1;
    }

  debug_printf ("fd %d, cmd %x\n", fd, cmd);
  fhandler_base *fh = fdtab[fd];
  if (fh->is_tty () && fh->get_device () != FH_PTYM)
    switch (cmd)
      {
	case TCGETA:
	  return tcgetattr (fd, (struct termios *) buf);
	case TCSETA:
	  return tcsetattr (fd, TCSANOW, (struct termios *) buf);
	case TCSETAW:
	  return tcsetattr (fd, TCSADRAIN, (struct termios *) buf);
	case TCSETAF:
	  return tcsetattr (fd, TCSAFLUSH, (struct termios *) buf);
      }

  return fh->ioctl (cmd, buf);
}
