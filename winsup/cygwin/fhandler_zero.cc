/* fhandler_dev_zero.cc: code to access /dev/zero

   Copyright 2000 Cygnus Solutions.

   Written by DJ Delorie (dj@cygnus.com)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include "fhandler.h"

fhandler_dev_zero::fhandler_dev_zero (const char *name)
  : fhandler_base (FH_ZERO, name)
{
  set_cb (sizeof *this);
}

int
fhandler_dev_zero::open (const char *, int flags, mode_t)
{
  set_flags (flags);
  return 1;
}

int
fhandler_dev_zero::write (const void *, size_t len)
{
  return len;
}

int
fhandler_dev_zero::read (void *ptr, size_t len)
{
  memset(ptr, 0, len);
  return len;
}

off_t
fhandler_dev_zero::lseek (off_t, int)
{
  return 0;
}

int
fhandler_dev_zero::close (void)
{
  return 0;
}

void
fhandler_dev_zero::dump ()
{
  paranoid_printf("here, fhandler_dev_zero");
}
