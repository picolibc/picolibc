/* init.cc for WIN32.

   Copyright 1996, 1997, 1998, 1999 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <ctype.h>
#include "winsup.h"

extern "C"
{
  int WINAPI dll_entry (HANDLE h, DWORD reason, void *ptr);
};

extern "C" void *export_malloc (unsigned int);
extern "C" void *export_realloc (void *,unsigned int);
extern "C" void *export_calloc (unsigned int,unsigned int);
extern "C" void export_free (void *);

extern void do_global_ctors (void (**in_pfunc)(), int force);

int NO_COPY dynamically_loaded;

int
WINAPI dll_entry (HANDLE, DWORD reason, void *static_load)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      dynamically_loaded = (static_load == NULL);
      break;
    case DLL_THREAD_ATTACH:
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
