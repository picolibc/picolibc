/* transport.h

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _TRANSPORT_H
#define _TRANSPORT_H

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
  virtual bool impersonate_client ();
  virtual bool revert_to_self ();
#endif

  virtual ~transport_layer_base ();
};

#endif /* _TRANSPORT_H */
