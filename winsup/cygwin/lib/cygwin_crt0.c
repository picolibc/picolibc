/* cygwin_crt0.c: crt0 for cygwin

   Copyright 2000, 2001, 2007, 2008, 2009, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef __INSIDE_CYGWIN__
#include "crt0.h"

extern void __stdcall _dll_crt0 ()
  __declspec (dllimport) __attribute__ ((noreturn));

/* for main module */
void
cygwin_crt0 (MainFunc f)
{
  _cygwin_crt0_common (f, NULL);
  _dll_crt0 ();	/* Jump into the dll, never to return */
}
