/*
 * ntpoapi.h
 *
 * APIs for power management.
 *
 * This file is part of the w32api package.
 *
 * Contributors:
 *   Created by Casper S. Hornstrup <chorns@users.sourceforge.net>
 *
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * This source code is offered for use in the public domain. You may
 * use, modify or distribute it freely.
 *
 * This code is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 * DISCLAIMED. This includes but is not limited to warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __NTPOAPI_H
#define __NTPOAPI_H

#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,4)

#include "ntddk.h"
#include "batclass.h"


#define ES_SYSTEM_REQUIRED                0x00000001
#define ES_DISPLAY_REQUIRED               0x00000002
#define ES_USER_PRESENT                   0x00000004
#define ES_CONTINUOUS                     0x80000000

typedef enum _LATENCY_TIME {
	LT_DONT_CARE,
	LT_LOWEST_LATENCY
} LATENCY_TIME, *PLATENCY_TIME;

#define POWER_SYSTEM_MAXIMUM              PowerSystemMaximum

typedef enum _POWER_INFORMATION_LEVEL {
	SystemPowerPolicyAc,
	SystemPowerPolicyDc,
	VerifySystemPolicyAc,
	VerifySystemPolicyDc,
	SystemPowerCapabilities,
	SystemBatteryState,
	SystemPowerStateHandler,
	ProcessorStateHandler,
	SystemPowerPolicyCurrent,
	AdministratorPowerPolicy,
	SystemReserveHiberFile,
	ProcessorInformation,
	SystemPowerInformation,
	ProcessorStateHandler2,
	LastWakeTime,
	LastSleepTime,
	SystemExecutionState,
	SystemPowerStateNotifyHandler,
	ProcessorPowerPolicyAc,
	ProcessorPowerPolicyDc,
	VerifyProcessorPowerPolicyAc,
	VerifyProcessorPowerPolicyDc,
	ProcessorPowerPolicyCurrent
} POWER_INFORMATION_LEVEL;

#define POWER_PERF_SCALE                  100
#define PERF_LEVEL_TO_PERCENT(x)          (((x) * 1000) / (POWER_PERF_SCALE * 10))
#define PERCENT_TO_PERF_LEVEL(x)          (((x) * POWER_PERF_SCALE * 10) / 1000)

typedef struct _PROCESSOR_IDLE_TIMES {
	ULONGLONG  StartTime;
	ULONGLONG  EndTime;
	ULONG  IdleHandlerReserved[4];
} PROCESSOR_IDLE_TIMES, *PPROCESSOR_IDLE_TIMES;

typedef BOOLEAN DDKFASTAPI
(*PPROCESSOR_IDLE_HANDLER)(
  IN OUT PPROCESSOR_IDLE_TIMES IdleTimes);

typedef struct _PROCESSOR_IDLE_HANDLER_INFO {
  ULONG  HardwareLatency;
  PPROCESSOR_IDLE_HANDLER  Handler;
} PROCESSOR_IDLE_HANDLER_INFO, *PPROCESSOR_IDLE_HANDLER_INFO;

typedef VOID DDKFASTAPI
(*PSET_PROCESSOR_THROTTLE)(
  IN UCHAR  Throttle);

typedef NTSTATUS DDKFASTAPI
(*PSET_PROCESSOR_THROTTLE2)(
  IN UCHAR  Throttle);

#define MAX_IDLE_HANDLERS                 3

typedef struct _PROCESSOR_STATE_HANDLER {
	UCHAR  ThrottleScale;
	BOOLEAN  ThrottleOnIdle;
	PSET_PROCESSOR_THROTTLE  SetThrottle;
	ULONG  NumIdleHandlers;
	PROCESSOR_IDLE_HANDLER_INFO  IdleHandler[MAX_IDLE_HANDLERS];
} PROCESSOR_STATE_HANDLER, *PPROCESSOR_STATE_HANDLER;

typedef enum _POWER_STATE_HANDLER_TYPE {
	PowerStateSleeping1,
	PowerStateSleeping2,
	PowerStateSleeping3,
	PowerStateSleeping4,
	PowerStateSleeping4Firmware,
	PowerStateShutdownReset,
	PowerStateShutdownOff,
	PowerStateMaximum
} POWER_STATE_HANDLER_TYPE, *PPOWER_STATE_HANDLER_TYPE;

typedef NTSTATUS DDKAPI
(*PENTER_STATE_SYSTEM_HANDLER)(
  IN PVOID  SystemContext);

typedef NTSTATUS DDKAPI
(*PENTER_STATE_HANDLER)(
  IN PVOID  Context,
  IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler  OPTIONAL,
  IN PVOID  SystemContext,
  IN LONG  NumberProcessors,
  IN VOLATILE PLONG  Number);

typedef struct _POWER_STATE_HANDLER {
	POWER_STATE_HANDLER_TYPE  Type;
	BOOLEAN  RtcWake;
	UCHAR  Spare[3];
	PENTER_STATE_HANDLER  Handler;
	PVOID  Context;
} POWER_STATE_HANDLER, *PPOWER_STATE_HANDLER;

typedef NTSTATUS STDCALL
(*PENTER_STATE_NOTIFY_HANDLER)(
  IN POWER_STATE_HANDLER_TYPE  State,
  IN PVOID  Context,
  IN BOOLEAN  Entering);

typedef struct _POWER_STATE_NOTIFY_HANDLER {
	PENTER_STATE_NOTIFY_HANDLER  Handler;
	PVOID  Context;
} POWER_STATE_NOTIFY_HANDLER, *PPOWER_STATE_NOTIFY_HANDLER;

NTOSAPI
NTSTATUS
DDKAPI
NtPowerInformation(
  IN POWER_INFORMATION_LEVEL  InformationLevel,
  IN PVOID  InputBuffer OPTIONAL,
  IN ULONG  InputBufferLength,
  OUT PVOID  OutputBuffer OPTIONAL,
  IN ULONG  OutputBufferLength);

#define PROCESSOR_STATE_TYPE_PERFORMANCE  1
#define PROCESSOR_STATE_TYPE_THROTTLE     2

typedef struct _PROCESSOR_PERF_LEVEL {
  UCHAR  PercentFrequency;
  UCHAR  Reserved;
  USHORT  Flags;
} PROCESSOR_PERF_LEVEL, *PPROCESSOR_PERF_LEVEL;

typedef struct _PROCESSOR_PERF_STATE {
  UCHAR  PercentFrequency;
  UCHAR  MinCapacity;
  USHORT  Power;
  UCHAR  IncreaseLevel;
  UCHAR  DecreaseLevel;
  USHORT  Flags;
  ULONG  IncreaseTime;
  ULONG  DecreaseTime;
  ULONG  IncreaseCount;
  ULONG  DecreaseCount;
  ULONGLONG  PerformanceTime;
} PROCESSOR_PERF_STATE, *PPROCESSOR_PERF_STATE;

typedef struct _PROCESSOR_STATE_HANDLER2 {
	ULONG  NumIdleHandlers;
	PROCESSOR_IDLE_HANDLER_INFO  IdleHandler[MAX_IDLE_HANDLERS];
	PSET_PROCESSOR_THROTTLE2  SetPerfLevel;
	ULONG  HardwareLatency;
	UCHAR  NumPerfStates;
	PROCESSOR_PERF_LEVEL  PerfLevel[1];
} PROCESSOR_STATE_HANDLER2, *PPROCESSOR_STATE_HANDLER2;

/* POWER_ACTION_POLICY.Flags constants */
#define POWER_ACTION_QUERY_ALLOWED        0x00000001
#define POWER_ACTION_UI_ALLOWED           0x00000002
#define POWER_ACTION_OVERRIDE_APPS        0x00000004
#define POWER_ACTION_LIGHTEST_FIRST       0x10000000
#define POWER_ACTION_LOCK_CONSOLE         0x20000000
#define POWER_ACTION_DISABLE_WAKES        0x40000000
#define POWER_ACTION_CRITICAL             0x80000000

