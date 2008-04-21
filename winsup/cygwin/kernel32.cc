/* kernel32.cc: Win32 replacement functions.

   Copyright 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "shared_info.h"
#include "ntdll.h"

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
  status = NtCreateEvent (&evt, CYG_EVENT_ACCESS, &attr,
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
  status = NtCreateMutant (&mtx, CYG_EVENT_ACCESS, &attr, bInitialOwner);
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
  status = NtCreateSemaphore (&sem, CYG_EVENT_ACCESS, &attr,
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
  ULONG attribs = flProtect & (SEC_BASED | SEC_NO_CHANGE | SEC_IMAGE | SEC_VLM
			       | SEC_RESERVE | SEC_COMMIT | SEC_NOCACHE);
  LARGE_INTEGER size = {{ LowPart  : dwMaximumSizeLow,
			  HighPart : dwMaximumSizeHigh }};
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
  if (!attribs)
    attribs = SEC_COMMIT;
  if (hFile == INVALID_HANDLE_VALUE)
    hFile = NULL;
  status = NtCreateSection (&sect, access, &attr, psize, prot, attribs, hFile);
  if (!NT_SUCCESS (status))
    {
      small_printf ("status %p\n", status);
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
