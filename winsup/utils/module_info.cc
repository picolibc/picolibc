/* module_info.cc

   Copyright 1999 Cygnus Solutions.

   Written by Egor Duda <deo@logos-m.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <windows.h>
#include <psapi.h>

static int psapi_loaded = 0;
static HMODULE psapi_module_handle = NULL;

typedef BOOL  WINAPI (tf_EnumProcessModules  ) ( HANDLE, HMODULE*, DWORD, LPDWORD );
typedef BOOL  WINAPI (tf_GetModuleInformation) ( HANDLE, HMODULE, LPMODULEINFO, DWORD );
typedef DWORD WINAPI (tf_GetModuleFileNameExA) ( HANDLE, HMODULE, LPSTR, DWORD );

static tf_EnumProcessModules *psapi_EnumProcessModules = NULL;
static tf_GetModuleInformation *psapi_GetModuleInformation = NULL;
static tf_GetModuleFileNameExA *psapi_GetModuleFileNameExA = NULL;

/*
 * Returns full name of Dll, which is loaded by hProcess at BaseAddress
 * Uses psapi.dll
 */

char*
psapi_get_module_name ( HANDLE hProcess, DWORD BaseAddress )
{
  DWORD len;
  MODULEINFO mi;
  unsigned int i;
  HMODULE dh_buf [ 1 ];
  HMODULE* DllHandle = dh_buf;
  DWORD cbNeeded;
  BOOL ok;

  char name_buf [ MAX_PATH + 1 ];

  if ( !psapi_loaded ||
       psapi_EnumProcessModules   == NULL ||
       psapi_GetModuleInformation == NULL ||
       psapi_GetModuleFileNameExA == NULL )
    {
      if ( psapi_loaded ) goto failed;
      psapi_loaded = 1;
      psapi_module_handle = LoadLibrary ( "psapi.dll" );
      if ( ! psapi_module_handle )
        goto failed;
      psapi_EnumProcessModules   = (tf_EnumProcessModules *) GetProcAddress ( psapi_module_handle, "EnumProcessModules"   );
      psapi_GetModuleInformation = (tf_GetModuleInformation *) GetProcAddress ( psapi_module_handle, "GetModuleInformation" );
      psapi_GetModuleFileNameExA = (tf_GetModuleFileNameExA*) GetProcAddress ( psapi_module_handle, "GetModuleFileNameExA" );
      if ( psapi_EnumProcessModules   == NULL ||
           psapi_GetModuleInformation == NULL ||
           psapi_GetModuleFileNameExA == NULL ) goto failed;
    }

  ok = (*psapi_EnumProcessModules) ( hProcess,
				     DllHandle,
				     sizeof ( HMODULE ),
				     &cbNeeded );

  if ( !ok || !cbNeeded ) goto failed;
  DllHandle = (HMODULE*) malloc ( cbNeeded );
  if ( ! DllHandle ) goto failed;
  ok = (*psapi_EnumProcessModules) ( hProcess,
				     DllHandle,
				     cbNeeded,
				     &cbNeeded );
  if ( ! ok )
    {
      free ( DllHandle );
      goto failed;
    }

  for ( i = 0; i < cbNeeded / sizeof ( HMODULE ); i++ )
    {
      if ( ! (*psapi_GetModuleInformation) ( hProcess,
					     DllHandle [ i ],
					     &mi,
					     sizeof ( mi ) ) )
        {
          free ( DllHandle );
          goto failed;
        }

      len = (*psapi_GetModuleFileNameExA) ( hProcess,
					    DllHandle [ i ],
					    name_buf,
					    MAX_PATH );
      if ( len == 0 )
        {
          free ( DllHandle );
          goto failed;
        }

      if ( (DWORD) (mi.lpBaseOfDll) == BaseAddress )
        {
          free ( DllHandle );
          return strdup ( name_buf );
        }
    }

failed:
  return NULL;
}