/* POWER_ACTION_POLICY.EventCode constants */
#define POWER_LEVEL_USER_NOTIFY_TEXT      0x00000001
#define POWER_LEVEL_USER_NOTIFY_SOUND     0x00000002
#define POWER_LEVEL_USER_NOTIFY_EXEC      0x00000004
#define POWER_USER_NOTIFY_BUTTON          0x00000008
#define POWER_USER_NOTIFY_SHUTDOWN        0x00000010
#define POWER_FORCE_TRIGGER_RESET         0x80000000

typedef struct _POWER_ACTION_POLICY {
	POWER_ACTION  Action;
	ULONG  Flags;
	ULONG  EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;

typedef struct _SYSTEM_POWER_LEVEL {
	BOOLEAN  Enable;
	UCHAR  Spare[3];
	ULONG  BatteryLevel;
	POWER_ACTION_POLICY  PowerPolicy;
	SYSTEM_POWER_STATE  MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

#define DISCHARGE_POLICY_CRITICAL         0
#define DISCHARGE_POLICY_LOW              1
#define NUM_DISCHARGE_POLICIES            4

#define PO_THROTTLE_NONE                  0
#define PO_THROTTLE_CONSTANT              1
#define PO_THROTTLE_DEGRADE               2
#define PO_THROTTLE_ADAPTIVE              3
#define PO_THROTTLE_MAXIMUM               4

typedef struct _SYSTEM_POWER_POLICY {
	ULONG  Revision;
	POWER_ACTION_POLICY  PowerButton;
	POWER_ACTION_POLICY  SleepButton;
	POWER_ACTION_POLICY  LidClose;
	SYSTEM_POWER_STATE  LidOpenWake;
	ULONG  Reserved;
	POWER_ACTION_POLICY  Idle;
	ULONG  IdleTimeout;
	UCHAR  IdleSensitivity;
	UCHAR  DynamicThrottle;
	UCHAR  Spare2[2];
	SYSTEM_POWER_STATE  MinSleep;
	SYSTEM_POWER_STATE  MaxSleep;
	SYSTEM_POWER_STATE  ReducedLatencySleep;
	ULONG  WinLogonFlags;
	ULONG  Spare3;
	ULONG  DozeS4Timeout;
	ULONG  BroadcastCapacityResolution;
	SYSTEM_POWER_LEVEL  DischargePolicy[NUM_DISCHARGE_POLICIES];
	ULONG  VideoTimeout;
	BOOLEAN  VideoDimDisplay;
	ULONG  VideoReserved[3];
	ULONG  SpindownTimeout;
	BOOLEAN  OptimizeForPower;
	UCHAR  FanThrottleTolerance;
	UCHAR  ForcedThrottle;
	UCHAR  MinThrottle;
	POWER_ACTION_POLICY  OverThrottled;
} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

typedef struct _PROCESSOR_POWER_POLICY_INFO {
	ULONG  TimeCheck;
	ULONG  DemoteLimit;
	ULONG  PromoteLimit;
	UCHAR  DemotePercent;
	UCHAR  PromotePercent;
	UCHAR  Spare[2];
	ULONG  AllowDemotion : 1;
	ULONG  AllowPromotion : 1;
	ULONG  Reserved : 30;
} PROCESSOR_POWER_POLICY_INFO, *PPROCESSOR_POWER_POLICY_INFO;

typedef struct _PROCESSOR_POWER_POLICY {
	ULONG  Revision;
	UCHAR  DynamicThrottle;
	UCHAR  Spare[3];
	ULONG  Reserved;
	ULONG  PolicyCount;
	PROCESSOR_POWER_POLICY_INFO  Policy[3];
} PROCESSOR_POWER_POLICY, *PPROCESSOR_POWER_POLICY;

typedef struct _ADMINISTRATOR_POWER_POLICY {
  SYSTEM_POWER_STATE  MinSleep;
  SYSTEM_POWER_STATE  MaxSleep;
  ULONG  MinVideoTimeout;
  ULONG  MaxVideoTimeout;
  ULONG  MinSpindownTimeout;
  ULONG  MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;

NTOSAPI
NTSTATUS
DDKAPI
NtSetThreadExecutionState(
  IN EXECUTION_STATE  esFlags,
  OUT EXECUTION_STATE  *PreviousFlags);

NTOSAPI
NTSTATUS
DDKAPI
NtRequestWakeupLatency(
  IN LATENCY_TIME  latency);

NTOSAPI
NTSTATUS
DDKAPI
NtInitiatePowerAction(
  IN POWER_ACTION  SystemAction,
  IN SYSTEM_POWER_STATE  MinSystemState,
  IN ULONG  Flags,
  IN BOOLEAN  Asynchronous);

NTOSAPI
NTSTATUS
DDKAPI
NtSetSystemPowerState(
  IN POWER_ACTION SystemAction,
  IN SYSTEM_POWER_STATE MinSystemState,
  IN ULONG Flags);

NTOSAPI
NTSTATUS
DDKAPI
NtGetDevicePowerState(
  IN HANDLE  Device,
  OUT DEVICE_POWER_STATE  *State);

NTOSAPI
NTSTATUS
DDKAPI
NtCancelDeviceWakeupRequest(
  IN HANDLE  Device);

NTOSAPI
BOOLEAN
DDKAPI
NtIsSystemResumeAutomatic(
  VOID);

NTOSAPI
NTSTATUS
DDKAPI
NtRequestDeviceWakeup(
  IN HANDLE  Device);

#define WINLOGON_LOCK_ON_SLEEP            0x00000001

typedef struct _SYSTEM_POWER_INFORMATION {
  ULONG  MaxIdlenessAllowed;
  ULONG  Idleness;
  ULONG  TimeRemaining;
  UCHAR  CoolingMode;
} SYSTEM_POWER_INFORMATION, *PSYSTEM_POWER_INFORMATION;

typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG  Number;
  ULONG  MaxMhz;
  ULONG  CurrentMhz;
  ULONG  MhzLimit;
  ULONG  MaxIdleState;
  ULONG  CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef struct _SYSTEM_BATTERY_STATE {
  BOOLEAN  AcOnLine;
  BOOLEAN  BatteryPresent;
  BOOLEAN  Charging;
  BOOLEAN  Discharging;
  BOOLEAN  Spare1[4];
  ULONG  MaxCapacity;
  ULONG  RemainingCapacity;
  ULONG  Rate;
  ULONG  EstimatedTime;
  ULONG  DefaultAlert1;
  ULONG  DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;

typedef struct _SYSTEM_POWER_CAPABILITIES {
  BOOLEAN  PowerButtonPresent;
  BOOLEAN  SleepButtonPresent;
  BOOLEAN  LidPresent;
  BOOLEAN  SystemS1;
  BOOLEAN  SystemS2;
  BOOLEAN  SystemS3;
  BOOLEAN  SystemS4;
  BOOLEAN  SystemS5;
  BOOLEAN  HiberFilePresent;
  BOOLEAN  FullWake;
  BOOLEAN  VideoDimPresent;
  BOOLEAN  ApmPresent;
  BOOLEAN  UpsPresent;
  BOOLEAN  ThermalControl;
  BOOLEAN  ProcessorThrottle;
  UCHAR  ProcessorMinThrottle;
  UCHAR  ProcessorMaxThrottle;
  UCHAR  spare2[4];
  BOOLEAN  DiskSpinDown;
  UCHAR  spare3[8];
  BOOLEAN  SystemBatteriesPresent;
  BOOLEAN  BatteriesAreShortTerm;
  BATTERY_REPORTING_SCALE  BatteryScale[3];
  SYSTEM_POWER_STATE  AcOnLineWake;
  SYSTEM_POWER_STATE  SoftLidWake;
  SYSTEM_POWER_STATE  RtcWake;
  SYSTEM_POWER_STATE  MinDeviceWakeState;
  SYSTEM_POWER_STATE  DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* __NTPOAPI_H */
