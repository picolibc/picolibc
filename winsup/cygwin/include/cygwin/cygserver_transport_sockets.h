/* cygserver_transport_sockets.h

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_TRANSPORT_SOCKETS_
#define _CYGSERVER_TRANSPORT_SOCKETS_

#include <sys/socket.h>
#include <sys/un.h>

class transport_layer_sockets : public transport_layer_base
{
public:
#ifndef __INSIDE_CYGWIN__
  virtual int listen ();
  virtual class transport_layer_sockets *accept (bool *recoverable);
#endif

  virtual void close ();
  virtual ssize_t read (void *buf, size_t len);
  virtual ssize_t write (void *buf, size_t len);
  virtual int connect ();

  transport_layer_sockets ();
  virtual ~transport_layer_sockets ();

private:
  /* for socket based communications */
  int _fd;
  struct sockaddr_un _addr;
  socklen_t _addr_len;
  const bool _is_accepted_endpoint;
  bool _is_listening_endpoint;

  transport_layer_sockets (int fd);
};

#endif /* _CYGSERVER_TRANSPORT_SOCKETS_ */
