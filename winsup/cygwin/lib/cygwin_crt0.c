/* crt0.cc: crt0 for libc

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include "crt0.h"

extern void __stdcall _dll_crt0 (void)  __declspec (dllimport) __attribute ((noreturn));

/* for main module */
void
cygwin_crt0 (MainFunc f)
{
  _cygwin_crt0_common (f);

 /* Jump into the dll. */
  _dll_crt0 ();
}
