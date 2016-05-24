/* transport.cc

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

#include "transport.h"
#include "transport_pipes.h"

/* The factory */
transport_layer_base *
create_server_transport ()
{
  return new transport_layer_pipes;
}

#ifndef __INSIDE_CYGWIN__

bool
transport_layer_base::impersonate_client ()
{
  return true;
}

bool
transport_layer_base::revert_to_self ()
{
  return true;
}

#endif /* !__INSIDE_CYGWIN__ */

transport_layer_base::~transport_layer_base ()
{}
