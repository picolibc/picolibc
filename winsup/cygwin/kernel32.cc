/* kernel32.cc: Win32 replacement functions.

   Copyright 2008, 2009, 2010, 2011, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "ntdll.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include "winf.h"
#include "sys/cygwin.h"

/* Implement CreateEvent/OpenEvent so that named objects are always created in
   Cygwin shared object namespace. */

HANDLE WINAPI
CreateEventW (LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset,
	      BOOL bInitialState, LPCWSTR lpName)
{
  HANDLE evt;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (lpEventAttributes && lpEventAttributes->bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_OPENIF | OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      lpEventAttributes
			      ? lpEventAttributes->lpSecurityDescriptor : NULL);
  status = NtCreateEvent (&evt, EVENT_ALL_ACCESS, &attr,
			  bManualReset ? NotificationEvent
				       : SynchronizationEvent,
			  bInitialState);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  SetLastError (status == STATUS_OBJECT_NAME_EXISTS
		? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
  return evt;
}

HANDLE WINAPI
CreateEventA (LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset,
	      BOOL bInitialState, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return CreateEventW (lpEventAttributes, bManualReset, bInitialState,
		       lpName ? name : NULL);
}

HANDLE WINAPI
OpenEventW (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
  HANDLE evt;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      NULL);
  status = NtOpenEvent (&evt, dwDesiredAccess, &attr);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  return evt;
}

HANDLE WINAPI
OpenEventA (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return OpenEventW (dwDesiredAccess, bInheritHandle, lpName ? name : NULL);
}

/* Implement CreateMutex/OpenMutex so that named objects are always created in
   Cygwin shared object namespace. */

HANDLE WINAPI
CreateMutexW (LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner,
	      LPCWSTR lpName)
{
  HANDLE mtx;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (lpMutexAttributes && lpMutexAttributes->bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_OPENIF | OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      lpMutexAttributes
			      ? lpMutexAttributes->lpSecurityDescriptor : NULL);
  status = NtCreateMutant (&mtx, MUTEX_ALL_ACCESS, &attr, bInitialOwner);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  SetLastError (status == STATUS_OBJECT_NAME_EXISTS
		? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
  return mtx;
}

HANDLE WINAPI
CreateMutexA (LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner,
	      LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return CreateMutexW (lpMutexAttributes, bInitialOwner, lpName ? name : NULL);
}

HANDLE WINAPI
OpenMutexW (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
  HANDLE mtx;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      NULL);
  status = NtOpenMutant (&mtx, dwDesiredAccess, &attr);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  return mtx;
}

HANDLE WINAPI
OpenMutexA (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return OpenMutexW (dwDesiredAccess, bInheritHandle, lpName ? name : NULL);
}

/* Implement CreateSemaphore/OpenSemaphore so that named objects are always
   created in Cygwin shared object namespace. */

HANDLE WINAPI
CreateSemaphoreW (LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		  LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName)
{
  HANDLE sem;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (lpSemaphoreAttributes && lpSemaphoreAttributes->bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_OPENIF | OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      lpSemaphoreAttributes
			      ? lpSemaphoreAttributes->lpSecurityDescriptor
			      : NULL);
  status = NtCreateSemaphore (&sem, SEMAPHORE_ALL_ACCESS, &attr,
			      lInitialCount, lMaximumCount);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  SetLastError (status == STATUS_OBJECT_NAME_EXISTS
		? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
  return sem;
}

HANDLE WINAPI
CreateSemaphoreA (LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
		  LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return CreateSemaphoreW (lpSemaphoreAttributes, lInitialCount, lMaximumCount,
			   lpName ? name : NULL);
}

HANDLE WINAPI
OpenSemaphoreW (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
  HANDLE sem;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      NULL);
  status = NtOpenSemaphore (&sem, dwDesiredAccess, &attr);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  return sem;
}

HANDLE WINAPI
OpenSemaphoreA (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return OpenSemaphoreW (dwDesiredAccess, bInheritHandle, lpName ? name : NULL);
}

/* Implement CreateFileMapping/OpenFileMapping so that named objects are always
   created in Cygwin shared object namespace. */

