/* wsa_event.h: type definition of a wsock event storage structure.

   Copyright 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WSA_EVENT_H_
#define _WSA_EVENT_H_

/* All Cygwin processes together can share 2048 sockets. */
#define NUM_SOCKS       (32768 / sizeof (wsa_event))
  
struct wsa_event 
{
  LONG serial_number;
  long events;
  int  connect_errorcode;
  pid_t owner;
};  

#endif /* _WSA_EVENT_H_ */
