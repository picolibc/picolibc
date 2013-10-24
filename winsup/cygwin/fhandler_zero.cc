/* fhandler_dev_zero.cc: code to access /dev/zero

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2013 Red Hat, Inc.

   Written by DJ Delorie (dj@cygnus.com)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "security.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"

fhandler_dev_zero::fhandler_dev_zero ()
  : fhandler_base ()
{
}

ssize_t __stdcall
fhandler_dev_zero::write (const void *, size_t len)
{
  if (get_device () == FH_FULL)
    {
      set_errno (ENOSPC);
      return -1;
    }
  return len;
}

void __reg3
fhandler_dev_zero::read (void *ptr, size_t& len)
{
  memset (ptr, 0, len);
}
