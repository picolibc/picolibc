/* cygserver_transport_sockets.cc

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#else
#include "winsup.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_sockets.h"

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifndef __OUTSIDE_CYGWIN__
extern "C" int
cygwin_socket (int af, int type, int protocol);
extern "C" int
cygwin_connect (int fd,
		const struct sockaddr *name,
		int namelen);
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, int *len);
extern "C" int
cygwin_listen (int fd, int backlog);
extern "C" int
cygwin_bind (int fd, const struct sockaddr *my_addr, int addrlen);

#else
#define cygwin_accept(A,B,C)  ::accept (A,B,C)
#define cygwin_socket(A,B,C)  ::socket (A,B,C)
#define cygwin_listen(A,B)    ::listen (A,B)
#define cygwin_bind(A,B,C)    ::bind (A,B,C)
#define cygwin_connect(A,B,C) ::connect (A,B,C)
#endif

transport_layer_sockets::transport_layer_sockets (int newfd)
  : fd (newfd)
{
  /* This may not be needed in this constructor - it's only used
   * when creating a connection via bind or connect
   */
  sockdetails.sa_family = AF_UNIX;
  strcpy (sockdetails.sa_data, "/tmp/cygdaemo");
  sdlen = strlen (sockdetails.sa_data) + sizeof (sockdetails.sa_family);
};

transport_layer_sockets::transport_layer_sockets (): fd (-1)
{
  sockdetails.sa_family = AF_UNIX;
  strcpy (sockdetails.sa_data, "/tmp/cygdaemo");
  sdlen = strlen (sockdetails.sa_data) + sizeof (sockdetails.sa_family);
}

transport_layer_sockets::~transport_layer_sockets ()
{
  close ();
}

#ifndef __INSIDE_CYGWIN__

void
transport_layer_sockets::listen ()
{
  /* we want a thread pool based approach. */
  if ((fd = cygwin_socket (AF_UNIX, SOCK_STREAM,0)) < 0)
    system_printf ("Socket not created error %d", errno);
  if (cygwin_bind (fd, &sockdetails, sdlen))
    system_printf ("Bind doesn't like you. Tsk Tsk. Bind said %d", errno);
  if (cygwin_listen (fd, 5) < 0)
    system_printf ("And the OS just isn't listening, all it says is %d",
		   errno);
}

class transport_layer_sockets *
transport_layer_sockets::accept (bool *const recoverable)
{
  /* FIXME: check we have listened */
  const int accept_fd = cygwin_accept (fd, &sockdetails, &sdlen);

  if (accept_fd == -1)
    {
      system_printf ("Nup, couldn't accept. %d", errno);
      switch (errno)
	{
	case ECONNABORTED:
	case EINTR:
	case EMFILE:
	case ENFILE:
	case ENOBUFS:
	case ENOMEM:
	  *recoverable = true;
	  break;

	default:
	  *recoverable = false;
	  break;
	}
      return NULL;
    }

  return safe_new (transport_layer_sockets, accept_fd);
}

#endif /* !__INSIDE_CYGWIN__ */

void
transport_layer_sockets::close ()
{
  /* FIXME - are we open? */
  if (fd != -1)
    {
      ::close (fd);
      fd = -1;
    }
}

ssize_t
transport_layer_sockets::read (void *buf, size_t len)
{
  /* FIXME: are we open? */
  return ::read (fd, buf, len);
}

ssize_t
transport_layer_sockets::write (void *buf, size_t len)
{
  /* FIXME: are we open? */
  return ::write (fd, buf, len);
}

bool
transport_layer_sockets::connect ()
{
  /* are we already connected? */
  if (fd != -1)
    return false;
  fd = cygwin_socket (AF_UNIX, SOCK_STREAM, 0);
  if (cygwin_connect (fd, &sockdetails, sdlen) < 0)
    {
      debug_printf ("client connect failure %d", errno);
      ::close (fd);
      return false;
    }
  return true;
}
