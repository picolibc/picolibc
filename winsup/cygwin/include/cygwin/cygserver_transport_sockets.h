/* cygserver.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _CYGSERVER_TRANSPORT_SOCKETS_
#define _CYGSERVER_TRANSPORT_SOCKETS_
class transport_layer_sockets : public transport_layer_base
{
  public:
    virtual void listen ();
    virtual class transport_layer_sockets * accept ();
    virtual void close ();
    virtual ssize_t read (char *buf, size_t len);
    virtual ssize_t write (char *buf, size_t len);
    virtual bool connect();
    transport_layer_sockets ();

  private:
    /* for socket based communications */
    int fd;
    struct sockaddr sockdetails;
    int sdlen;
    transport_layer_sockets (int newfd);
};
#endif /* _CYGSERVER_TRANSPORT_SOCKETS_ */
