/* wsock_event.h: Defining the wsock_event class

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __WSOCK_EVENT_H__
#define __WSOCK_EVENT_H__

class wsock_event
{
  WSAEVENT		event;
  WSAOVERLAPPED		ovr;
public:
  wsock_event () : event (NULL) {};
  ~wsock_event ()
    {
      if (event)
	WSACloseEvent (event);
      event = NULL;
    };

  /* The methods are implemented in net.cc */
  LPWSAOVERLAPPED prepare ();
  int wait (int socket, LPDWORD flags);
};

#endif /* __WSOCK_EVENT_H__ */
