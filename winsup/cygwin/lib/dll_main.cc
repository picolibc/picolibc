/* dll_main.cc: Provide the DllMain stub that the user can override.

   Copyright 1998, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <stdio.h>

extern "C"
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason,
                       LPVOID reserved /* Not used. */ );

BOOL APIENTRY
DllMain (
	 HINSTANCE hInst /* Library instance handle. */ ,
	 DWORD reason /* Reason this function is being called. */ ,
	 LPVOID reserved /* Not used. */ )
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
