/* cygserver_transport_pipes.h

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGSERVER_TRANSPORT_PIPES_
#define _CYGSERVER_TRANSPORT_PIPES_

/* Named pipes based transport, for security on NT */
class transport_layer_pipes : public transport_layer_base
{
public:
#ifndef __INSIDE_CYGWIN__
  virtual int listen ();
  virtual class transport_layer_pipes *accept (bool *recoverable);
#endif

  virtual void close ();
  virtual ssize_t read (void *buf, size_t len);
  virtual ssize_t write (void *buf, size_t len);
  virtual int connect ();

#ifndef __INSIDE_CYGWIN__
  virtual void impersonate_client ();
  virtual void revert_to_self ();
#endif

  transport_layer_pipes ();
  virtual ~transport_layer_pipes ();

private:
  /* for pipe based communications */
  void init_security ();

  //FIXME: allow inited, sd, all_nih_.. to be static members
  SECURITY_DESCRIPTOR _sd;
  SECURITY_ATTRIBUTES _sec_all_nih;
  const char *const _pipe_name;
  HANDLE _hPipe;
  const bool _is_accepted_endpoint;
  bool _is_listening_endpoint;

  transport_layer_pipes (HANDLE hPipe);
};

#endif /* _CYGSERVER_TRANSPORT_PIPES_ */
