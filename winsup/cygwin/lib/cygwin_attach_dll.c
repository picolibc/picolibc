/* attach_dll.cc: crt0 for attaching cygwin DLL from a non-cygwin app.

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
#include "crt0.h"

/* for a loaded dll */
int
cygwin_attach_dll (HMODULE h, MainFunc f)
{
  _cygwin_crt0_common (f);

  /* jump into the dll. */
  return dll_dllcrt0 (h, NULL);
}
