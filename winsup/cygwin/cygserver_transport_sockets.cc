/* cygserver_transport_sockets.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "wincap.h"
#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_sockets.h"

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifndef __OUTSIDE_CYGWIN__
#include "winsup.h"
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
#define cygwin_accept(A,B,C)  ::accept(A,B,C)
#define cygwin_socket(A,B,C)  ::socket(A,B,C)
#define cygwin_listen(A,B)    ::listen(A,B)
#define cygwin_bind(A,B,C)    ::bind(A,B,C)
#define cygwin_connect(A,B,C) ::connect(A,B,C)
#define debug_printf printf
#endif

transport_layer_sockets::transport_layer_sockets (int newfd): fd(newfd)
{
  /* This may not be needed in this constructor - it's only used
   * when creating a connection via bind or connect
   */
  sockdetails.sa_family = AF_UNIX;
  strcpy (sockdetails.sa_data, "/tmp/cygdaemo");
  sdlen = strlen(sockdetails.sa_data) + sizeof(sockdetails.sa_family);
};

transport_layer_sockets::transport_layer_sockets (): fd (-1)
{
  sockdetails.sa_family = AF_UNIX;
  strcpy (sockdetails.sa_data, "/tmp/cygdaemo");
  sdlen = strlen(sockdetails.sa_data) + sizeof(sockdetails.sa_family);
}

void
transport_layer_sockets::listen ()
{
  /* we want a thread pool based approach. */
  if ((fd = cygwin_socket (AF_UNIX, SOCK_STREAM,0)) < 0)
    printf ("Socket not created error %d\n", errno);
  if (cygwin_bind(fd, &sockdetails, sdlen))
    printf ("Bind doesn't like you. Tsk Tsk. Bind said %d\n", errno);
  if (cygwin_listen(fd, 5) < 0)
    printf ("And the OS just isn't listening, all it says is %d\n", errno);
}

class transport_layer_sockets *
transport_layer_sockets::accept ()
{
  /* FIXME: check we have listened */
  int new_fd;

  if ((new_fd = cygwin_accept(fd, &sockdetails, &sdlen)) < 0)
    {
      printf ("Nup, could' accept. %d\n",errno);
      return NULL;
    }
  
  transport_layer_sockets *new_conn = new transport_layer_sockets (new_fd);

  return new_conn;  

}

void
transport_layer_sockets::close()
{
  /* FIXME - are we open? */
  ::close (fd);
}

ssize_t
transport_layer_sockets::read (char *buf, size_t len)
{
  /* FIXME: are we open? */
  return ::read (fd, buf, len);
}

ssize_t
transport_layer_sockets::write (char *buf, size_t len)
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
      debug_printf("client connect failure %d\n", errno);
      ::close (fd);
      return false;
    }
  return true;
}