HANDLE WINAPI
CreateFileMappingW (HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes,
		    DWORD flProtect, DWORD dwMaximumSizeHigh,
		    DWORD dwMaximumSizeLow, LPCWSTR lpName)
{
  HANDLE sect;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;
  ACCESS_MASK access = READ_CONTROL | SECTION_QUERY | SECTION_MAP_READ;
  ULONG prot = flProtect & (PAGE_NOACCESS | PAGE_READONLY | PAGE_READWRITE
			    | PAGE_WRITECOPY | PAGE_EXECUTE
			    | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE
			    | PAGE_EXECUTE_WRITECOPY);
  ULONG attribs = flProtect & (SEC_COMMIT | SEC_IMAGE | SEC_NOCACHE
			       | SEC_RESERVE);
  LARGE_INTEGER size = {{ LowPart  : dwMaximumSizeLow,
			  HighPart : (LONG) dwMaximumSizeHigh }};
  PLARGE_INTEGER psize = size.QuadPart ? &size : NULL;

  if (prot & (PAGE_READWRITE | PAGE_WRITECOPY
	      | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
    access |= SECTION_MAP_WRITE;
  if (prot & (PAGE_EXECUTE | PAGE_EXECUTE_READ
	      | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
    access |= SECTION_MAP_EXECUTE;
  if (lpAttributes && lpAttributes->bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_OPENIF | OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      lpAttributes
			      ? lpAttributes->lpSecurityDescriptor
			      : NULL);
  if (!(attribs & (SEC_RESERVE | SEC_COMMIT)))
    attribs |= SEC_COMMIT;
  if (hFile == INVALID_HANDLE_VALUE)
    hFile = NULL;
  status = NtCreateSection (&sect, access, &attr, psize, prot, attribs, hFile);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  SetLastError (status == STATUS_OBJECT_NAME_EXISTS
		? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
  return sect;
}

HANDLE WINAPI
CreateFileMappingA (HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes,
		    DWORD flProtect, DWORD dwMaximumSizeHigh,
		    DWORD dwMaximumSizeLow, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return CreateFileMappingW (hFile, lpAttributes, flProtect, dwMaximumSizeHigh,
			     dwMaximumSizeLow, lpName ? name : NULL);
}

HANDLE WINAPI
OpenFileMappingW (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
  HANDLE sect;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  ULONG flags = 0;

  if (bInheritHandle)
    flags |= OBJ_INHERIT;
  if (lpName)
    {
      RtlInitUnicodeString (&uname, lpName);
      flags |= OBJ_CASE_INSENSITIVE;
    }
  InitializeObjectAttributes (&attr, lpName ? &uname : NULL, flags,
			      lpName ? get_shared_parent_dir () : NULL,
			      NULL);
  status = NtOpenSection (&sect, dwDesiredAccess, &attr);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return NULL;
    }
  return sect;
}

HANDLE WINAPI
OpenFileMappingA (DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
  WCHAR name[MAX_PATH];

  if (lpName && !sys_mbstowcs (name, MAX_PATH, lpName))
    {
      SetLastError (ERROR_FILENAME_EXCED_RANGE);
      return NULL;
    }
  return OpenFileMappingW (dwDesiredAccess, bInheritHandle, lpName ? name : NULL);
}

/* The external functions below wrap Windows functions of the same name
   and provide a Windows interface to Cygwin functionality.  */

/* Construct a unicode version of the Cygwin command line from __argv) */
static UNICODE_STRING *
ucmd ()
{
  static UNICODE_STRING wcmd;
  if (!wcmd.Buffer)
    {
      linebuf cmd;
      path_conv real_path (__argv[0]);
      av newargv (__argc, __argv);
      cmd.fromargv (newargv, real_path.get_win32 (), true);
      RtlInitUnicodeString (&wcmd, cmd);
    }
  return &wcmd;
}

/* Cygwin replacement for GetCommandLineW.  Returns a concatenated wide string
   representing the argv list, constructed using roughly the same mechanism as
   child_info_spawn::worker */
extern "C" LPWSTR WINAPI
cygwin_GetCommandLineW (void)
{
  return ucmd ()->Buffer;
}

/* Cygwin replacement for GetCommandLineA.  Returns a concatenated string
   representing the argv list, constructed using roughly the same mechanism
   as child_info_spawn::worker */
extern "C" LPSTR WINAPI
cygwin_GetCommandLineA (void)
{
  static ANSI_STRING cmd;
  if (!cmd.Buffer)
    RtlUnicodeStringToAnsiString (&cmd, ucmd (), TRUE);
  return cmd.Buffer;
}
