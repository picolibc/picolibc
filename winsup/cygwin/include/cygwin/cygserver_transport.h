/* cygserver.cc

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _CYGSERVER_TRANSPORT_
#define _CYGSERVER_TRANSPORT_
class transport_layer_base *create_server_transport();

/* the base class does nothing. */
class transport_layer_base
{
  public:
    virtual void listen () = 0;
    virtual class transport_layer_base * accept () = 0;
    virtual void close () = 0;
    virtual ssize_t read (char *buf, size_t len) = 0;
    virtual ssize_t write (char *buf, size_t len) = 0;
    virtual bool connect() = 0;
    virtual void impersonate_client ();
    virtual void revert_to_self ();
};

#endif /* _CYGSERVER_TRANSPORT_ */
