/* binmode.c

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>

extern int _fmode;
void
cygwin_premain0 (int argc, char **argv, struct per_process *myself)
{
  _fmode &= ~_O_TEXT;
  _fmode |= _O_BINARY;
}
