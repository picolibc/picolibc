/* ntdll.h.  Contains ntdll specific stuff not defined elsewhere.

   Copyright 2000, 2001, 2002, 2003 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xc0000004)
#define PDI_MODULES 0x01
#define PDI_HEAPS 0x04
#define LDRP_IMAGE_DLL 0x00000004
#define WSLE_PAGE_READONLY 0x001
#define WSLE_PAGE_EXECUTE 0x002
#define WSLE_PAGE_EXECUTE_READ 0x003
#define WSLE_PAGE_READWRITE 0x004
#define WSLE_PAGE_WRITECOPY 0x005
#define WSLE_PAGE_EXECUTE_READWRITE 0x006
#define WSLE_PAGE_EXECUTE_WRITECOPY 0x007
#define WSLE_PAGE_SHARE_COUNT_MASK 0x0E0
#define WSLE_PAGE_SHAREABLE 0x100

typedef ULONG KAFFINITY;

typedef enum _SYSTEM_INFORMATION_CLASS
{
  SystemBasicInformation = 0,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemProcessesAndThreadsInformation = 5,
  SystemProcessorTimes = 8,
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
  UCHAR NumberProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct __attribute__ ((aligned (8))) _SYSTEM_PROCESSOR_TIMES
{
  LARGE_INTEGER IdleTime;
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER DpcTime;
  LARGE_INTEGER InterruptTime;
  ULONG InterruptCount;
} SYSTEM_PROCESSOR_TIMES, *PSYSTEM_PROCESSOR_TIMES;

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
  DWORD Reserved;
} SYSTEM_THREADS, *PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES
{
  ULONG NextEntryDelta;
  ULONG ThreadCount;
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

typedef struct _IO_STATUS_BLOCK
{
  NTSTATUS Status;
  ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION
{
  LARGE_INTEGER IdleTime;
  LARGE_INTEGER ReadTransferCount;
  LARGE_INTEGER WriteTransferCount;
  LARGE_INTEGER OtherTransferCount;
  ULONG ReadOperationCount;
  ULONG WriteOperationCount;
  ULONG OtherOperationCount;
  ULONG AvailablePages;
  ULONG TotalCommittedPages;
  ULONG TotalCommitLimit;
  ULONG PeakCommitment;
  ULONG PageFaults;
  ULONG WriteCopyFaults;
  ULONG TransitionFaults;
  ULONG Reserved1;
  ULONG DemandZeroFaults;
  ULONG PagesRead;
  ULONG PageReadIos;
  ULONG Reserved2[2];
  ULONG PagefilePagesWritten;
  ULONG PagefilePageWriteIos;
  ULONG MappedFilePagesWritten;
  ULONG MappedFilePageWriteIos;
  ULONG PagedPoolUsage;
  ULONG NonPagedPoolUsage;
  ULONG PagedPoolAllocs;
  ULONG PagedPoolFrees;
  ULONG NonPagedPoolAllocs;
  ULONG NonPagedPoolFrees;
  ULONG TotalFreeSystemPtes;
  ULONG SystemCodePage;
  ULONG TotalSystemDriverPages;
  ULONG TotalSystemCodePages;
  ULONG SmallNonPagedLookasideListAllocateHits;
  ULONG SmallPagedLookasideListAllocateHits;
  ULONG Reserved3;
  ULONG MmSystemCachePage;
  ULONG PagedPoolPage;
  ULONG SystemDriverPage;
  ULONG FastReadNoWait;
  ULONG FastReadWait;
  ULONG FastReadResourceMiss;
  ULONG FastReadNotPossible;
  ULONG FastMdlReadNoWait;
  ULONG FastMdlReadWait;
  ULONG FastMdlReadResourceMiss;
  ULONG FastMdlReadNotPossible;
  ULONG MapDataNoWait;
  ULONG MapDataWait;
  ULONG MapDataNoWaitMiss;
  ULONG MapDataWaitMiss;
  ULONG PinMappedDataCount;
  ULONG PinReadNoWait;
  ULONG PinReadWait;
  ULONG PinReadNoWaitMiss;
  ULONG PinReadWaitMiss;
  ULONG CopyReadNoWait;
  ULONG CopyReadWait;
  ULONG CopyReadNoWaitMiss;
  ULONG CopyReadWaitMiss;
  ULONG MdlReadNoWait;
  ULONG MdlReadWait;
  ULONG MdlReadNoWaitMiss;
  ULONG MdlReadWaitMiss;
  ULONG ReadAheadIos;
  ULONG LazyWriteIos;
  ULONG LazyWritePages;
  ULONG DataFlushes;
  ULONG DataPages;
  ULONG ContextSwitches;
  ULONG FirstLevelTbFills;
  ULONG SecondLevelTbFills;
  ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct __attribute__ ((aligned(8))) _SYSTEM_TIME_OF_DAY_INFORMATION
{
  LARGE_INTEGER BootTime;
  LARGE_INTEGER CurrentTime;
  LARGE_INTEGER TimeZoneBias;
  ULONG CurrentTimeZoneId;
} SYSTEM_TIME_OF_DAY_INFORMATION, *PSYSTEM_TIME_OF_DAY_INFORMATION;

typedef enum _PROCESSINFOCLASS
{
  ProcessBasicInformation = 0,
  ProcessQuotaLimits = 1,
  ProcessVmCounters = 3,
  ProcessTimes =4,
} PROCESSINFOCLASS;

typedef struct _DEBUG_BUFFER
{
  HANDLE SectionHandle;
  PVOID SectionBase;
  PVOID RemoteSectionBase;
  ULONG SectionBaseDelta;
  HANDLE EventPairHandle;
  ULONG Unknown[2];
  HANDLE RemoteThreadHandle;
  ULONG InfoClassMask;
  ULONG SizeOfInfo;
  ULONG AllocatedSize;
  ULONG SectionSize;
  PVOID ModuleInformation;
  PVOID BackTraceInformation;
  PVOID HeapInformation;
  PVOID LockInformation;
  PVOID Reserved[9];
} DEBUG_BUFFER, *PDEBUG_BUFFER;

typedef struct _DEBUG_HEAP_INFORMATION
{
  ULONG Base;
  ULONG Flags;
  USHORT Granularity;
  USHORT Unknown;
  ULONG Allocated;
  ULONG Committed;
  ULONG TagCount;
  ULONG BlockCount;
  ULONG Reserved[7];
  PVOID Tags;
  PVOID Blocks;
} DEBUG_HEAP_INFORMATION, *PDEBUG_HEAP_INFORMATION;

typedef struct _DEBUG_MODULE_INFORMATION
{
  ULONG Reserved[2];
  ULONG Base;
  ULONG Size;
  ULONG Flags;
  USHORT Index;
  USHORT Unknown;
  USHORT LoadCount;
  USHORT ModuleNameOffset;
  CHAR ImageName[256];
} DEBUG_MODULE_INFORMATION, *PDEBUG_MODULE_INFORMATION;

typedef struct _KERNEL_USER_TIMES
{
  LARGE_INTEGER CreateTime;
  LARGE_INTEGER ExitTime;
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES, *PKERNEL_USER_TIMES;

typedef void *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION
{
  NTSTATUS ExitStatus;
  PPEB PebBaseAddress;
  KAFFINITY AffinityMask;
  KPRIORITY BasePriority;
  ULONG UniqueProcessId;
  ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef enum _MEMORY_INFORMATION_CLASS
{
  MemoryBasicInformation,
  MemoryWorkingSetList,
  MemorySectionName,
  MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_WORKING_SET_LIST
{
  ULONG NumberOfPages;
  ULONG WorkingSetList[1];
} MEMORY_WORKING_SET_LIST, *PMEMORY_WORKING_SET_LIST;

typedef struct _FILE_NAME_INFORMATION
{
  DWORD FileNameLength;
  WCHAR FileName[MAX_PATH + 100];
} FILE_NAME_INFORMATION;

typedef enum _OBJECT_INFORMATION_CLASS
{
   ObjectBasicInformation = 0,
   ObjectNameInformation = 1,
   ObjectHandleInformation = 4
   // and many more
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_NAME_INFORMATION
{
  UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION;

/* Function declarations for ntdll.dll.  These don't appear in any
   standard Win32 header.  */
