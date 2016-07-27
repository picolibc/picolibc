/* init.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include "ntdll.h"
#include "shared_info.h"

static DWORD _my_oldfunc;

static char *search_for  = (char *) cygthread::stub;
unsigned threadfunc_ix[8];

static bool dll_finished_loading;
#define OLDFUNC_OFFSET -1

static void WINAPI
threadfunc_fe (VOID *arg)
{
#ifndef __x86_64__
  (void)__builtin_return_address(1);
  asm volatile ("andl $-16,%%esp" ::: "%esp");
#endif
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
      char **top = (char **) NtCurrentTeb()->Tib.StackBase;
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
      char *threadfunc = NULL;

      NtQueryInformationThread (NtCurrentThread (),
				ThreadQuerySetWin32StartAddress,
				&threadfunc, sizeof threadfunc, NULL);
      if (!search_for || threadfunc == search_for)
	{
	  search_for = NULL;
	  for (i = 0; threadfunc_ix[i]; i++)
	    if (!threadfunc || ebp[threadfunc_ix[i]] == threadfunc)
	       ebp[threadfunc_ix[i]] = (char *) threadfunc_fe;
	  TlsSetValue (_my_oldfunc, threadfunc);
	}
    }
}

void dll_crt0_0 ();

/* Non-static fake variable so GCC doesn't second-guess if we *really*
   need the alloca'd space in the DLL_PROCESS_ATTACH case below... */
void *alloca_dummy;

extern "C" BOOL WINAPI
dll_entry (HANDLE h, DWORD reason, void *static_load)
{
  BOOL test_stack_marker;

  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      init_console_handler (false);

      cygwin_hmodule = (HMODULE) h;
      dynamically_loaded = (static_load == NULL);

      /* Starting with adding the POSIX-1.2008 per-thread locale functionality,
	 we need an initalized _REENT area even for the functions called from
	 dll_crt0_0.  Most importantly, we need the _REENT->_locale pointer
	 initialized to NULL, so subsequent calls to locale-specific functions
	 will always fall back to __global_locale, rather then crash due to
	 _REENT->_locale having an arbitrary value. */
      alloca_dummy = alloca (CYGTLS_PADSIZE);
      memcpy (_REENT, _GLOBAL_REENT, sizeof (struct _reent));

      dll_crt0_0 ();
      _my_oldfunc = TlsAlloc ();
      dll_finished_loading = true;
      break;
    case DLL_PROCESS_DETACH:
      if (dynamically_loaded)
	shared_destroy ();
      break;
    case DLL_THREAD_ATTACH:
      if (dll_finished_loading)
	munge_threadfunc ();
      break;
    case DLL_THREAD_DETACH:
      if (dll_finished_loading
	  && (PVOID) &_my_tls > (PVOID) &test_stack_marker
	  && _my_tls.isinitialized ())
	_my_tls.remove (0);
      break;
    }

  return TRUE;
}
