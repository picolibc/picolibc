/* One of many ways to emulate flock() on top of real (good) POSIX locks.
 *
 * This flock() emulation is based upon source taken from the Red Hat
 * implementation used in their imap-2002d SRPM.
 *
 * $RH: flock.c,v 1.2 2000/08/23 17:07:00 nalin Exp $
 */
/* flock.c

   Copyright 2003 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int
flock (int fd, int operation)
{
  int i, cmd;
  struct flock l = { 0, 0, 0, 0, 0 };
  if (operation & LOCK_NB)
    {
      cmd = F_SETLK;
    }
  else
    {
      cmd = F_SETLKW;
    }
  l.l_whence = SEEK_SET;
  switch (operation & (~LOCK_NB))
    {
    case LOCK_EX:
      l.l_type = F_WRLCK;
      i = fcntl (fd, cmd, &l);
      if (i == -1)
	{
	  if ((errno == EAGAIN) || (errno == EACCES))
	    {
	      errno = EWOULDBLOCK;
	    }
	}
      break;
    case LOCK_SH:
      l.l_type = F_RDLCK;
      i = fcntl (fd, cmd, &l);
      if (i == -1)
	{
	  if ((errno == EAGAIN) || (errno == EACCES))
	    {
	      errno = EWOULDBLOCK;
	    }
	}
      break;
    case LOCK_UN:
      l.l_type = F_UNLCK;
      i = fcntl (fd, cmd, &l);
      if (i == -1)
	{
	  if ((errno == EAGAIN) || (errno == EACCES))
	    {
	      errno = EWOULDBLOCK;
	    }
	}
      break;
    default:
      i = -1;
      errno = EINVAL;
      break;
    }
  return i;
}

#ifdef FLOCK_EMULATE_IS_MAIN
int
main (int argc, char **argv)
{
  int fd = open (argv[1], O_WRONLY);
  flock (fd, LOCK_EX);
  return 0;
}
#endif
