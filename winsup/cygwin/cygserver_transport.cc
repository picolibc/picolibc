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

#include "safe_memory.h"

#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_pipes.h"
#include "cygwin/cygserver_transport_sockets.h"

/* The factory */
transport_layer_base *
create_server_transport ()
{
  if (wincap.is_winnt ())
    return safe_new0 (transport_layer_pipes);
  else
    return safe_new0 (transport_layer_sockets);
}

#ifndef __INSIDE_CYGWIN__

void
transport_layer_base::impersonate_client ()
{}

void
transport_layer_base::revert_to_self ()
{}

#endif /* !__INSIDE_CYGWIN__ */

transport_layer_base::~transport_layer_base ()
{}
