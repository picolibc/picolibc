/* cygwin_crt0.cc: crt0 for cygwin

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef __INSIDE_CYGWIN__
#include <windows.h>
#include <sys/cygwin.h>
#include "crt0.h"

extern void dll_crt0__FP11per_process (struct per_process *)  __declspec (dllimport) __attribute ((noreturn));

/* for main module */
void
cygwin_crt0 (MainFunc f)
{
  struct per_process *u;
  if (_cygwin_crt0_common (f, NULL))
    u = NULL;		/* Newer DLL.  Use DLL internal per_process. */
  else			/* Older DLL.  Provide a per_process */
    {
      u = (struct per_process *) alloca (sizeof (*u));
      memset (u, 0, sizeof (u));
      (void) _cygwin_crt0_common (f, u);
    }
  dll_crt0__FP11per_process (u); 	/* Jump into the dll, never to return */
}
