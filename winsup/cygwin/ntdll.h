/* ntdll.h.  Contains ntdll specific stuff which is nowhere defined.

   Copyright 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/*
 * The following both data structures aren't defined anywhere in the Microsoft
 * header files. Taken from the book "Windows NT/2000 Native API Reference"
 * by Gary Nebbett.
 */
typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation = 0
  /* Dropped each other since not used here. */
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_BASIC_INFORMATION {
  ULONG Unknown;
  ULONG MaximumIncrement;
  ULONG PhysicalPageSize;
  ULONG NumberOfPhysicalPages;
  ULONG LowestPhysicalPage;
  ULONG HighestPhysicalPage;
  ULONG AllocationGranularity;
  ULONG LowestUserAddress;
  ULONG HighestUserAddress;
  ULONG ActiveProcessors;
  ULONG NumberProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

/*
 * Function declarations for ntdll.dll. They doesn't appear in any
 * Win32 header either.
 */
extern "C" {
NTSTATUS NTAPI NtMapViewOfSection(HANDLE,HANDLE,PVOID*,ULONG,ULONG,
                                  PLARGE_INTEGER,PULONG,SECTION_INHERIT,
                                  ULONG,ULONG);
NTSTATUS NTAPI NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS,
                                        PVOID,ULONG,PULONG);
NTSTATUS NTAPI NtOpenSection(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);
NTSTATUS NTAPI NtUnmapViewOfSection(HANDLE,PVOID);
VOID NTAPI RtlInitUnicodeString(PUNICODE_STRING,PCWSTR);
ULONG NTAPI RtlNtStatusToDosError(NTSTATUS);
}

