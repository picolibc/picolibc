/* init.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "thread.h"
#include "perprocess.h"
#include "cygtls.h"

int NO_COPY dynamically_loaded;

extern "C" int
WINAPI dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      _my_tls.stackptr = _my_tls.stack;
      dynamically_loaded = (static_load == NULL);
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      _my_tls.stackptr = _my_tls.stack;
      if (MT_INTERFACE->reent_key.set (&MT_INTERFACE->reents))
	    api_fatal ("thread initialization failed");
      break;
    }
  return 1;
}
