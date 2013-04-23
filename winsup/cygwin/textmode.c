/* binmode.c

   Copyright 2000, 2001, 2010, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winlean.h"
#include <sys/fcntl.h>
#include <sys/cygwin.h>

extern int _fmode;
void
cygwin_premain0 (int argc __attribute__ ((unused)),
		 char **argv __attribute__ ((unused)),
		 struct per_process *myself __attribute__ ((unused)))
{
  _fmode &= ~_O_BINARY;
  _fmode |= _O_TEXT;
}
