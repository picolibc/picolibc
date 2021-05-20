/* module_info.cc

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <windows.h>
#define PSAPI_VERSION 1
#include <psapi.h>
#include "loadlib.h"

/* Returns full name of Dll, which is loaded by hProcess at BaseAddress.
   Uses psapi.dll. */

char *
psapi_get_module_name (HANDLE hProcess, LPVOID BaseAddress)
{
  DWORD len;
  MODULEINFO mi;
  unsigned int i;
  HMODULE dh_buf[1];
  HMODULE *DllHandle = dh_buf;
  DWORD cbNeeded;
  BOOL ok;

  char name_buf[MAX_PATH + 1];

  ok = EnumProcessModules (hProcess, DllHandle, sizeof (HMODULE), &cbNeeded);

  if (!ok || !cbNeeded)
    goto failed;
  DllHandle = (HMODULE *) malloc (cbNeeded);
  if (!DllHandle)
    goto failed;
  ok = EnumProcessModules (hProcess, DllHandle, cbNeeded, &cbNeeded);
  if (!ok)
    {
      free (DllHandle);
      goto failed;
    }

  for (i = 0; i < cbNeeded / sizeof (HMODULE); i++)
    {
      if (!GetModuleInformation (hProcess, DllHandle[i], &mi, sizeof (mi)))
	{
	  free (DllHandle);
	  goto failed;
	}

      len = GetModuleFileNameExA (hProcess, DllHandle[i], name_buf, MAX_PATH);
      if (len == 0)
	{
	  free (DllHandle);
	  goto failed;
	}

      if (mi.lpBaseOfDll == BaseAddress)
	{
	  free (DllHandle);
	  return strdup (name_buf);
	}
    }

failed:
  return NULL;
}
