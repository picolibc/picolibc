/*
 * mthr_init.c
 *
 * Do the thread-support DLL initialization.
 *
 * This file is used iff the following conditions are met:
 *  - gcc uses -mthreads option 
 *  - user code uses C++ exceptions
 *
 * The sole job of the Mingw thread support DLL (MingwThr) is to catch 
 * all the dying threads and clean up the data allocated in the TLSs 
 * for exception contexts during C++ EH. Posix threads have key dtors, 
 * but win32 TLS keys do not, hence the magic. Without this, there's at 
 * least `24 * sizeof (void*)' bytes leaks for each catch/throw in each
 * thread.
 * 
 * See mthr.c for all the magic.
 *  
 * Created by Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <stdio.h>

BOOL APIENTRY DllMain (HANDLE hDllHandle, DWORD reason, 
                       LPVOID reserved /* Not used. */ );

/*
 *----------------------------------------------------------------------
 *
 * DllMain --
 *
 *	This routine is called by the Mingw32, Cygwin32 or VC++ C run 
 *	time library init code, or the Borland DllEntryPoint routine. It 
 *	is responsible for initializing various dynamically loaded 
 *	libraries.
 *
 * Results:
 *      TRUE on sucess, FALSE on failure.
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */
BOOL APIENTRY
DllMain (HANDLE hDllHandle /* Library instance handle. */,
	 DWORD reason /* Reason this function is being called. */,
	 LPVOID reserved /* Not used. */)
{

  extern CRITICAL_SECTION __mingwthr_cs;
  extern void __mingwthr_run_key_dtors( void );

#ifdef DEBUG
  printf ("%s: reason %d\n", __FUNCTION__, reason );
#endif

  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
       InitializeCriticalSection (&__mingwthr_cs);
       break;

    case DLL_PROCESS_DETACH:
      __mingwthr_run_key_dtors();
       DeleteCriticalSection (&__mingwthr_cs);
      break;

    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      __mingwthr_run_key_dtors();
      break;
    }
  return TRUE;
}
