/*
	psapi.h - Include file for PSAPI.DLL APIs

	Written by Mumit Khan <khan@nanotech.wisc.edu>

	This file is part of a free library for the Win32 API.

	NOTE: This strictly does not belong in the Win32 API since it's
	really part of Platform SDK. However,GDB needs it and we might
	as well provide it here.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/
#ifndef _PSAPI_H
#define _PSAPI_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED

typedef struct _MODULEINFO {
	LPVOID lpBaseOfDll;
	DWORD SizeOfImage;
	LPVOID EntryPoint;
} MODULEINFO,*LPMODULEINFO;

typedef struct _PSAPI_WS_WATCH_INFORMATION {
	LPVOID FaultingPc;
	LPVOID FaultingVa;
} PSAPI_WS_WATCH_INFORMATION,*PPSAPI_WS_WATCH_INFORMATION;

typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	DWORD PeakWorkingSetSize;
	DWORD WorkingSetSize;
	DWORD QuotaPeakPagedPoolUsage;
	DWORD QuotaPagedPoolUsage;
	DWORD QuotaPeakNonPagedPoolUsage;
	DWORD QuotaNonPagedPoolUsage;
	DWORD PagefileUsage;
	DWORD PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS,*PPROCESS_MEMORY_COUNTERS;

typedef struct _PROCESS_MEMORY_COUNTERS_EX {
	DWORD cb;
	DWORD PageFaultCount;
	DWORD PeakWorkingSetSize;
	DWORD WorkingSetSize;
	DWORD QuotaPeakPagedPoolUsage;
	DWORD QuotaPagedPoolUsage;
	DWORD QuotaPeakNonPagedPoolUsage;
	DWORD QuotaNonPagedPoolUsage;
	DWORD PagefileUsage;
	DWORD PeakPagefileUsage;
	DWORD PrivateUsage;
} PROCESS_MEMORY_COUNTERS_EX,*PPROCESS_MEMORY_COUNTERS_EX;

typedef struct _PERFORMANCE_INFORMATION {
  DWORD cb;
  DWORD CommitTotal;
  DWORD CommitLimit;
  DWORD CommitPeak;
  DWORD PhysicalTotal;
  DWORD PhysicalAvailable;
  DWORD SystemCache;
  DWORD KernelTotal;
  DWORD KernelPaged;
  DWORD KernelNonpaged;
  DWORD PageSize;
  DWORD HandleCount;
  DWORD ProcessCount;
  DWORD ThreadCount;
} PERFORMANCE_INFORMATION, *PPERFORMANCE_INFORMATION;

typedef union _PSAPI_WORKING_SET_BLOCK {
  ULONG_PTR Flags;
  struct {
    ULONG_PTR Protection  :5;
    ULONG_PTR ShareCount  :3;
    ULONG_PTR Shared  :1;
    ULONG_PTR Reserved  :3;
    ULONG_PTR VirtualPage  :20;
  } ;
} PSAPI_WORKING_SET_BLOCK, *PPSAPI_WORKING_SET_BLOCK;

typedef struct _PSAPI_WORKING_SET_INFORMATION {
  ULONG_PTR               NumberOfEntries;
  PSAPI_WORKING_SET_BLOCK WorkingSetInfo[1];
} PSAPI_WORKING_SET_INFORMATION, *PPSAPI_WORKING_SET_INFORMATION;

/* Grouped by application,not in alphabetical order. */
BOOL WINAPI EnumProcesses(DWORD *,DWORD,DWORD *);
BOOL WINAPI EnumProcessModules(HANDLE,HMODULE *,DWORD,LPDWORD);
DWORD WINAPI GetModuleBaseNameA(HANDLE,HMODULE,LPSTR,DWORD);
DWORD WINAPI GetModuleBaseNameW(HANDLE,HMODULE,LPWSTR,DWORD);
DWORD WINAPI GetModuleFileNameExA(HANDLE,HMODULE,LPSTR,DWORD);
DWORD WINAPI GetModuleFileNameExW(HANDLE,HMODULE,LPWSTR,DWORD);
BOOL WINAPI GetModuleInformation(HANDLE,HMODULE,LPMODULEINFO,DWORD);
BOOL WINAPI EmptyWorkingSet(HANDLE);
BOOL WINAPI QueryWorkingSet(HANDLE,PVOID,DWORD);
BOOL WINAPI InitializeProcessForWsWatch(HANDLE);
BOOL WINAPI GetWsChanges(HANDLE,PPSAPI_WS_WATCH_INFORMATION,DWORD);
DWORD WINAPI GetMappedFileNameW(HANDLE,LPVOID,LPWSTR,DWORD);
DWORD WINAPI GetMappedFileNameA(HANDLE,LPVOID,LPSTR,DWORD);
BOOL WINAPI EnumDeviceDrivers(LPVOID *,DWORD,LPDWORD);
DWORD WINAPI GetDeviceDriverBaseNameA(LPVOID,LPSTR,DWORD);
DWORD WINAPI GetDeviceDriverBaseNameW(LPVOID,LPWSTR,DWORD);
DWORD WINAPI GetDeviceDriverFileNameA(LPVOID,LPSTR,DWORD);
DWORD WINAPI GetDeviceDriverFileNameW(LPVOID,LPWSTR,DWORD);
BOOL WINAPI GetProcessMemoryInfo(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD);
BOOL WINAPI GetPerformanceInfo(PPERFORMANCE_INFORMATION,DWORD);
#if (_WIN32_WINNT >= 0x0501)
DWORD WINAPI GetProcessImageFileNameA(HANDLE,LPSTR,DWORD);
DWORD WINAPI GetProcessImageFileNameW(HANDLE,LPWSTR,DWORD);
#endif

#endif /* not RC_INVOKED */

#ifdef UNICODE
#define GetModuleBaseName GetModuleBaseNameW
#define GetModuleFileNameEx GetModuleFileNameExW
#define GetMappedFileName GetMappedFileNameW
#define GetDeviceDriverBaseName GetDeviceDriverBaseNameW
#define GetDeviceDriverFileName GetDeviceDriverFileNameW
#if (_WIN32_WINNT >= 0x0501)
#define GetProcessImageFileName GetProcessImageFileNameW
#endif
#else
#define GetModuleBaseName GetModuleBaseNameA
#define GetModuleFileNameEx GetModuleFileNameExA
#define GetMappedFileName GetMappedFileNameA
#define GetDeviceDriverBaseName GetDeviceDriverBaseNameA
#define GetDeviceDriverFileName GetDeviceDriverFileNameA
#if (_WIN32_WINNT >= 0x0501)
#define GetProcessImageFileName GetProcessImageFileNameA
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _PSAPI_H */

