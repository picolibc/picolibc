/* loadlib.h

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef _LOADLIB_H
#define _LOADLIB_H

#include <windows.h>
#include <wchar.h>

/* Load all system libs from the windows system directory by prepending the
   full path.  This doesn't work for loadling cygwin1.dll.  For this case,
   instead of prepending the path, make sure that the CWD is removed from
   the DLL search path, if possible (XP SP1++, Vista++). */
static HMODULE _load_sys_library (const wchar_t *dll) __attribute__ ((used));

static HMODULE
_load_sys_library (const wchar_t *dll)
{
  static BOOL WINAPI (*set_dll_directory)(LPCWSTR);
  static WCHAR sysdir[MAX_PATH];
  static UINT sysdir_len;

  WCHAR dllpath[MAX_PATH];

  if (!sysdir_len)
    {
      sysdir_len = GetSystemDirectoryW (sysdir, MAX_PATH);
      sysdir[sysdir_len++] = L'\\';
      sysdir[sysdir_len] = L'\0';
    }
  if (!set_dll_directory)
    {
      HMODULE k32 = GetModuleHandleW (L"kernel32.dll");
      if (k32)
      	set_dll_directory = (BOOL WINAPI (*)(LPCWSTR))
		     GetProcAddress (k32, "SetDllDirectoryW");
      if (!set_dll_directory)
	set_dll_directory = (BOOL WINAPI (*)(LPCWSTR)) -1;
      else
      	set_dll_directory (L"");
    }

  if (wcscmp (dll, L"cygwin1.dll") == 0)
    return LoadLibraryExW (L"cygwin1.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

  wcscpy (dllpath, sysdir);
  wcscpy (dllpath + sysdir_len, dll);
  return LoadLibraryExW (dllpath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

#define LoadLibraryW(d)	_load_sys_library(d)
#define LoadLibraryA(d)	_load_sys_library(L##d)

#endif /* _LOADLIB_H */
