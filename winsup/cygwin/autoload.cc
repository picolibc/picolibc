/* autoload.cc: all dynamic load stuff.

   Copyright 2000, 2001 Red Hat, Inc.

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
  .section	." #dllname "_info,\"w\"		\n\
  .linkonce						\n\
  .long		std_dll_init				\n\
  .long		0					\n\
  .long		-1					\n\
  .long		" #init_also "				\n\
  .asciz	\"" #dllname "\"			\n\
  .text							\n\
");

/* Create a "decorated" name */
#define mangle(name, n) #name "@" #n

/* Standard DLL load macro.  Invokes a fatal warning if the function isn't
   found. */
#define LoadDLLfunc(name, n, dllname) LoadDLLfuncEx (name, n, dllname, 0)

/* Main DLL setup stuff. */
#define LoadDLLfuncEx(name, n, dllname, notimp) \
  LoadDLLprime (dllname, dll_func_load)			\
  __asm__ ("						\n\
  .section	." #dllname "_text,\"wx\"		\n\
  .global	_" mangle (name, n) "			\n\
  .global	_win32_" mangle (name, n) "		\n\
  .align	8					\n\
_" mangle (name, n) ":					\n\
_win32_" mangle (name, n) ":				\n\
  movl		(1f),%eax				\n\
  call		*(%eax)					\n\
1:.long		." #dllname "_info			\n\
  .long		" #n "+" #notimp "			\n\
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

/* called by the secondary initialization function to call dll_func_load. */
extern "C" void dll_chain1 () __asm__ ("dll_chain1");

extern "C" {

/* FIXME: This is not thread-safe? */
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
	movl	$127,%eax	# ERROR_PROC_NOT_FOUND			\n\
	pushl	%eax		# First argument			\n\
	call	_SetLastError@4	# Set it				\n\
	xor	%eax,%eax	# Zero functional return		\n\
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
	popl	%ecx		# Pointer to 'return address'		\n\
	movb	$0xe9,-7(%ecx)	# Turn preceding call to a jmp *%eax	\n\
	movl	%eax,%edx	# Save					\n\
	subl	%ecx,%eax	# Make it relative			\n\
	addl	$2,%eax		# Tweak					\n\
	movl	%eax,-6(%ecx)	# Move relative address after jump	\n\
	jmp	*%edx		# Jump to actual function		\n\
									\n\
	.global	dll_chain						\n\
dll_chain:								\n\
	pushl	%eax		# Restore 'return address'		\n\
	movl	(%eax),%eax	# Get address of DLL info block		\n\
	movl	$dll_func_load,(%eax) # Just load func now		\n\
	jmp	*%edx		# Jump to next init function		\n\
									\n\
dll_chain1:								\n\
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
static long long std_dll_init () __asm__ ("std_dll_init") __attribute__ ((unused));
static long long
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
	Sleep (0);
      }
    while (InterlockedIncrement (&dll->here));
  else if (!dll->handle)
    {
      if ((h = LoadLibrary (dll->name)) != NULL)
	dll->handle = h;
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
static long long wsock_init () __asm__ ("wsock_init") __attribute__ ((unused, regparm(1)));
bool NO_COPY wsock_started = 0;
static long long
wsock_init ()
{
  static LONG NO_COPY here = -1L;
  extern WSADATA wsadata;
  struct func_info *func = (struct func_info *) __builtin_return_address (0);
  struct dll_info *dll = func->dll;
  retchain ret;

  __asm__ ("						\n\
	.section .ws2_32_info				\n\
	.equ	_ws2_32_handle,.ws2_32_info + 4		\n\
	.global _ws2_32_handle				\n\
	.section .wsock32_info				\n\
	.equ	_wsock32_handle,.wsock32_info + 4	\n\
	.global	_wsock32_handle				\n\
	.text						\n\
  ");

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      Sleep (0);
    }

  if (!wsock_started && (wsock32_handle || ws2_32_handle))
    {
      /* Don't use autoload to load WSAStartup to eliminate recursion. */
      int (*wsastartup) (int, WSADATA *);

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

  InterlockedDecrement (&here);

  /* Kludge alert.  Redirects the return address to dll_chain1. */
  __asm__ __volatile__ ("		\n\
	movl	$dll_chain1,4(%ebp)	\n\
  ");

  /* Set "arguments for dll_chain1. */
  ret.low = (long) dll_func_load;
  ret.high = (long) func;
  return ret.ll;
}

LoadDLLprime (wsock32, wsock_init)
LoadDLLprime (ws2_32, wsock_init)

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
LoadDLLfuncEx (DuplicateTokenEx, 24, advapi32, 1)
LoadDLLfunc (EqualSid, 8, advapi32)
LoadDLLfunc (GetAce, 12, advapi32)
LoadDLLfunc (GetFileSecurityA, 20, advapi32)
LoadDLLfunc (GetLengthSid, 4, advapi32)
LoadDLLfunc (GetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (GetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (GetSecurityDescriptorOwner, 12, advapi32)
LoadDLLfunc (GetSidIdentifierAuthority, 4, advapi32)
LoadDLLfunc (GetSidSubAuthority, 8, advapi32)
LoadDLLfunc (GetSidSubAuthorityCount, 4, advapi32)
LoadDLLfunc (GetTokenInformation, 20, advapi32)
LoadDLLfunc (GetUserNameA, 8, advapi32)
LoadDLLfunc (ImpersonateLoggedOnUser, 4, advapi32)
LoadDLLfunc (InitializeAcl, 12, advapi32)
LoadDLLfunc (InitializeSecurityDescriptor, 8, advapi32)
LoadDLLfunc (InitializeSid, 12, advapi32)
LoadDLLfunc (IsValidSid, 4, advapi32)
LoadDLLfunc (LogonUserA, 24, advapi32)
LoadDLLfunc (LookupAccountNameA, 28, advapi32)
LoadDLLfunc (LookupAccountSidA, 28, advapi32)
LoadDLLfunc (LookupPrivilegeValueA, 12, advapi32)
LoadDLLfuncEx (LsaNtStatusToWinError, 4, advapi32, 1)
LoadDLLfunc (MakeSelfRelativeSD, 12, advapi32)
LoadDLLfunc (OpenProcessToken, 12, advapi32)
LoadDLLfunc (RegCloseKey, 4, advapi32)
LoadDLLfunc (RegCreateKeyExA, 36, advapi32)
LoadDLLfunc (RegDeleteKeyA, 8, advapi32)
LoadDLLfunc (RegDeleteValueA, 8, advapi32)
LoadDLLfunc (RegLoadKeyA, 12, advapi32)
LoadDLLfunc (RegEnumKeyExA, 32, advapi32)
LoadDLLfunc (RegEnumValueA, 32, advapi32)
LoadDLLfunc (RegOpenKeyExA, 20, advapi32)
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

LoadDLLfunc (NetWkstaUserGetInfo, 12, netapi32)
LoadDLLfunc (NetUserGetInfo, 16, netapi32)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)

LoadDLLfuncEx (NtMapViewOfSection, 40, ntdll, 1)
LoadDLLfuncEx (NtOpenSection, 12, ntdll, 1)
LoadDLLfuncEx (NtQuerySystemInformation, 16, ntdll, 1)
LoadDLLfuncEx (NtUnmapViewOfSection, 8, ntdll, 1)
LoadDLLfuncEx (RtlInitUnicodeString, 8, ntdll, 1)
LoadDLLfuncEx (RtlNtStatusToDosError, 4, ntdll, 1)
LoadDLLfuncEx (ZwQuerySystemInformation, 16, ntdll, 1)

LoadDLLfuncEx (LsaDeregisterLogonProcess, 4, secur32, 1)
LoadDLLfuncEx (LsaFreeReturnBuffer, 4, secur32, 1)
LoadDLLfuncEx (LsaLogonUser, 56, secur32, 1)
LoadDLLfuncEx (LsaLookupAuthenticationPackage, 12, secur32, 1)
LoadDLLfuncEx (LsaRegisterLogonProcess, 12, secur32, 1)

LoadDLLfunc (CharToOemA, 8, user32)
LoadDLLfunc (CharToOemBuffA, 12, user32)
LoadDLLfunc (CloseClipboard, 0, user32)
LoadDLLfunc (CreateWindowExA, 48, user32)
LoadDLLfunc (DefWindowProcA, 16, user32)
LoadDLLfunc (DispatchMessageA, 4, user32)
LoadDLLfunc (EmptyClipboard, 0, user32)
LoadDLLfunc (FindWindowA, 8, user32)
LoadDLLfunc (GetClipboardData, 4, user32)
LoadDLLfunc (GetKeyboardLayout, 4, user32)
LoadDLLfunc (GetMessageA, 16, user32)
LoadDLLfunc (GetPriorityClipboardFormat, 8, user32)
LoadDLLfunc (GetProcessWindowStation, 0, user32)
LoadDLLfunc (GetThreadDesktop, 4, user32)
LoadDLLfunc (GetUserObjectInformationA, 20, user32)
LoadDLLfunc (KillTimer, 8, user32)
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
LoadDLLfunc (SetTimer, 16, user32)
LoadDLLfunc (SetUserObjectSecurity, 12, user32)

LoadDLLfunc (WSAAsyncSelect, 16, wsock32)
LoadDLLfunc (WSACleanup, 0, wsock32)
LoadDLLfunc (WSAGetLastError, 0, wsock32)
LoadDLLfunc (WSASetLastError, 4, wsock32)
LoadDLLfunc (WSAStartup, 8, wsock32)
LoadDLLfunc (__WSAFDIsSet, 8, wsock32)
LoadDLLfunc (accept, 12, wsock32)
LoadDLLfunc (bind, 12, wsock32)
LoadDLLfunc (closesocket, 4, wsock32)
LoadDLLfunc (connect, 12, wsock32)
LoadDLLfunc (gethostbyaddr, 12, wsock32)
LoadDLLfunc (gethostbyname, 4, wsock32)
LoadDLLfunc (gethostname, 8, wsock32)
LoadDLLfunc (getpeername, 12, wsock32)
LoadDLLfunc (getprotobyname, 4, wsock32)
LoadDLLfunc (getprotobynumber, 4, wsock32)
LoadDLLfunc (getservbyname, 8, wsock32)
LoadDLLfunc (getservbyport, 8, wsock32)
LoadDLLfunc (getsockname, 12, wsock32)
LoadDLLfunc (getsockopt, 20, wsock32)
LoadDLLfunc (inet_addr, 4, wsock32)
LoadDLLfunc (inet_network, 4, wsock32)
LoadDLLfunc (inet_ntoa, 4, wsock32)
LoadDLLfunc (ioctlsocket, 12, wsock32)
LoadDLLfunc (listen, 8, wsock32)
LoadDLLfunc (rcmd, 24, wsock32)
LoadDLLfunc (recv, 16, wsock32)
LoadDLLfunc (recvfrom, 24, wsock32)
LoadDLLfunc (rexec, 24, wsock32)
LoadDLLfunc (rresvport, 4, wsock32)
LoadDLLfunc (select, 20, wsock32)
LoadDLLfunc (send, 16, wsock32)
LoadDLLfunc (sendto, 24, wsock32)
LoadDLLfunc (setsockopt, 20, wsock32)
LoadDLLfunc (shutdown, 8, wsock32)
LoadDLLfunc (socket, 12, wsock32)

LoadDLLfuncEx (WSACloseEvent, 4, ws2_32, 1)
LoadDLLfuncEx (WSACreateEvent, 0, ws2_32, 1)
LoadDLLfuncEx (WSADuplicateSocketA, 12, ws2_32, 1)
LoadDLLfuncEx (WSAGetOverlappedResult, 20, ws2_32, 1)
LoadDLLfuncEx (WSARecv, 28, ws2_32, 1)
LoadDLLfuncEx (WSARecvFrom, 36, ws2_32, 1)
LoadDLLfuncEx (WSASend, 28, ws2_32, 1)
LoadDLLfuncEx (WSASendTo, 36, ws2_32, 1)
LoadDLLfuncEx (WSASetEvent, 4, ws2_32, 1)
LoadDLLfuncEx (WSASocketA, 24, ws2_32, 1)
LoadDLLfuncEx (WSAWaitForMultipleEvents, 20, ws2_32, 1)

LoadDLLfuncEx (GetIfTable, 12, iphlpapi, 1)
LoadDLLfuncEx (GetIpAddrTable, 12, iphlpapi, 1)

LoadDLLfunc (CoInitialize, 4, ole32)
LoadDLLfunc (CoUninitialize, 0, ole32)
LoadDLLfunc (CoCreateInstance, 20, ole32)

LoadDLLfuncEx (SignalObjectAndWait, 16, kernel32, 1)

LoadDLLfuncEx (waveOutGetNumDevs, 0, winmm, 1)
LoadDLLfuncEx (waveOutOpen, 24, winmm, 1)
LoadDLLfuncEx (waveOutReset, 4, winmm, 1)
LoadDLLfuncEx (waveOutClose, 4, winmm, 1)
LoadDLLfuncEx (waveOutGetVolume, 8, winmm, 1)
LoadDLLfuncEx (waveOutSetVolume, 8, winmm, 1)
LoadDLLfuncEx (waveOutUnprepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveOutPrepareHeader, 12, winmm, 1)
LoadDLLfuncEx (waveOutWrite, 12, winmm, 1)
}
