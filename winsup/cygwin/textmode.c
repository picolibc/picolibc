/* binmode.c

   Copyright 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <sys/fcntl.h>

extern int _fmode;
void
cygwin_premain0 (int argc, char **argv)
{
  _fmode &= ~_O_BINARY;
  _fmode |= _O_TEXT;
}
