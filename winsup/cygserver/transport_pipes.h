/* transport_pipes.h

   Copyright 2001, 2002, 2003 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _TRANSPORT_PIPES_H
#define _TRANSPORT_PIPES_H

#define PIPE_NAME_PREFIX	L"\\\\.\\pipe\\cygwin-"
#define PIPE_NAME_SUFFIX	L"-lpc"

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
  virtual bool impersonate_client ();
  virtual bool revert_to_self ();
#endif

  transport_layer_pipes ();
  virtual ~transport_layer_pipes ();

private:
  wchar_t _pipe_name[40];
  HANDLE _hPipe;
  const bool _is_accepted_endpoint;
  bool _is_listening_endpoint;

  transport_layer_pipes (HANDLE hPipe);
};

#endif /* _TRANSPORT_PIPES_H */
