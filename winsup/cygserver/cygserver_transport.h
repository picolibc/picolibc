/* cygserver_transport.h

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_TRANSPORT_
#define _CYGSERVER_TRANSPORT_

class transport_layer_base *create_server_transport ();

class transport_layer_base
{
public:
#ifndef __INSIDE_CYGWIN__
  virtual int listen () = 0;
  virtual class transport_layer_base *accept (bool *recoverable) = 0;
#endif

  virtual void close () = 0;
  virtual ssize_t read (void *buf, size_t len) = 0;
  virtual ssize_t write (void *buf, size_t len) = 0;
  virtual int connect () = 0;

#ifndef __INSIDE_CYGWIN__
  virtual void impersonate_client ();
  virtual void revert_to_self ();
#endif

  virtual ~transport_layer_base ();
};

#endif /* _CYGSERVER_TRANSPORT_ */
