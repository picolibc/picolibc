/* _cygwin_S_IEXEC.cc: stat helper stuff

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

extern "C" {
#include "winsup.h"
#include <sys/stat.h>
#include <sys/unistd.h>

unsigned _cygwin_S_IEXEC = S_IEXEC;
unsigned _cygwin_S_IXUSR = S_IXUSR;
unsigned _cygwin_S_IXGRP = S_IXGRP;
unsigned _cygwin_S_IXOTH = S_IXOTH;
unsigned _cygwin_X_OK = X_OK;
};
