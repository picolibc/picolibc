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
static char *search_for = (char *) cygthread::stub;
static unsigned threadfunc_ix __attribute__((section ("cygwin_dll_common"), shared)) = 0;
DWORD tls_func;

HANDLE sync_startup;

#define OLDFUNC_OFFSET -1

static void WINAPI
threadfunc_fe (VOID *arg)
{
  _threadinfo::call ((DWORD (*)  (void *, void *)) (((char **) _tlsbase)[OLDFUNC_OFFSET]), arg);
  // void *threadfunc = (void *) TlsGetValue (tls_func);
  // TlsFree (tls_func);
  // _threadinfo::call ((DWORD (*)  (void *, void *)) (threadfunc), arg);
}

static DWORD WINAPI
calibration_thread (VOID *arg)
{
  ExitThread (0);
}

static void
munge_threadfunc (HANDLE cygwin_hmodule)
{
  char **ebp = (char **) __builtin_frame_address (0);
  if (!threadfunc_ix)
    {
      for (char **peb = ebp; peb < (char **) _tlsbase; peb++)
	if (*peb == search_for)
	  {
	    threadfunc_ix = peb - ebp;
	    goto foundit;
	  }
#ifdef DEBUGGING
      system_printf ("non-fatal warning: unknown thread! search_for %p, cygthread::stub %p, calibration_thread %p, possible func offset %p",
		     search_for, cygthread::stub, calibration_thread, ebp[137]);
#endif
      try_to_debug ();
      return;
    }

foundit:
  char *threadfunc = ebp[threadfunc_ix];
  if (threadfunc == (char *) calibration_thread)
    /* no need for the overhead */;
  else
    {
      ebp[threadfunc_ix] = (char *) threadfunc_fe;
      ((char **) _tlsbase)[OLDFUNC_OFFSET] = threadfunc;
      // TlsSetValue (tls_func, (void *) threadfunc);
    }
}

void
prime_threads ()
{
  // tls_func = TlsAlloc ();
  if (!threadfunc_ix)
    {
      DWORD id;
      search_for = (char *) calibration_thread;
      sync_startup = CreateThread (NULL, 0, calibration_thread, 0, 0, &id);
    }
}

extern void __stdcall dll_crt0_0 ();

extern "C" int WINAPI
dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      dynamically_loaded = (static_load == NULL);
      // __cygwin_user_data.impure_ptr = &_my_tls.local_clib;
      dll_crt0_0 ();
      prime_threads ();
      // small_printf ("%u, %p, %p\n", cygwin_pid (GetCurrentProcessId ()), _tlstop, _tlsbase);
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      munge_threadfunc (h);
      // small_printf ("%u, %p, %p\n", cygwin_pid (GetCurrentProcessId ()), _tlstop, _tlsbase);
      break;
    case DLL_THREAD_DETACH:
      _my_tls.remove (0);
      break;
    }
  return 1;
}
