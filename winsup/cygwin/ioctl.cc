/* ioctl.cc: ioctl routines.

   Written by Doug Evans of Cygnus Support
   dje@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

extern "C" int
ioctl (int fd, int cmd, ...)
{

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return -1;

  /* check for optional mode argument */
  va_list ap;
  va_start (ap, cmd);
  char *argp = va_arg (ap, char *);
  va_end (ap);

  debug_printf ("ioctl(fd %d, cmd %y)", fd, cmd);
  int res;
  if (cfd->get_flags () & O_PATH)
    {
      set_errno (EBADF);
      return -1;
    }
  /* FIXME: This stinks.  There are collisions between cmd types
     depending on whether fd is associated with a pty master or not.
     Something to fix for Cygwin2.  CGF 2006-06-04 */
  if (cfd->is_tty () && cfd->get_major () != DEV_PTYM_MAJOR)
    switch (cmd)
      {
	case TCGETA:
	  res = tcgetattr (fd, (struct termios *) argp);
	  goto out;
	case TCSETA:
	  res = tcsetattr (fd, TCSANOW, (struct termios *) argp);
	  goto out;
	case TCSETAW:
	  res = tcsetattr (fd, TCSADRAIN, (struct termios *) argp);
	  goto out;
	case TCSETAF:
	  res = tcsetattr (fd, TCSAFLUSH, (struct termios *) argp);
	  goto out;
      }

  res = cfd->ioctl (cmd, argp);

out:
  syscall_printf ("%R = ioctl(%d, %y, ...)", res, fd, cmd);
  return res;
}
