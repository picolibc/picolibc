/* cygserver_transport.cc

   Copyright 2001, 2002 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#else
#include "winsup.h"
#endif

#include <sys/socket.h>

#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_pipes.h"
#include "cygwin/cygserver_transport_sockets.h"

/* The factory */
class transport_layer_base *create_server_transport()
{
  transport_layer_base *temp;
  /* currently there is only the base class! */
  if (wincap.is_winnt ())
    temp = new transport_layer_pipes ();
  else
    temp = new transport_layer_sockets ();
  return temp;
}

void
transport_layer_base::impersonate_client ()
{
}

void
transport_layer_base::revert_to_self ()
{
}
