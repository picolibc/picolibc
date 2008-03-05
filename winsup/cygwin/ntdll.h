/* ntdll.h.  Contains ntdll specific stuff not defined elsewhere.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef STATUS_INVALID_INFO_CLASS
/* Some w32api header file defines this so we need to conditionalize this
   define to avoid warnings. */
#define STATUS_INVALID_INFO_CLASS   ((NTSTATUS) 0xc0000003)
#endif
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xc0000004)
#define STATUS_INVALID_PARAMETER    ((NTSTATUS) 0xc000000d)
#define STATUS_BUFFER_TOO_SMALL     ((NTSTATUS) 0xc0000023)
#define STATUS_OBJECT_NAME_INVALID  ((NTSTATUS) 0xc0000033)
#define STATUS_WORKING_SET_QUOTA    ((NTSTATUS) 0xc00000a1L)
#define STATUS_INVALID_LEVEL        ((NTSTATUS) 0xc0000148)
#define STATUS_NO_MORE_FILES	    ((NTSTATUS)0x80000006L)
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

typedef enum _FILE_INFORMATION_CLASS
{
  FileDirectoryInformation = 1,
  FileFullDirectoryInformation, // 2
  FileBothDirectoryInformation, // 3
  FileBasicInformation, // 4 wdm
  FileStandardInformation, // 5 wdm
  FileInternalInformation, // 6
  FileEaInformation, // 7
  FileAccessInformation, // 8
  FileNameInformation, // 9
  FileRenameInformation, // 10
  FileLinkInformation, // 11
  FileNamesInformation, // 12
  FileDispositionInformation, // 13
  FilePositionInformation, // 14 wdm
  FileFullEaInformation, // 15
  FileModeInformation, // 16
  FileAlignmentInformation, // 17
  FileAllInformation, // 18
  FileAllocationInformation, // 19
  FileEndOfFileInformation, // 20 wdm
  FileAlternateNameInformation, // 21
  FileStreamInformation, // 22
  FilePipeInformation, // 23
  FilePipeLocalInformation, // 24
  FilePipeRemoteInformation, // 25
  FileMailslotQueryInformation, // 26
  FileMailslotSetInformation, // 27
  FileCompressionInformation, // 28
  FileObjectIdInformation, // 29
  FileCompletionInformation, // 30
  FileMoveClusterInformation, // 31
  FileQuotaInformation, // 32
  FileReparsePointInformation, // 33
  FileNetworkOpenInformation, // 34
  FileAttributeTagInformation, // 35
  FileTrackingInformation, // 36
  FileIdBothDirectoryInformation, // 37
  FileIdFullDirectoryInformation, // 38
  FileValidDataLengthInformation, // 39
  FileShortNameInformation, // 40
  FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _FILE_BOTH_DIR_INFORMATION
{
  ULONG  NextEntryOffset;
  ULONG  FileIndex;
  LARGE_INTEGER  CreationTime;
  LARGE_INTEGER  LastAccessTime;
  LARGE_INTEGER  LastWriteTime;
  LARGE_INTEGER  ChangeTime;
  LARGE_INTEGER  EndOfFile;
  LARGE_INTEGER  AllocationSize;
  ULONG  FileAttributes;
  ULONG  FileNameLength;
  ULONG  EaSize;
  CCHAR  ShortNameLength;
  WCHAR  ShortName[12];
  WCHAR  FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION
{
  ULONG  NextEntryOffset;
  ULONG  FileIndex;
  LARGE_INTEGER  CreationTime;
  LARGE_INTEGER  LastAccessTime;
  LARGE_INTEGER  LastWriteTime;
  LARGE_INTEGER  ChangeTime;
  LARGE_INTEGER  EndOfFile;
  LARGE_INTEGER  AllocationSize;
  ULONG  FileAttributes;
  ULONG  FileNameLength;
  ULONG  EaSize;
  CCHAR  ShortNameLength;
  WCHAR  ShortName[12];
  LARGE_INTEGER  FileId;
  WCHAR  FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;


#define AT_EXTENDABLE_FILE 0x00002000
#define AT_ROUND_TO_PAGE 0x40000000

#define LOCK_VM_IN_WSL 1
#define LOCK_VM_IN_RAM 2

#define DIRECTORY_QUERY 1

typedef ULONG KAFFINITY;

typedef enum _SYSTEM_INFORMATION_CLASS
{
  SystemBasicInformation = 0,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemProcessesAndThreadsInformation = 5,
  SystemProcessorTimes = 8,
  SystemPagefileInformation = 18,
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

typedef struct _SYSTEM_PAGEFILE_INFORMATION
{
  ULONG NextEntryOffset;
  ULONG CurrentSize;
  ULONG TotalUsed;
  ULONG PeakUsed;
  UNICODE_STRING FileName;
} SYSTEM_PAGEFILE_INFORMATION, *PSYSTEM_PAGEFILE_INFORMATION;

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

typedef struct _FILE_BASIC_INFORMATION {
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
  LARGE_INTEGER AllocationSize;
  LARGE_INTEGER EndOfFile;
  ULONG NumberOfLinks;
  BOOLEAN DeletePending;
  BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
  LARGE_INTEGER FileId;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
  ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
  ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {
  LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
  ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {
  ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
  ULONG FileNameLength;
  WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
  FILE_BASIC_INFORMATION     BasicInformation;
  FILE_STANDARD_INFORMATION  StandardInformation;
  FILE_INTERNAL_INFORMATION  InternalInformation;
  FILE_EA_INFORMATION        EaInformation;
  FILE_ACCESS_INFORMATION    AccessInformation;
  FILE_POSITION_INFORMATION  PositionInformation;
  FILE_MODE_INFORMATION      ModeInformation;
  FILE_ALIGNMENT_INFORMATION AlignmentInformation;
  FILE_NAME_INFORMATION      NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION
{
  ULONG NamedPipeType;
  ULONG NamedPipeConfiguration;
  ULONG MaximumInstances;
  ULONG CurrentInstances;
  ULONG InboundQuota;
  ULONG ReadDataAvailable;
  ULONG OutboundQuota;
  ULONG WriteQuotaAvailable;
  ULONG NamedPipeState;
  ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _FILE_COMPRESSION_INFORMATION
{
  LARGE_INTEGER CompressedSize;
  USHORT CompressionFormat;
  UCHAR	CompressionUnitShift;
  UCHAR Unknown;
  UCHAR ClusterSizeShift;
} FILE_COMPRESSION_INFORMATION, *PFILE_COMPRESSION_INFORMATION;

typedef struct _FILE_FS_VOLUME_INFORMATION
{
  LARGE_INTEGER VolumeCreationTime;
  ULONG VolumeSerialNumber;
  ULONG VolumeLabelLength;
  BOOLEAN SupportsObjects;
  WCHAR VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

typedef struct _FILE_FS_SIZE_INFORMATION
{
  LARGE_INTEGER TotalAllocationUnits;
  LARGE_INTEGER AvailableAllocationUnits;
  ULONG SectorsPerAllocationUnit;
  ULONG BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef enum _FSINFOCLASS {
  FileFsVolumeInformation = 1,
  FileFsLabelInformation,
  FileFsSizeInformation,
  FileFsDeviceInformation,
  FileFsAttributeInformation,
  FileFsControlInformation,
  FileFsFullSizeInformation,
  FileFsObjectIdInformation,
  FileFsDriverPathInformation,
  FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

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

typedef struct _DIRECTORY_BASIC_INFORMATION
{
  UNICODE_STRING ObjectName;
  UNICODE_STRING ObjectTypeName;
} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;

typedef struct _FILE_GET_EA_INFORMATION
{
  ULONG   NextEntryOffset;
  UCHAR   EaNameLength;
  CHAR    EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;


typedef struct _FILE_FULL_EA_INFORMATION
{
  ULONG NextEntryOffset;
  UCHAR Flags;
  UCHAR EaNameLength;
  USHORT EaValueLength;
  CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

/* Function declarations for ntdll.dll.  These don't appear in any
   standard Win32 header.  */
extern "C"
{
  NTSTATUS NTAPI NtClose (HANDLE);
  NTSTATUS NTAPI NtCreateFile (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
			     PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG,
			     ULONG, ULONG, PVOID, ULONG);
  NTSTATUS NTAPI NtCreateSection (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
  				  PLARGE_INTEGER, ULONG, ULONG, HANDLE);
  NTSTATUS NTAPI NtCreateToken (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
				TOKEN_TYPE, PLUID, PLARGE_INTEGER, PTOKEN_USER,
				PTOKEN_GROUPS, PTOKEN_PRIVILEGES, PTOKEN_OWNER,
				PTOKEN_PRIMARY_GROUP, PTOKEN_DEFAULT_DACL,
				PTOKEN_SOURCE);
  NTSTATUS NTAPI NtLockVirtualMemory (HANDLE, PVOID *, ULONG *, ULONG);
  NTSTATUS NTAPI NtMapViewOfSection (HANDLE, HANDLE, PVOID *, ULONG, ULONG,
				     PLARGE_INTEGER, PULONG, SECTION_INHERIT,
				     ULONG, ULONG);
  NTSTATUS NTAPI NtOpenDirectoryObject (PHANDLE, ACCESS_MASK,
  					POBJECT_ATTRIBUTES);
  NTSTATUS NTAPI NtOpenFile (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
			     PIO_STATUS_BLOCK, ULONG, ULONG);
  NTSTATUS NTAPI NtOpenSection (PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
  NTSTATUS NTAPI NtQueryDirectoryFile(HANDLE, HANDLE, PVOID, PVOID,
				      PIO_STATUS_BLOCK, PVOID, ULONG,
				      FILE_INFORMATION_CLASS, BOOLEAN,
				      PUNICODE_STRING, BOOLEAN);
  NTSTATUS NTAPI NtQueryDirectoryObject (HANDLE, PVOID, ULONG, BOOLEAN,
  					 BOOLEAN, PULONG, PULONG);
  NTSTATUS NTAPI NtQueryEaFile (HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG,
				BOOLEAN, PVOID, ULONG, PULONG, BOOLEAN);
  NTSTATUS NTAPI NtQueryInformationFile (HANDLE, PIO_STATUS_BLOCK, PVOID,
					 ULONG, FILE_INFORMATION_CLASS);
  NTSTATUS NTAPI NtQueryInformationProcess (HANDLE, PROCESSINFOCLASS,
					    PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtQueryObject (HANDLE, OBJECT_INFORMATION_CLASS, VOID *,
				ULONG, ULONG *);
  NTSTATUS NTAPI NtQuerySystemInformation (SYSTEM_INFORMATION_CLASS,
					   PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtQuerySecurityObject (HANDLE, SECURITY_INFORMATION,
  					PSECURITY_DESCRIPTOR, ULONG, PULONG);
  NTSTATUS NTAPI NtQueryVirtualMemory (HANDLE, PVOID, MEMORY_INFORMATION_CLASS,
				       PVOID, ULONG, PULONG);
  NTSTATUS NTAPI NtQueryVolumeInformationFile (HANDLE, IO_STATUS_BLOCK *,
					       VOID *, ULONG,
					       FS_INFORMATION_CLASS);
  NTSTATUS NTAPI NtSetEaFile (HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG);
  NTSTATUS NTAPI NtSetSecurityObject (HANDLE, SECURITY_INFORMATION,
				      PSECURITY_DESCRIPTOR);
  NTSTATUS NTAPI NtUnlockVirtualMemory (HANDLE, PVOID *, ULONG *, ULONG);
  NTSTATUS NTAPI NtUnmapViewOfSection (HANDLE, PVOID);
  VOID NTAPI RtlInitUnicodeString (PUNICODE_STRING, PCWSTR);
  ULONG NTAPI RtlIsDosDeviceName_U (PCWSTR);
  ULONG NTAPI RtlNtStatusToDosError (NTSTATUS);
}
