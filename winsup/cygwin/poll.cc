/* poll.cc. Implements poll(2) via usage of select(2) call.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define  __INSIDE_CYGWIN_NET__

#define FD_SETSIZE 16384		// lots of fds
#include "winsup.h"
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <stdlib.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"

extern "C" int
poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
  int max_fd = 0;
  fd_set *read_fds, *write_fds, *except_fds;
  struct timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };

  for (unsigned int i = 0; i < nfds; ++i)
    if (fds[i].fd > max_fd)
      max_fd = fds[i].fd;

  size_t fds_size = howmany (max_fd + 1, NFDBITS) * sizeof (fd_mask);

  read_fds = (fd_set *) alloca (fds_size);
  write_fds = (fd_set *) alloca (fds_size);
  except_fds = (fd_set *) alloca (fds_size);

  if (!read_fds || !write_fds || !except_fds)
    {
      set_errno (EINVAL);	/* According to SUSv3. */
      return -1;
    }

  memset (read_fds, 0, fds_size);
  memset (write_fds, 0, fds_size);
  memset (except_fds, 0, fds_size);

  int invalid_fds = 0;
  for (unsigned int i = 0; i < nfds; ++i)
    {
      fds[i].revents = 0;
      if (!cygheap->fdtab.not_open (fds[i].fd))
	{
	  if (fds[i].events & POLLIN)
	    FD_SET(fds[i].fd, read_fds);
	  if (fds[i].events & POLLOUT)
	    FD_SET(fds[i].fd, write_fds);
	  /* On sockets, except_fds is needed to catch failed connects. */
	  if ((fds[i].events & POLLPRI)
	      || cygheap->fdtab[fds[i].fd]->is_socket ())
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

  int ret = cygwin_select (max_fd + 1, read_fds, write_fds, except_fds,
			   timeout < 0 ? NULL : &tv);
  if (ret <= 0)
    return ret;

  /* Set revents fields and count fds with non-zero revents fields for
     return value. */
  ret = 0;
  for (unsigned int i = 0; i < nfds; ++i)
    {
      if (fds[i].fd >= 0)
	{
	  if (cygheap->fdtab.not_open (fds[i].fd))
	    fds[i].revents = POLLHUP;
	  else
	    {
	      fhandler_socket *sock;

	      if (FD_ISSET(fds[i].fd, read_fds))
		{
		  char peek[1];
		  sock = cygheap->fdtab[fds[i].fd]->is_socket ();
		  if (!sock)
		    fds[i].revents |= POLLIN;
		  else
		    {
		      /* The following action can change errno.  We have to
			 reset it to it's old value. */
		      int old_errno = get_errno ();
		      switch (sock->recvfrom (peek, sizeof (peek), MSG_PEEK,
					      NULL, NULL))
			{
			  case -1: /* Something weird happened */
			    /* When select returns that data is available,
			       that could mean that the socket is in
			       listen mode and a client tries to connect.
			       Unfortunately, recvfrom() doesn't make much
			       sense then.  It returns WSAENOTCONN in that
			       case.  Since that's not actually an error,
			       we must not set POLLERR but POLLIN. */
			    if (WSAGetLastError () != WSAENOTCONN)
			      fds[i].revents |= POLLERR;
			    else
			      fds[i].revents |= POLLIN;
			    break;
			  case 0:  /* Closed on the read side... */
			    /* ...or shutdown(SHUT_WR) on the write side.
			       We set revents to POLLHUP until 1.5.18, but
			       this is semantically borderline. */
			    fds[i].revents |= POLLIN;
			    break;
			  default:
			    fds[i].revents |= POLLIN;
			    break;
			}
		      set_errno (old_errno);
		    }
		}
	      /* Handle failed connect. */
	      if (FD_ISSET(fds[i].fd, write_fds)
		  && FD_ISSET(fds[i].fd, except_fds)
		  && (sock = cygheap->fdtab[fds[i].fd]->is_socket ())
		  && sock->connect_state () == connect_failed)
		fds[i].revents |= (POLLIN | POLLERR);
	      else
		{
		  if (FD_ISSET(fds[i].fd, write_fds))
		    fds[i].revents |= POLLOUT;
		  if (FD_ISSET(fds[i].fd, except_fds))
		    fds[i].revents |= POLLPRI;
		}
	    }
	  if (fds[i].revents)
	    ++ret;
	}
    }

  return ret;
}
