/*
 * dllmain.c
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * A stub DllMain function which will be called by DLLs which do not
 * have a user supplied DllMain.
 *
 */

#include <windows.h>

BOOL WINAPI 
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  return TRUE;
}

