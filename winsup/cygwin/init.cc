/* init.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "thread.h"
#include "perprocess.h"
#include "cygthread.h"
#include "cygtls.h"

int NO_COPY dynamically_loaded;

static void WINAPI
threadfunc_fe (VOID *arg)
{
  _threadinfo::call ((DWORD (*)  (void *, void *)) (((char **) _tlsbase)[-1]), arg);
}

static void
munge_threadfunc (HANDLE cygwin_hmodule)
{
  char **ebp = (char **) __builtin_frame_address (0); 
  static unsigned threadfunc_ix;
  if (!threadfunc_ix)
    {
      for (char **peb = ebp; peb < (char **) _tlsbase; peb++)
	if (*peb == (char *) cygthread::stub)
	  {
	    threadfunc_ix = peb - ebp;
	    goto foundit;
	  }
      return;
    }

foundit:
  char *threadfunc = ebp[threadfunc_ix];
  ebp[threadfunc_ix] = (char *) threadfunc_fe;
  ((char **) _tlsbase)[-1] = threadfunc;
}

extern "C" int WINAPI
dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      dynamically_loaded = (static_load == NULL);
      // __cygwin_user_data.impure_ptr = &_my_tls.local_clib;
      _my_tls.stackptr = NULL;
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      munge_threadfunc (h);
      break;
    case DLL_THREAD_DETACH:
      break;
    }
  return 1;
}
