/* dll_entry.cc: Provide the default user DLL linker entry point.

   Copyright 1998, 2000, 2001 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Here we simply instantiate the DECLARE_CYGWIN_DLL to define the
   linker entry point, __cygwin_dll_entry@12, which in turn calls
   _DllMain@12 to do user-specific initialization, if any. There is a
   default DllMain stub in the library if there is no user supplied
   one. */

#include "cygwin/cygwin_dll.h"

DECLARE_CYGWIN_DLL (DllMain);
