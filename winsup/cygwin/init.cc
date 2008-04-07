/* init.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include "ntdll.h"

static DWORD _my_oldfunc;

int NO_COPY dynamically_loaded;
static char NO_COPY *search_for = (char *) cygthread::stub;
unsigned threadfunc_ix[8] __attribute__((section (".cygwin_dll_common"), shared));
extern cygthread *hwait_sig;

#define OLDFUNC_OFFSET -1

static void WINAPI
threadfunc_fe (VOID *arg)
{
  (void)__builtin_return_address(1);
  asm volatile ("andl $-16,%%esp" ::: "%esp");
  _cygtls::call ((DWORD (*)  (void *, void *)) TlsGetValue (_my_oldfunc), arg);
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
      if (0 && !threadfunc_ix[0])
	{
	  try_to_debug ();
	  return;
	}
    }

  if (threadfunc_ix[0])
    {
      char *threadfunc = ebp[threadfunc_ix[0]];
      if (!search_for || threadfunc == search_for)
	{
	  search_for = NULL;
	  for (i = 0; threadfunc_ix[i]; i++)
	    ebp[threadfunc_ix[i]] = (char *) threadfunc_fe;
	  TlsSetValue (_my_oldfunc, threadfunc);
	}
    }
}

inline static void
respawn_wow64_process ()
{
  NTSTATUS ret;
  PROCESS_BASIC_INFORMATION pbi;
  HANDLE parent;

  BOOL is_wow64_proc = TRUE;	/* Opt on the safe side. */

  /* Unfortunately there's no simpler way to retrieve the
     parent process in NT, as far as I know.  Hints welcome. */
  ret = NtQueryInformationProcess (GetCurrentProcess (),
				   ProcessBasicInformation,
				   (PVOID) &pbi,
				   sizeof pbi, NULL);
  if (ret == STATUS_SUCCESS
      && (parent = OpenProcess (PROCESS_QUERY_INFORMATION,
				FALSE,
				pbi.InheritedFromUniqueProcessId)))
    {
      IsWow64Process (parent, &is_wow64_proc);
      CloseHandle (parent);
    }

  /* The parent is a real 64 bit process?  Respawn! */
  if (!is_wow64_proc)
    {
      PROCESS_INFORMATION pi;
      STARTUPINFOW si;
      DWORD ret = 0;

      GetStartupInfoW (&si);
      if (!CreateProcessW (NULL, GetCommandLineW (), NULL, NULL, TRUE,
			   CREATE_DEFAULT_ERROR_MODE
			   | GetPriorityClass (GetCurrentProcess ()),
			   NULL, NULL, &si, &pi))
	api_fatal ("Failed to create process <%s>, %E", GetCommandLineA ());
      CloseHandle (pi.hThread);
      if (WaitForSingleObject (pi.hProcess, INFINITE) == WAIT_FAILED)
	api_fatal ("Waiting for process %d failed, %E", pi.dwProcessId);
      GetExitCodeProcess (pi.hProcess, &ret);
      CloseHandle (pi.hProcess);
      ExitProcess (ret);
    }
}

extern void __stdcall dll_crt0_0 ();

HMODULE NO_COPY cygwin_hmodule;

extern "C" BOOL WINAPI
dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  BOOL wow64_test_stack_marker;

  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      wincap.init ();
      init_console_handler (false);

      cygwin_hmodule = (HMODULE) h;
      dynamically_loaded = (static_load == NULL);

      /* Is the stack at an unusual address?  This is, an address which
	 is in the usual space occupied by the process image, but below
	 the auto load address of DLLs?
	 Check if we're running in WOW64 on a 64 bit machine *and* are
	 spawned by a genuine 64 bit process.  If so, respawn. */
      if (wincap.is_wow64 ()
	  && &wow64_test_stack_marker >= (PBOOL) 0x400000
	  && &wow64_test_stack_marker <= (PBOOL) 0x10000000)
	respawn_wow64_process ();

      dll_crt0_0 ();
      _my_oldfunc = TlsAlloc ();
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      if (hwait_sig)
	munge_threadfunc ();
      break;
    case DLL_THREAD_DETACH:
      if (hwait_sig && (void *) &_my_tls > (void *) &wow64_test_stack_marker
	  && _my_tls.isinitialized ())
	_my_tls.remove (0);
      break;
    }

  return TRUE;
}
