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
#include "cygtls.h"
#include "pinfo.h"

int NO_COPY dynamically_loaded;
static char *search_for = (char *) cygthread::stub;
unsigned threadfunc_ix[8] __attribute__((section ("cygwin_dll_common"), shared));
DWORD tls_func;

HANDLE sync_startup;

#define OLDFUNC_OFFSET -1

static void WINAPI
threadfunc_fe (VOID *arg)
{
  (void)__builtin_return_address(1);
  asm volatile ("andl $-16,%%esp" ::: "%esp");
  _cygtls::call ((DWORD (*)  (void *, void *)) (((char **) _tlsbase)[OLDFUNC_OFFSET]), arg);
}

static DWORD WINAPI
calibration_thread (VOID *arg)
{
  ExitThread (0);
}

/* We need to know where the OS stores the address of the thread function
   on the stack so that we can intercept the call and insert some tls
   stuff on the stack.  This function starts a known calibration thread.
   When it starts, a call will be made to dll_entry which will call munge_threadfunc
   looking for the calibration thread offset on the stack.  This offset will
   be stored and used by all executing cygwin processes. */
void
prime_threads ()
{
  if (!threadfunc_ix[0])
    {
      DWORD id;
      search_for = (char *) calibration_thread;
      sync_startup = CreateThread (NULL, 0, calibration_thread, 0, 0, &id);
    }
}

/* If possible, redirect the thread entry point to a cygwin routine which
   adds tls stuff to the stack. */
static void
munge_threadfunc ()
{
  int i;
  char **ebp = (char **) __builtin_frame_address (0);
  if (!threadfunc_ix[0])
    {
      char **peb;
      char **top = (char **) _tlsbase;
      for (peb = ebp, i = 0; peb < top && i < 7; peb++)
	if (*peb == search_for)
	  threadfunc_ix[i++] = peb - ebp;
      if (!threadfunc_ix[0])
	{
	  try_to_debug ();
	  return;
	}
    }

  char *threadfunc = ebp[threadfunc_ix[0]];
  if (threadfunc == (char *) calibration_thread)
    /* no need for the overhead */;
  else
    {
      for (i = 0; threadfunc_ix[i]; i++)
	ebp[threadfunc_ix[i]] = (char *) threadfunc_fe;
      ((char **) _tlsbase)[OLDFUNC_OFFSET] = threadfunc;
    }
}

static void __attribute__ ((noreturn))
respawn_wow64_process ()
{
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  GetStartupInfo (&si);
  if (!CreateProcessA (NULL, GetCommandLineA (), NULL, NULL, TRUE,
		       CREATE_DEFAULT_ERROR_MODE
		       | GetPriorityClass (GetCurrentProcess ()),
		       NULL, NULL, &si, &pi))
    api_fatal ("Failed to create process <%s>, %E", GetCommandLineA ());
  CloseHandle (pi.hThread);
  if (WaitForSingleObject (pi.hProcess, INFINITE) == WAIT_FAILED)
    api_fatal ("Waiting for process %d failed, %E", pi.dwProcessId);
  CloseHandle (pi.hProcess);
  ExitProcess (0);
}

extern void __stdcall dll_crt0_0 ();

extern "C" int WINAPI
dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  BOOL is_64bit_machine = FALSE;
  extern HANDLE hExeced;

  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      /* Is the stack at an unusual high address?  Check if we're running on
	 a 64 bit machine.  If so, respawn. */
      if (&is_64bit_machine >= (PBOOL) 0x400000
	  && IsWow64Process (hMainProc, &is_64bit_machine)
	  && is_64bit_machine)
	respawn_wow64_process ();

      prime_threads ();
      dynamically_loaded = (static_load == NULL);
      dll_crt0_0 ();
      break;
    case DLL_PROCESS_DETACH:
      if (myself)
	{
	  if (!hExeced && myself->exitcode == EXITCODE_UNSET)
	    myself->exitcode = 1 << 8;
	  myself.release ();
	}
      break;
    case DLL_THREAD_ATTACH:
      munge_threadfunc ();
      break;
    case DLL_THREAD_DETACH:
      _my_tls.remove (0);
      break;
    }
  return 1;
}
