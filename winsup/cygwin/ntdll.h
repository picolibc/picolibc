/* ntdll.h.  Contains ntdll specific stuff which is nowhere defined.

   Copyright 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xc0000004)

typedef enum _SYSTEM_INFORMATION_CLASS
{
  SystemBasicInformation = 0,
  SystemProcessesAndThreadsInformation = 5,
  /* There are a lot more of these... */
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_BASIC_INFORMATION
{
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

typedef LONG KPRIORITY;
typedef struct _VM_COUNTERS
{
  ULONG PeakVirtualSize;
  ULONG VirtualSize;
  ULONG PageFaultCount;
  ULONG PeakWorkingSetSize;
  ULONG WorkingSetSize;
  ULONG QuotaPeakPagedPoolUsage;
  ULONG QuotaPagedPoolUsage;
  ULONG QuotaPeakNonPagedPoolUsage;
  ULONG QuotaNonPagedPoolUsage;
  ULONG PagefileUsage;
  ULONG PeakPagefileUsage;
} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _IO_COUNTERS
{
  LARGE_INTEGER ReadOperationCount;
  LARGE_INTEGER WriteOperationCount;
  LARGE_INTEGER OtherOperationCount;
  LARGE_INTEGER ReadTransferCount;
  LARGE_INTEGER WriteTransferCount;
  LARGE_INTEGER OtherTransferCount;
} IO_COUNTERS, *PIO_COUNTERS;

typedef struct _CLIENT_ID
{
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef enum
{
  StateInitialized,
  StateReady,
  StateRunning,
  StateStandby,
  StateTerminated,
  StateWait,
  StateTransition,
  StateUnknown,
} THREAD_STATE;

typedef enum
{
  Executive,
  FreePage,
  PageIn,
  PoolAllocation,
  DelayExecution,
  Suspended,
  UserRequest,
  WrExecutive,
  WrFreePage,
  WrPageIn,
  WrPoolAllocation,
  WrDelayExecution,
  WrSuspended,
  WrUserRequest,
  WrEventPair,
  WrQueue,
  WrLpcReceive,
  WrLpcReply,
  WrVirtualMemory,
  WrPageOut,
  WrRendezvous,
  Spare2,
  Spare3,
  Spare4,
  Spare5,
  Spare6,
  WrKernel,
  MaximumWaitReason
} KWAIT_REASON;

typedef struct _SYSTEM_THREADS
{
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER CreateTime;
  ULONG WaitTime;
  PVOID StartAddress;
  CLIENT_ID ClientId;
  KPRIORITY Priority;
  KPRIORITY BasePriority;
  ULONG ContextSwitchCount;
  THREAD_STATE State;
  KWAIT_REASON WaitReason;
} SYSTEM_THREADS, *PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES
{
  ULONG NextEntryDelta;
  ULONG Threadcount;
  ULONG Reserved1[6];
  LARGE_INTEGER CreateTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;
  UNICODE_STRING ProcessName;
  KPRIORITY BasePriority;
  ULONG ProcessId;
  ULONG InheritedFromProcessId;
  ULONG HandleCount;
  ULONG Reserved2[2];
  VM_COUNTERS VmCounters;
  IO_COUNTERS IoCounters;
  SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;

/* Function declarations for ntdll.dll.  These don't appear in any
   standard Win32 header.  */
extern "C"
{
  NTSTATUS NTAPI NtMapViewOfSection (HANDLE, HANDLE, PVOID *, ULONG, ULONG,
				     PLARGE_INTEGER, PULONG, SECTION_INHERIT,
				     ULONG, ULONG);
  NTSTATUS NTAPI NtQuerySystemInformation (SYSTEM_INFORMATION_CLASS,
					   PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtOpenSection (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
  NTSTATUS NTAPI NtUnmapViewOfSection (HANDLE, PVOID);
  VOID NTAPI RtlInitUnicodeString (PUNICODE_STRING, PCWSTR);
  ULONG NTAPI RtlNtStatusToDosError (NTSTATUS);
  NTSTATUS NTAPI ZwQuerySystemInformation (IN SYSTEM_INFORMATION_CLASS,
					   IN OUT PVOID, IN ULONG,
					   OUT PULONG);
}
