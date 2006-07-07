/* autoload.cc: all dynamic load stuff.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>

/* Macro for defining "auto-load" functions.
 * Note that this is self-modifying code *gasp*.
 * The first invocation of a routine will trigger the loading of
 * the DLL.  This will then be followed by the discovery of
 * the procedure's entry point, which is placed into the location
 * pointed to by the stack pointer.  This code then changes
 * the "call" operand which invoked it to a "jmp" which will
 * transfer directly to the DLL function on the next invocation.
 *
 * Subsequent calls to routines whose transfer address has not been
 * determined will skip the "load the dll" step, starting at the
 * "discovery of the entry point" step.
 *
 * So, immediately following the the call to one of the above routines
 * we have:
 *  DLL info (4 bytes)	 Pointer to a block of information concerning
 *			 the DLL (see below).
 *  DLL args (4 bytes)	 The number of arguments pushed on the stack by
 *			 the call.  If this is an odd value then this
 *			 is a flag that non-existence of this function
 *			 is not a fatal error
 *  func name (n bytes)	 asciz string containing the name of the function
 *			 to be loaded.
 *
 * The DLL info block consists of the following
 *  load_state (4 bytes) Pointer to a word containing the routine used
 *			 to eventually invoke the function.  Initially
 *			 points to an init function which loads the
 *			 DLL, gets the process's load address,
 *			 changes the contents here to point to the
 *			 function address, and changes the call *(%eax)
 *			 to a jmp func.  If the initialization has been
 *			 done, only the load part is done.
 *  DLL handle (4 bytes) The handle to use when loading the DLL.
 *  DLL locker (4 bytes) Word to use to avoid multi-thread access during
 *			 initialization.
 *  extra init (4 bytes) Extra initialization function.
 *  DLL name (n bytes)	 asciz string containing the name of the DLL.
 */

/* LoadDLLprime is used to prime the DLL info information, providing an
   additional initialization routine to call prior to calling the first
   function.  */
#define LoadDLLprime(dllname, init_also) __asm__ ("	\n\
.ifndef " #dllname "_primed				\n\
  .section	.data_cygwin_nocopy,\"w\"		\n\
  .align	4					\n\
."#dllname "_info:					\n\
  .long		_std_dll_init				\n\
  .long		0					\n\
  .long		-1					\n\
  .long		" #init_also "				\n\
  .asciz	\"" #dllname "\"			\n\
  .text							\n\
  .set		" #dllname "_primed, 1			\n\
.endif							\n\
");

/* Create a "decorated" name */
#define mangle(name, n) #name "@" #n

/* Standard DLL load macro.  May invoke a fatal error if the function isn't
   found. */
#define LoadDLLfunc(name, n, dllname) \
  LoadDLLfuncEx (name, n, dllname, 0)
#define LoadDLLfuncEx(name, n, dllname, notimp) \
  LoadDLLfuncEx2(name, n, dllname, notimp, 0)
#define LoadDLLfuncEx2(name, n, dllname, notimp, err) \
  LoadDLLfuncEx3(name, n, dllname, notimp, err, 0)

