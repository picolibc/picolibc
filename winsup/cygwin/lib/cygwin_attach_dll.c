/* attach_dll.cc: crt0 for attaching cygwin DLL from a non-cygwin app.

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef __INSIDE_CYGWIN__
#include <windows.h>
#include <sys/cygwin.h>
#include "crt0.h"

/* for a loaded dll */
int
cygwin_attach_dll (HMODULE h, MainFunc f)
{
  struct per_process *u;
  if (_cygwin_crt0_common (f, NULL))
    u = NULL;		/* Newer DLL.  Use DLL internal per_process. */
  else			/* Older DLL.  Provide a per_process */
    {
      u = (struct per_process *) alloca (sizeof (*u));
      (void) _cygwin_crt0_common (f, u);
    }

  /* jump into the dll. */
  return dll_dllcrt0 (h, u);
}
