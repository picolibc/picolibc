/* autoload.cc: all dynamic load stuff.

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "autoload.h"

extern "C" {

/* This struct is unused, but it illustrates the layout of a DLL
   information block. */
struct DLLinfo
{
  char jmpinst[4];
  HANDLE h;
  DWORD flag;
  char name[0];
};

/* FIXME: This is not thread-safe! */
__asm__ ("\n\
msg1:\n\
  .ascii \"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"\n\
\n\
  .align 32\n\
noload:\n\
  popl %edx		# Get the address of the information block\n\
  movl 8(%edx),%eax	# Should we 'ignore' the lack\n\
  test $1,%eax		#  of this function?\n\
  jz 1f			# Nope.\n\
  decl %eax		# Yes.  This is the # of bytes + 1\n\
  popl %edx		# Caller's caller\n\
  addl %eax,%esp	# Pop off bytes\n\
  xor %eax,%eax		# Zero functional return\n\
  jmp *%edx		# Return\n\
1:\n\
  movl 4(%edx),%eax	# Handle value\n\
  pushl (%eax)\n\
  leal 12(%edx),%eax	# Location of name of function\n\
  push %eax\n\
  push $msg1		# The message\n\
  call ___api_fatal	# Print message. Never returns\n\
\n\
  .globl cygwin_dll_func_load\n\
cygwin_dll_func_load:\n\
  movl (%esp),%eax	# 'Return address' contains load info\n\
  addl $12,%eax		# Address of name of function to load\n\
  pushl %eax		# Second argument\n\
  movl -8(%eax),%eax	# Address of Handle to DLL\n\
  pushl (%eax)		# Handle to DLL\n\
  call _GetProcAddress@8# Load it\n\
  test %eax,%eax	# Success?\n\
  jne gotit		# Yes\n\
  jmp noload		# Issue an error or return\n\
gotit:\n\
  popl %ecx		# Pointer to 'return address'\n\
  movb $0xe0,-1(%ecx)	# Turn preceding call to a jmp *%eax\n\
  movl %eax,(%ecx)	# Point dispatch to address loaded above\n\
  jmp *%eax\n\
");

int
std_dll_init (HANDLE &dll_handle, const char *dll_name, LONG &here)
{
  HANDLE h;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      Sleep (0);
    }

  if (dll_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary (dll_name)) != NULL)
    dll_handle = h;
  else
    api_fatal ("could not load %s, %E", dll_name);

  InterlockedDecrement (&here);
  return 0;
}

LoadDLLinitfunc (advapi32)
LoadDLLinitfunc (netapi32)
LoadDLLinitfunc (ntdll)
LoadDLLinitfunc (secur32)
LoadDLLinitfunc (user32)
LoadDLLinitfunc (iphlpapi)
LoadDLLinitfunc (ole32)
LoadDLLinitfunc (kernel32)
LoadDLLinitfunc (winmm)

extern void wsock_init ();

LoadDLLinitfuncdef (wsock32)
{
  LoadDLLstdfunc (wsock32);
  wsock_init ();
  return 0;
}

LoadDLLinitfuncdef (ws2_32)
{
  LoadDLLstdfunc (ws2_32);
  wsock_init ();
  return 0;
}

static void __stdcall dummy_autoload (void) __attribute__ ((unused));
static void __stdcall
dummy_autoload (void)
{
LoadDLLinit (advapi32)
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

LoadDLLinit (netapi32)
LoadDLLfunc (NetWkstaUserGetInfo, 12, netapi32)
LoadDLLfunc (NetUserGetInfo, 16, netapi32)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)

LoadDLLinit (ntdll)
LoadDLLfuncEx (NtMapViewOfSection, 40, ntdll, 1)
LoadDLLfuncEx (NtOpenSection, 12, ntdll, 1)
LoadDLLfuncEx (NtQuerySystemInformation, 16, ntdll, 1)
LoadDLLfuncEx (NtUnmapViewOfSection, 8, ntdll, 1)
LoadDLLfuncEx (RtlInitUnicodeString, 8, ntdll, 1)
LoadDLLfuncEx (RtlNtStatusToDosError, 4, ntdll, 1)
LoadDLLfuncEx (ZwQuerySystemInformation, 16, ntdll, 1)

LoadDLLinit (secur32)
LoadDLLfuncEx (LsaDeregisterLogonProcess, 4, secur32, 1)
LoadDLLfuncEx (LsaFreeReturnBuffer, 4, secur32, 1)
LoadDLLfuncEx (LsaLogonUser, 56, secur32, 1)
LoadDLLfuncEx (LsaLookupAuthenticationPackage, 12, secur32, 1)
LoadDLLfuncEx (LsaRegisterLogonProcess, 12, secur32, 1)

LoadDLLinit (user32)
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

LoadDLLinit (wsock32)
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

LoadDLLinit (ws2_32)
LoadDLLfuncEx (WSADuplicateSocketA, 12, ws2_32, 1)
LoadDLLfuncEx (WSASocketA, 24, ws2_32, 1)

LoadDLLinit (iphlpapi)
LoadDLLfuncEx (GetIfTable, 12, iphlpapi, 1)
LoadDLLfuncEx (GetIpAddrTable, 12, iphlpapi, 1)

LoadDLLinit (ole32)
LoadDLLfunc (CoInitialize, 4, ole32)
LoadDLLfunc (CoUninitialize, 0, ole32)
LoadDLLfunc (CoCreateInstance, 20, ole32)

LoadDLLinit (kernel32)
LoadDLLfuncEx (SignalObjectAndWait, 16, kernel32, 1)

LoadDLLinit (winmm)
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
}