/* Main DLL setup stuff. */
#define LoadDLLfuncEx3(name, n, dllname, notimp, err, fn) \
  LoadDLLprime (dllname, dll_func_load)			\
  __asm__ ("						\n\
  .section	." #dllname "_autoload_text,\"wx\"	\n\
  .global	_" mangle (name, n) "			\n\
  .global	_win32_" mangle (name, n) "		\n\
  .align	8					\n\
_" mangle (name, n) ":					\n\
_win32_" mangle (name, n) ":				\n\
  .byte		0xe9					\n\
  .long		-4 + 1f - .				\n\
1:movl		(2f),%eax				\n\
   call		*(%eax)					\n\
2:.long		." #dllname "_info			\n\
   .long		(" #n "+" #notimp ") | (((" #err ") & 0xff) <<16) | (((" #fn ") & 0xff) << 24)	\n\
   .asciz	\"" #name "\"				\n\
   .text							\n\
");

/* DLL loader helper functions used during initialization. */

/* The function which finds the address, given the name and overwrites
   the call so that future invocations go straight to the function in
   the DLL. */
extern "C" void dll_func_load () __asm__ ("dll_func_load");

/* Called by the primary initialization function "init_std_dll" to
   setup the stack and eliminate future calls to init_std_dll for other
   functions from this DLL.  */
extern "C" void dll_chain () __asm__ ("dll_chain");

extern "C" {

__asm__ ("								\n\
msg1:									\n\
	.ascii	\"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"\n\
									\n\
	.align	32							\n\
noload:									\n\
	popl	%edx		# Get the address of the information block\n\
	movl	4(%edx),%eax	# Should we 'ignore' the lack		\n\
	test	$1,%eax		#  of this function?			\n\
	jz	1f		# Nope.					\n\
	decl	%eax		# Yes.  This is the # of bytes + 1	\n\
	popl	%edx		# Caller's caller			\n\
	addl	%eax,%esp	# Pop off bytes				\n\
	andl	$0xffff0000,%eax# upper word				\n\
	subl	%eax,%esp	# adjust for possible return value	\n\
	pushl	%eax		# Save for later			\n\
	movl	$127,%eax	# ERROR_PROC_NOT_FOUND			\n\
	pushl	%eax		# First argument			\n\
	call	_SetLastError@4	# Set it				\n\
	popl	%eax		# Get back argument			\n\
	sarl	$16,%eax	# return value in high order word	\n\
	jmp	*%edx		# Return				\n\
1:									\n\
	movl	(%edx),%eax	# Handle value				\n\
	pushl	4(%eax)							\n\
	leal	8(%edx),%eax	# Location of name of function		\n\
	push	%eax							\n\
	push	$msg1		# The message				\n\
	call	___api_fatal	# Print message. Never returns		\n\
									\n\
	.globl	dll_func_load						\n\
dll_func_load:								\n\
	movl	(%esp),%eax	# 'Return address' contains load info	\n\
	addl	$8,%eax		# Address of name of function to load	\n\
	pushl	%eax		# Second argument			\n\
	movl	-8(%eax),%eax	# Where handle lives			\n\
	movl	4(%eax),%eax	# Address of Handle to DLL		\n\
	pushl	%eax		# Handle to DLL				\n\
	call	_GetProcAddress@8# Load it				\n\
	test	%eax,%eax	# Success?				\n\
	jne	gotit		# Yes					\n\
	jmp	noload		# Issue an error or return		\n\
gotit:									\n\
	popl	%edx		# Pointer to 'return address'		\n\
	subl	%edx,%eax	# Make it relative			\n\
	addl	$7,%eax		# Tweak					\n\
	subl	$12,%edx	# Point to jmp				\n\
	movl	%eax,1(%edx)	# Move relative address after jump	\n\
	jmp	*%edx		# Jump to actual function		\n\
									\n\
	.global	dll_chain						\n\
dll_chain:								\n\
	pushl	%eax		# Restore 'return address'		\n\
	jmp	*%edx		# Jump to next init function		\n\
");

/* C representations of the two info blocks described above.
   FIXME: These structures confuse gdb for some reason.  GDB can print
   the whole structure but has problems with the name field? */
struct dll_info
{
  DWORD load_state;
  HANDLE handle;
  LONG here;
  void (*init) ();
  char name[];
};

struct func_info
{
  struct dll_info *dll;
  LONG decoration;
  char name[];
};

/* Mechanism for setting up info for passing to dll_chain routines. */
union retchain
{
  struct {long high; long low;};
  long long ll;
};

/* The standard DLL initialization routine. */
__attribute__ ((used, noinline)) static long long
std_dll_init ()
{
  HANDLE h;
  struct func_info *func = (struct func_info *) __builtin_return_address (0);
  struct dll_info *dll = func->dll;
  retchain ret;

  if (InterlockedIncrement (&dll->here))
    do
      {
	InterlockedDecrement (&dll->here);
	low_priority_sleep (0);
      }
    while (InterlockedIncrement (&dll->here));
  else if (!dll->handle)
    {
      unsigned fpu_control = 0;
      __asm__ __volatile__ ("fnstcw %0": "=m" (fpu_control));
      if ((h = LoadLibrary (dll->name)) != NULL)
	{
	  __asm__ __volatile__ ("fldcw %0": : "m" (fpu_control));
	  dll->handle = h;
	}
      else if (!(func->decoration & 1))
	api_fatal ("could not load %s, %E", dll->name);
      else
	dll->handle = INVALID_HANDLE_VALUE;
    }

  InterlockedDecrement (&dll->here);

  /* Kludge alert.  Redirects the return address to dll_chain. */
  __asm__ __volatile__ ("		\n\
	movl	$dll_chain,4(%ebp)	\n\
  ");

  /* Set "arguments for dll_chain. */
  ret.low = (long) dll->init;
  ret.high = (long) func;
  return ret.ll;
}

/* Initialization function for winsock stuff. */
bool NO_COPY wsock_started = 0;
WSADATA NO_COPY wsadata;
__attribute__ ((used, noinline, regparm(1))) static long long
wsock_init ()
{
  static LONG NO_COPY here = -1L;
  struct func_info *func = (struct func_info *) __builtin_return_address (0);
  struct dll_info *dll = func->dll;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      low_priority_sleep (0);
    }

  if (!wsock_started)
    {
      int (*wsastartup) (int, WSADATA *);

      /* Don't use autoload to load WSAStartup to eliminate recursion. */
      wsastartup = (int (*)(int, WSADATA *))
		   GetProcAddress ((HMODULE) (dll->handle), "WSAStartup");
      if (wsastartup)
	{
	  int res = wsastartup ((2<<8) | 2, &wsadata);

	  debug_printf ("res %d", res);
	  debug_printf ("wVersion %d", wsadata.wVersion);
	  debug_printf ("wHighVersion %d", wsadata.wHighVersion);
	  debug_printf ("szDescription %s", wsadata.szDescription);
	  debug_printf ("szSystemStatus %s", wsadata.szSystemStatus);
	  debug_printf ("iMaxSockets %d", wsadata.iMaxSockets);
	  debug_printf ("iMaxUdpDg %d", wsadata.iMaxUdpDg);
	  debug_printf ("lpVendorInfo %d", wsadata.lpVendorInfo);

	  wsock_started = 1;
	}
    }

  /* Kludge alert.  Redirects the return address to dll_chain. */
  __asm__ __volatile__ ("		\n\
	movl	$dll_chain,4(%ebp)	\n\
  ");

  InterlockedDecrement (&here);

  volatile retchain ret;
  /* Set "arguments for dll_chain. */
  ret.low = (long) dll_func_load;
  ret.high = (long) func;
  return ret.ll;
}

LoadDLLprime (wsock32, _wsock_init)
LoadDLLprime (ws2_32, _wsock_init)

LoadDLLfunc (AccessCheck, 32, advapi32)
LoadDLLfunc (AddAccessAllowedAce, 16, advapi32)
LoadDLLfunc (AddAccessDeniedAce, 16, advapi32)
LoadDLLfunc (AddAce, 20, advapi32)
LoadDLLfunc (AdjustTokenPrivileges, 24, advapi32)
LoadDLLfuncEx (AllocateLocallyUniqueId, 4, advapi32, 1)
LoadDLLfunc (CopySid, 12, advapi32)
LoadDLLfunc (CreateProcessAsUserA, 44, advapi32)
LoadDLLfuncEx (CryptAcquireContextA, 20, advapi32, 1)
LoadDLLfuncEx (CryptGenRandom, 12, advapi32, 1)
LoadDLLfuncEx (CryptReleaseContext, 8, advapi32, 1)
LoadDLLfunc (DeregisterEventSource, 4, advapi32)
LoadDLLfunc (DuplicateToken, 12, advapi32)
LoadDLLfuncEx (DuplicateTokenEx, 24, advapi32, 1)
LoadDLLfunc (EqualSid, 8, advapi32)
LoadDLLfunc (FindFirstFreeAce, 8, advapi32)
LoadDLLfunc (GetAce, 12, advapi32)
LoadDLLfunc (GetFileSecurityA, 20, advapi32)
LoadDLLfunc (GetKernelObjectSecurity, 20, advapi32)
LoadDLLfunc (GetLengthSid, 4, advapi32)
LoadDLLfunc (GetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (GetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (GetSecurityDescriptorOwner, 12, advapi32)
LoadDLLfunc (GetSecurityInfo, 32, advapi32)
LoadDLLfunc (GetSidIdentifierAuthority, 4, advapi32)
LoadDLLfunc (GetSidSubAuthority, 8, advapi32)
LoadDLLfunc (GetSidSubAuthorityCount, 4, advapi32)
LoadDLLfunc (GetTokenInformation, 20, advapi32)
LoadDLLfunc (GetUserNameA, 8, advapi32)
LoadDLLfunc (ImpersonateLoggedOnUser, 4, advapi32)
LoadDLLfunc (ImpersonateNamedPipeClient, 4, advapi32)
LoadDLLfunc (InitializeAcl, 12, advapi32)
LoadDLLfunc (InitializeSecurityDescriptor, 8, advapi32)
LoadDLLfunc (InitializeSid, 12, advapi32)
LoadDLLfunc (IsValidSid, 4, advapi32)
LoadDLLfunc (LogonUserA, 24, advapi32)
LoadDLLfunc (LookupAccountNameA, 28, advapi32)
LoadDLLfunc (LookupAccountNameW, 28, advapi32)
LoadDLLfunc (LookupAccountSidA, 28, advapi32)
LoadDLLfunc (LookupPrivilegeValueA, 12, advapi32)
LoadDLLfunc (LsaClose, 4, advapi32)
LoadDLLfunc (LsaEnumerateAccountRights, 16, advapi32)
LoadDLLfunc (LsaFreeMemory, 4, advapi32)
LoadDLLfunc (LsaNtStatusToWinError, 4, advapi32)
LoadDLLfunc (LsaOpenPolicy, 16, advapi32)
LoadDLLfunc (LsaQueryInformationPolicy, 12, advapi32)
LoadDLLfunc (MakeSelfRelativeSD, 12, advapi32)
LoadDLLfunc (OpenProcessToken, 12, advapi32)
LoadDLLfunc (OpenThreadToken, 16, advapi32)
// LoadDLLfunc (RegCloseKey, 4, advapi32)
LoadDLLfunc (RegCreateKeyExA, 36, advapi32)
LoadDLLfunc (RegDeleteKeyA, 8, advapi32)
LoadDLLfunc (RegDeleteValueA, 8, advapi32)
LoadDLLfunc (RegLoadKeyA, 12, advapi32)
LoadDLLfunc (RegEnumKeyExA, 32, advapi32)
LoadDLLfunc (RegEnumValueA, 32, advapi32)
LoadDLLfunc (RegOpenKeyExA, 20, advapi32)
LoadDLLfunc (RegQueryInfoKeyA, 48, advapi32)
LoadDLLfunc (RegQueryValueExA, 24, advapi32)
LoadDLLfunc (RegSetValueExA, 24, advapi32)
LoadDLLfunc (RegisterEventSourceA, 8, advapi32)
LoadDLLfunc (ReportEventA, 36, advapi32)
LoadDLLfunc (RevertToSelf, 0, advapi32)
LoadDLLfunc (SetKernelObjectSecurity, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorControl, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (SetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorOwner, 12, advapi32)
LoadDLLfunc (SetTokenInformation, 16, advapi32)
LoadDLLfunc (RegGetKeySecurity, 16, advapi32)

/* 127 == ERROR_PROC_NOT_FOUND */
LoadDLLfuncEx2 (DsGetDcNameA, 24, netapi32, 1, 127)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)
LoadDLLfuncEx (NetGetAnyDCName, 12, netapi32, 1)
LoadDLLfuncEx (NetGetDCName, 12, netapi32, 1)
LoadDLLfunc (NetLocalGroupEnum, 28, netapi32)
LoadDLLfunc (NetLocalGroupGetMembers, 32, netapi32)
LoadDLLfunc (NetUserGetGroups, 28, netapi32)
LoadDLLfunc (NetUserGetInfo, 16, netapi32)
LoadDLLfunc (NetWkstaUserGetInfo, 12, netapi32)

/* 0xc000007a == STATUS_PROCEDURE_NOT_FOUND */
#define LoadDLLfuncNt(name, n, dllname) \
  LoadDLLfuncEx2(name, n, dllname, 1, 0xc000007a)
LoadDLLfuncNt (NtClose, 4, ntdll)
LoadDLLfuncNt (NtCreateFile, 44, ntdll)
LoadDLLfuncNt (NtCreateSection, 28, ntdll)
LoadDLLfuncNt (NtCreateToken, 52, ntdll)
LoadDLLfuncNt (NtLockVirtualMemory, 16, ntdll)
LoadDLLfuncNt (NtMapViewOfSection, 40, ntdll)
LoadDLLfuncNt (NtOpenDirectoryObject, 12, ntdll)
LoadDLLfuncNt (NtOpenFile, 24, ntdll)
LoadDLLfuncNt (NtOpenSection, 12, ntdll)
LoadDLLfuncNt (NtQueryDirectoryObject, 28, ntdll)
LoadDLLfuncNt (NtQueryDirectoryFile, 44, ntdll)
LoadDLLfuncNt (NtQueryEaFile, 36, ntdll)
LoadDLLfuncNt (NtQueryInformationFile, 20, ntdll)
LoadDLLfuncNt (NtQueryInformationProcess, 20, ntdll)
LoadDLLfuncNt (NtQueryObject, 20, ntdll)
LoadDLLfuncNt (NtQuerySystemInformation, 16, ntdll)
LoadDLLfuncNt (NtQuerySecurityObject, 20, ntdll)
LoadDLLfuncNt (NtQueryVirtualMemory, 24, ntdll)
LoadDLLfuncNt (NtQueryVolumeInformationFile, 20, ntdll)
LoadDLLfuncNt (NtSetEaFile, 16, ntdll)
LoadDLLfuncNt (NtSetSecurityObject, 12, ntdll)
LoadDLLfuncNt (NtUnlockVirtualMemory, 16, ntdll)
LoadDLLfuncNt (NtUnmapViewOfSection, 8, ntdll)
LoadDLLfuncNt (RtlInitUnicodeString, 8, ntdll)
LoadDLLfuncNt (RtlIsDosDeviceName_U, 4, ntdll)
LoadDLLfuncNt (RtlNtStatusToDosError, 4, ntdll)

LoadDLLfuncEx (EnumProcessModules, 16, psapi, 1)
LoadDLLfuncEx (GetModuleFileNameExA, 16, psapi, 1)
LoadDLLfuncEx (GetModuleInformation, 16, psapi, 1)
LoadDLLfuncEx (GetProcessMemoryInfo, 12, psapi, 1)
LoadDLLfuncEx (QueryWorkingSet, 12, psapi, 1)

LoadDLLfuncEx (LsaDeregisterLogonProcess, 4, secur32, 1)
LoadDLLfuncEx (LsaFreeReturnBuffer, 4, secur32, 1)
LoadDLLfuncEx (LsaLogonUser, 56, secur32, 1)
LoadDLLfuncEx (LsaLookupAuthenticationPackage, 12, secur32, 1)
LoadDLLfuncEx (LsaRegisterLogonProcess, 12, secur32, 1)

LoadDLLfunc (CharToOemA, 8, user32)
LoadDLLfunc (CharToOemBuffA, 12, user32)
LoadDLLfunc (CloseClipboard, 0, user32)
LoadDLLfunc (CloseWindowStation, 4, user32)
LoadDLLfunc (CreateWindowExA, 48, user32)
LoadDLLfunc (CreateWindowStationA, 16, user32)
LoadDLLfunc (DefWindowProcA, 16, user32)
LoadDLLfunc (DispatchMessageA, 4, user32)
LoadDLLfunc (EmptyClipboard, 0, user32)
LoadDLLfunc (FindWindowA, 8, user32)
LoadDLLfunc (GetClipboardData, 4, user32)
LoadDLLfunc (GetForegroundWindow, 0, user32)
LoadDLLfunc (GetKeyboardLayout, 4, user32)
LoadDLLfunc (GetMessageA, 16, user32)
LoadDLLfunc (GetPriorityClipboardFormat, 8, user32)
LoadDLLfunc (GetProcessWindowStation, 0, user32)
LoadDLLfunc (GetThreadDesktop, 4, user32)
LoadDLLfunc (GetWindowThreadProcessId, 8, user32)
LoadDLLfunc (GetUserObjectInformationA, 20, user32)
LoadDLLfunc (KillTimer, 8, user32)
LoadDLLfunc (MessageBeep, 4, user32)
LoadDLLfunc (MessageBoxA, 16, user32)
LoadDLLfunc (MsgWaitForMultipleObjects, 20, user32)
LoadDLLfunc (OemToCharBuffA, 12, user32)
LoadDLLfunc (OpenClipboard, 4, user32)
LoadDLLfunc (PeekMessageA, 20, user32)
LoadDLLfunc (PostMessageA, 16, user32)
LoadDLLfunc (PostQuitMessage, 4, user32)
LoadDLLfunc (RegisterClassA, 4, user32)
LoadDLLfunc (RegisterClipboardFormatA, 4, user32)
LoadDLLfunc (SendMessageA, 16, user32)
LoadDLLfunc (SetClipboardData, 8, user32)
LoadDLLfunc (SetProcessWindowStation, 4, user32)
LoadDLLfunc (SetTimer, 16, user32)
LoadDLLfunc (SetUserObjectSecurity, 12, user32)

LoadDLLfunc (rcmd, 24, wsock32)

LoadDLLfunc (accept, 12, ws2_32)
LoadDLLfunc (bind, 12, ws2_32)
LoadDLLfunc (closesocket, 4, ws2_32)
LoadDLLfunc (connect, 12, ws2_32)
LoadDLLfunc (gethostbyaddr, 12, ws2_32)
LoadDLLfunc (gethostbyname, 4, ws2_32)
LoadDLLfuncEx2 (gethostname, 8, ws2_32, 1, 1)
LoadDLLfunc (getpeername, 12, ws2_32)
LoadDLLfunc (getprotobyname, 4, ws2_32)
LoadDLLfunc (getprotobynumber, 4, ws2_32)
LoadDLLfunc (getservbyname, 8, ws2_32)
LoadDLLfunc (getservbyport, 8, ws2_32)
LoadDLLfunc (getsockname, 12, ws2_32)
LoadDLLfunc (getsockopt, 20, ws2_32)
LoadDLLfunc (ioctlsocket, 12, ws2_32)
LoadDLLfunc (listen, 8, ws2_32)
LoadDLLfunc (recv, 16, ws2_32)
LoadDLLfunc (recvfrom, 24, ws2_32)
LoadDLLfunc (select, 20, ws2_32)
LoadDLLfunc (send, 16, ws2_32)
LoadDLLfunc (sendto, 24, ws2_32)
LoadDLLfunc (setsockopt, 20, ws2_32)
LoadDLLfunc (shutdown, 8, ws2_32)
LoadDLLfunc (socket, 12, ws2_32)
LoadDLLfunc (WSAAsyncSelect, 16, ws2_32)
LoadDLLfunc (WSACloseEvent, 4, ws2_32)
LoadDLLfunc (WSACreateEvent, 0, ws2_32)
LoadDLLfunc (WSADuplicateSocketA, 12, ws2_32)
LoadDLLfunc (WSAGetLastError, 0, ws2_32)
LoadDLLfunc (WSAGetOverlappedResult, 20, ws2_32)
LoadDLLfunc (WSARecv, 28, ws2_32)
LoadDLLfunc (WSARecvFrom, 36, ws2_32)
LoadDLLfunc (WSASend, 28, ws2_32)
LoadDLLfunc (WSASendTo, 36, ws2_32)
LoadDLLfunc (WSASetEvent, 4, ws2_32)
LoadDLLfunc (WSASetLastError, 4, ws2_32)
LoadDLLfunc (WSASocketA, 24, ws2_32)
// LoadDLLfunc (WSAStartup, 8, ws2_32)
LoadDLLfunc (WSAWaitForMultipleEvents, 20, ws2_32)
LoadDLLfunc (WSAEventSelect, 12, ws2_32)
LoadDLLfunc (WSAEnumNetworkEvents, 12, ws2_32)
LoadDLLfunc (__WSAFDIsSet, 8, ws2_32)

LoadDLLfuncEx (GetIfTable, 12, iphlpapi, 1)
LoadDLLfuncEx (GetIfEntry, 4, iphlpapi, 1)
LoadDLLfuncEx (GetIpAddrTable, 12, iphlpapi, 1)
LoadDLLfuncEx (GetNetworkParams, 8, iphlpapi, 1)
LoadDLLfuncEx (GetTcpTable, 12, iphlpapi, 1)

LoadDLLfunc (CoTaskMemFree, 4, ole32)

LoadDLLfuncEx (CancelIo, 4, kernel32, 1)
LoadDLLfuncEx (CreateHardLinkA, 12, kernel32, 1)
LoadDLLfuncEx (CreateToolhelp32Snapshot, 8, kernel32, 1)
LoadDLLfuncEx (FindFirstVolumeA, 8, kernel32, 1)
LoadDLLfuncEx (FindNextVolumeA, 12, kernel32, 1)
LoadDLLfuncEx (FindVolumeClose, 4, kernel32, 1)
LoadDLLfuncEx2 (GetCompressedFileSizeA, 8, kernel32, 1, 0xffffffff)
LoadDLLfuncEx (GetConsoleWindow, 0, kernel32, 1)
LoadDLLfuncEx (GetDiskFreeSpaceEx, 16, kernel32, 1)
LoadDLLfuncEx (GetNativeSystemInfo, 4, kernel32, 1)
LoadDLLfuncEx (GetProcessWorkingSetSize, 12, kernel32, 1)
LoadDLLfuncEx (GetVolumeNameForVolumeMountPointA, 12, kernel32, 1)
LoadDLLfuncEx2 (IsDebuggerPresent, 0, kernel32, 1, 1)
LoadDLLfunc (IsProcessorFeaturePresent, 4, kernel32);
LoadDLLfuncEx (IsWow64Process, 8, kernel32, 1);
LoadDLLfuncEx (Process32First, 8, kernel32, 1)
LoadDLLfuncEx (Process32Next, 8, kernel32, 1)
LoadDLLfuncEx (RegisterServiceProcess, 8, kernel32, 1)
LoadDLLfuncEx (SetProcessWorkingSetSize, 12, kernel32, 1)
LoadDLLfuncEx (SignalObjectAndWait, 16, kernel32, 1)
LoadDLLfuncEx (SwitchToThread, 0, kernel32, 1)

LoadDLLfunc (SHGetDesktopFolder, 4, shell32)

LoadDLLfuncEx (waveOutGetNumDevs, 0, winmm, 1)
LoadDLLfuncEx (waveOutOpen, 24, winmm, 1)
LoadDLLfuncEx (waveOutReset, 4, winmm, 1)
LoadDLLfuncEx (waveOutClose, 4, winmm, 1)
LoadDLLfuncEx (waveOutGetVolume, 8, winmm, 1)
LoadDLLfuncEx (waveOutSetVolume, 8, winmm, 1)
LoadDLLfuncEx (waveOutUnprepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveOutPrepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveOutWrite, 12, winmm, 1)
LoadDLLfuncEx (timeGetDevCaps, 8, winmm, 1)
LoadDLLfuncEx (timeGetTime, 0, winmm, 1)
LoadDLLfuncEx (timeBeginPeriod, 4, winmm, 1)
LoadDLLfuncEx (timeEndPeriod, 4, winmm, 1)

LoadDLLfuncEx (waveInGetNumDevs, 0, winmm, 1)
LoadDLLfuncEx (waveInOpen, 24, winmm, 1)
LoadDLLfuncEx (waveInUnprepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveInPrepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveInAddBuffer, 12, winmm, 1)
LoadDLLfuncEx (waveInStart, 4, winmm, 1)
LoadDLLfuncEx (waveInReset, 4, winmm, 1)
LoadDLLfuncEx (waveInClose, 4, winmm, 1)

LoadDLLfunc (WNetGetResourceInformationA, 16, mpr)
LoadDLLfunc (WNetGetResourceParentA, 12, mpr)
LoadDLLfunc (WNetOpenEnumA, 20, mpr)
LoadDLLfunc (WNetEnumResourceA, 16, mpr)
LoadDLLfunc (WNetCloseEnum, 4, mpr)

LoadDLLfuncEx (UuidCreate, 4, rpcrt4, 1)
LoadDLLfuncEx (UuidCreateSequential, 4, rpcrt4, 1)
}
