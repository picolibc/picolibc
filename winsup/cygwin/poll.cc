/* poll.cc. Implements poll(2) via usage of select(2) call.

   Copyright 2000, 2001, 2002 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/time.h>
#include <sys/poll.h>
#include <errno.h>
#include <stdlib.h>
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygerrno.h"
#include "cygheap.h"
#include "sigproc.h"

extern "C"
int
poll (struct pollfd *fds, unsigned int nfds, int timeout)
{
  int max_fd = 0;
  fd_set *read_fds, *write_fds, *except_fds;
  struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
  sigframe thisframe (mainthread);

  for (unsigned int i = 0; i < nfds; ++i)
    if (fds[i].fd > max_fd)
      max_fd = fds[i].fd;

  size_t fds_size = howmany(max_fd + 1, NFDBITS) * sizeof (fd_mask);

  read_fds = (fd_set *) alloca (fds_size);
  write_fds = (fd_set *) alloca (fds_size);
  except_fds = (fd_set *) alloca (fds_size);

  if (!read_fds || !write_fds || !except_fds)
    {
      set_errno (ENOMEM);
      return -1;
    }

  memset (read_fds, 0, fds_size);
  memset (write_fds, 0, fds_size);
  memset (except_fds, 0, fds_size);

  int invalid_fds = 0;
  for (unsigned int i = 0; i < nfds; ++i)
    {
      fds[i].revents = 0;
      if (!cygheap->fdtab.not_open(fds[i].fd))
	{
	  if (fds[i].events & POLLIN)
	    FD_SET(fds[i].fd, read_fds);
	  if (fds[i].events & POLLOUT)
	    FD_SET(fds[i].fd, write_fds);
	  if (fds[i].events & POLLPRI)
	    FD_SET(fds[i].fd, except_fds);
	}
      else if (fds[i].fd >= 0)
	{
	  ++invalid_fds;
	  fds[i].revents = POLLNVAL;
	}
    }

  if (invalid_fds)
    return invalid_fds;

  int ret = cygwin_select (max_fd + 1, read_fds, write_fds, except_fds, timeout < 0 ? NULL : &tv);

  if (ret > 0)
    for (unsigned int i = 0; i < nfds; ++i)
      {
	if (fds[i].fd >= 0)
	  {
	    if (cygheap->fdtab.not_open(fds[i].fd))
	      fds[i].revents = POLLHUP;
	    else
	      {
		if (FD_ISSET(fds[i].fd, read_fds))
		  fds[i].revents |= POLLIN;
		if (FD_ISSET(fds[i].fd, write_fds))
		  fds[i].revents |= POLLOUT;
		if (FD_ISSET(fds[i].fd, except_fds))
		  fds[i].revents |= POLLPRI;
	      }
	  }
      }

  return ret;
}
