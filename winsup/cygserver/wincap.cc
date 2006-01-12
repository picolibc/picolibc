/* wincap.cc -- figure out on which OS we're running.
		Lightweight version for Cygserver

   Copyright 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "woutsup.h"

wincapc wincap;

void
wincapc::init ()
{
  memset (&version, 0, sizeof version);
  /* Request simple version info. */
  version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&version);
}
