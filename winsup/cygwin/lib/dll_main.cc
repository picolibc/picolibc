/* dll_main.cc: Provide the DllMain stub that the user can override.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */


#include "winlean.h"

extern "C"
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason,
		       LPVOID reserved /* Not used. */);

BOOL APIENTRY
DllMain (
	 HINSTANCE hInst /* Library instance handle. */ ,
	 DWORD reason /* Reason this function is being called. */ ,
	 LPVOID reserved /* Not used. */)
{
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      break;

    case DLL_PROCESS_DETACH:
      break;

    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      break;
    }
  return TRUE;
}
