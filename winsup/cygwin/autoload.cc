/* autoload.cc: all dynamic load stuff.

   Copyright 2000 Cygnus Solutions.

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
__asm__ ("
msg1:
  .ascii \"couldn't dynamically determine load address for '%s' (handle %p), %E\\0\"

  .align 32
noload:
  popl %edx		# Get the address of the information block
  movl 8(%edx),%eax	# Should we 'ignore' the lack
  test $1,%eax		#  of this function?
  jz 1f			# Nope.
  decl %eax		# Yes.  This is the # of bytes + 1
  popl %edx		# Caller's caller
  addl %eax,%esp	# Pop off bytes
  xor %eax,%eax		# Zero functional return
  jmp *%edx		# Return
1:
  movl 4(%edx),%eax	# Handle value
  pushl (%eax)
  leal 12(%edx),%eax	# Location of name of function
  push %eax
  push $msg1		# The message
  call ___api_fatal	# Print message. Never returns

  .globl cygwin_dll_func_load
cygwin_dll_func_load:
  movl (%esp),%eax	# 'Return address' contains load info
  addl $12,%eax		# Address of name of function to load
  pushl %eax		# Second argument
  movl -8(%eax),%eax	# Address of Handle to DLL
  pushl (%eax)		# Handle to DLL
  call _GetProcAddress@8# Load it
  test %eax,%eax	# Success?
  jne gotit		# Yes
  jmp noload		# Issue an error or return
gotit:
  popl %ecx		# Pointer to 'return address'
  movb $0xe0,-1(%ecx)	# Turn preceding call to a jmp *%eax
  movl %eax,(%ecx)	# Point dispatch to address loaded above
  jmp *%eax
");

LoadDLLinitfunc (advapi32)
{
  HANDLE h;
  static NO_COPY LONG here = -1L;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      Sleep (0);
    }

  if (advapi32_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary ("advapi32.dll")) != NULL)
    advapi32_handle = h;
  else if (!advapi32_handle)
    api_fatal ("could not load advapi32.dll, %E");

  InterlockedDecrement (&here);
  return 0;
}

LoadDLLinitfunc (netapi32)
{
  HANDLE h;

  if ((h = LoadLibrary ("netapi32.dll")) != NULL)
    netapi32_handle = h;
  else if (! netapi32_handle)
    api_fatal ("could not load netapi32.dll. %d", GetLastError ());
  return 0;
}

LoadDLLinitfunc (ntdll)
{
  HANDLE h;
  static NO_COPY LONG here = -1L;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
small_printf ("Multiple tries to read ntdll.dll\n");
      Sleep (0);
    }

  if (ntdll_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary ("ntdll.dll")) != NULL)
    ntdll_handle = h;
  else if (!ntdll_handle)
    api_fatal ("could not load ntdll.dll, %E");

  InterlockedDecrement (&here);
  return 0;
}

LoadDLLinitfunc (user32)
{
  HANDLE h;
  static NO_COPY LONG here = -1L;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      Sleep (0);
    }

  if (user32_handle)
    /* nothing to do */;
  else if ((h = LoadLibrary ("user32.dll")) != NULL)
    user32_handle = h;
  else if (!user32_handle)
    api_fatal ("could not load user32.dll, %E");

  InterlockedDecrement (&here);
  return 0;		/* Already done by another thread? */
}

LoadDLLinitfunc (wsock32)
{
  extern void wsock_init ();
  HANDLE h;

  if ((h = LoadLibrary ("wsock32.dll")) != NULL)
    wsock32_handle = h;
  else if (!wsock32_handle)
    api_fatal ("could not load wsock32.dll.  Is TCP/IP installed?");
  else
    return 0;		/* Already done by another thread? */

  if (!ws2_32_handle)
    wsock_init ();

  return 0;
}

LoadDLLinitfunc (ws2_32)
{
  extern void wsock_init ();
  HANDLE h;

  if ((h = LoadLibrary ("ws2_32.dll")) == NULL)
    return 0;          /* Already done or not available. */
  ws2_32_handle = h;

  if (!wsock32_handle)
    wsock_init ();

  return 0;
}

static void dummy_autoload (void) __attribute__ ((unused));
static void
dummy_autoload (void)
{
LoadDLLinit (advapi32)
LoadDLLfunc (AddAccessAllowedAce, 16, advapi32)
LoadDLLfunc (AddAccessDeniedAce, 16, advapi32)
LoadDLLfunc (AddAce, 20, advapi32)
LoadDLLfunc (AdjustTokenPrivileges, 24, advapi32)
LoadDLLfunc (CopySid, 12, advapi32)
LoadDLLfunc (CreateProcessAsUserA, 44, advapi32)
LoadDLLfuncEx (CryptAcquireContextA, 20, advapi32, 1)
LoadDLLfuncEx (CryptGenRandom, 12, advapi32, 1)
LoadDLLfuncEx (CryptReleaseContext, 8, advapi32, 1)
LoadDLLfunc (DeregisterEventSource, 4, advapi32)
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
LoadDLLfunc (SetSecurityDescriptorDacl, 16, advapi32)
LoadDLLfunc (SetSecurityDescriptorGroup, 12, advapi32)
LoadDLLfunc (SetSecurityDescriptorOwner, 12, advapi32)

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

LoadDLLinit (user32)
LoadDLLfunc (CharToOemBuffA, 12, user32)
LoadDLLfunc (CloseClipboard, 0, user32)
LoadDLLfunc (CreateWindowExA, 48, user32)
LoadDLLfunc (DefWindowProcA, 16, user32)
LoadDLLfunc (DispatchMessageA, 4, user32)
LoadDLLfunc (FindWindowA, 8, user32)
LoadDLLfunc (GetClipboardData, 4, user32)
LoadDLLfunc (GetMessageA, 16, user32)
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
LoadDLLfunc (SendMessageA, 16, user32)
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
}
}
