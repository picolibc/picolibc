/* init.cc for WIN32.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <ctype.h>
#include "winsup.h"

extern HMODULE cygwin_hmodule;

int NO_COPY dynamically_loaded;

extern "C" int
WINAPI dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      cygwin_hmodule = (HMODULE) h;
      dynamically_loaded = (static_load == NULL);
      break;
    case DLL_THREAD_ATTACH:
      if (user_data->threadinterface)
	{
	  if ( !TlsSetValue(user_data->threadinterface->reent_index,
                    &user_data->threadinterface->reents))
	    api_fatal("Sig proc MT init failed\n");
	}
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_DETACH:
#if 0 // FIXME: REINSTATE SOON
      waitq *w;
      if ((w = waitq_storage.get ()) != NULL)
	{
	  if (w->thread_ev != NULL)
	    {
	      system_printf ("closing %p", w->thread_ev);
	      (void) CloseHandle (w->thread_ev);
	    }
	  memset (w, 0, sizeof(*w));	// FIXME: memory leak
	}
	// FIXME: Need to add other per_thread stuff here
#endif
      break;
    }
  return 1;
}
