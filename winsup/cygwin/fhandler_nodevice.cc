/* fhandler_nodevice.cc.  "No such device" handler.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"

int
fhandler_nodevice::open (int, mode_t)
{
  if (!pc.error)
    set_errno (ENXIO);
  return 0;
}

fhandler_nodevice::fhandler_nodevice ()
{
}
