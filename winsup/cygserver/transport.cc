/* cygserver_transport.cc

   Copyright 2001 Red Hat Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "wincap.h"
#include "cygwin/cygserver_transport.h"
#include "cygwin/cygserver_transport_pipes.h"
#include "cygwin/cygserver_transport_sockets.h"

/* to allow this to link into cygwin and the .dll, a little magic is needed. */
#ifndef __OUTSIDE_CYGWIN__
#include "winsup.h"
#else
#define debug_printf printf
#endif

/* The factory */
class transport_layer_base *create_server_transport()
{
  transport_layer_base *temp;
  /* currently there is only the base class! */
  if (wincap.is_winnt ())
    temp = new transport_layer_pipes ();
  else
    temp = new transport_layer_base ();
  return temp;
}


transport_layer_base::transport_layer_base ()
{
  /* should we throw an error of some sort ? */
}

void
transport_layer_base::listen ()
{
}

class transport_layer_base *
transport_layer_base::accept ()
{
  return NULL;
}

void
transport_layer_base::close()
{
}

ssize_t
transport_layer_base::read (char *buf, size_t len)
{
  return 0;
}

ssize_t
transport_layer_base::write (char *buf, size_t len)
{
  return 0;
}

bool
transport_layer_base::connect ()
{
  return false;
}

void
transport_layer_base::impersonate_client ()
{
}

void
transport_layer_base::revert_to_self ()
{
}
