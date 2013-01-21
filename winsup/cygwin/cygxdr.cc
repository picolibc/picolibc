/* cygxdr.cc:

   Copyright 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdarg.h>
#include <stdio.h>
#include "cygxdr.h"

extern "C" void
cygxdr_vwarnx (const char * fmt, va_list ap)
{
  /* Imitate glibc behavior for xdr: messages are printed to stderr */
  (void) fputs ("xdr-routines: ", stderr);
  (void) vfprintf (stderr, fmt, ap);
  (void) fputs ("\n", stderr);
}