extern "C"
{
  NTSTATUS NTAPI NtCreateToken (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
				TOKEN_TYPE, PLUID, PLARGE_INTEGER, PTOKEN_USER,
				PTOKEN_GROUPS, PTOKEN_PRIVILEGES, PTOKEN_OWNER,
				PTOKEN_PRIMARY_GROUP, PTOKEN_DEFAULT_DACL,
				PTOKEN_SOURCE);
  NTSTATUS NTAPI NtMapViewOfSection (HANDLE, HANDLE, PVOID *, ULONG, ULONG,
				     PLARGE_INTEGER, PULONG, SECTION_INHERIT,
				     ULONG, ULONG);
  NTSTATUS NTAPI NtOpenFile (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
			     PIO_STATUS_BLOCK, ULONG, ULONG);
  NTSTATUS NTAPI NtOpenSection (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
  NTSTATUS NTAPI NtQueryInformationFile (HANDLE, IO_STATUS_BLOCK *, VOID *,
					 DWORD, DWORD);
  NTSTATUS NTAPI NtQueryInformationProcess (HANDLE, PROCESSINFOCLASS,
					    PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtQueryObject (HANDLE, OBJECT_INFORMATION_CLASS, VOID *,
				ULONG, ULONG *);
  NTSTATUS NTAPI NtQuerySystemInformation (SYSTEM_INFORMATION_CLASS,
					   PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtQueryVirtualMemory (HANDLE, PVOID, MEMORY_INFORMATION_CLASS,
				       PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtUnmapViewOfSection (HANDLE, PVOID);
  VOID NTAPI RtlInitUnicodeString (PUNICODE_STRING, PCWSTR);
  ULONG NTAPI RtlNtStatusToDosError (NTSTATUS);
}
