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
public:
  wsock_event () : event (NULL) {};
  ~wsock_event ()
    {
      if (event)
	WSACloseEvent (event);
      event = NULL;
    };

  /* The methods are implemented in net.cc */
  bool prepare (int sock, long event_mask);
  int wait (int sock, int &closed);
  void release (int sock);
};

#endif /* __WSOCK_EVENT_H__ */
