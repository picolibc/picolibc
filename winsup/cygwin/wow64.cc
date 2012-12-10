/* wow64.cc

   Copyright 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __x86_64__
/* WOW64 only plays a role in the 32 bit version.  Don't use any of this
   in the 64 bit version. */

#include "winsup.h"
#include "cygtls.h"
#include "ntdll.h"
#include <sys/param.h>

#define PTR_ADD(p,o)	((PVOID)((PBYTE)(p)+(o)))

bool NO_COPY wow64_needs_stack_adjustment = false;

static void
wow64_eval_expected_main_stack (PVOID &allocbase, PVOID &stackbase)
{
  PIMAGE_DOS_HEADER dosheader;
  PIMAGE_NT_HEADERS32 ntheader;
  DWORD size;

  dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
  ntheader = (PIMAGE_NT_HEADERS32) ((PBYTE) dosheader + dosheader->e_lfanew);
  /* The main thread stack is expected to be located at 0x30000, which is the
     case for all observed NT systems up to Server 2003 R2, unless the
     stacksize requested by the StackReserve field in the PE/COFF header is
     so big that the stack doesn't fit in the area between 0x30000 and the
     start of the image.  In case of a conflict, the OS allocates the stack
     right after the image.
     Sidenote: While post-2K3 32 bit systems continue to have the default
     main thread stack address located at 0x30000, the default main thread
     stack address on Vista/2008 64 bit is 0x80000 and on W7/2K8R2 64 bit
     it is 0x90000.  However, this is no problem because the system sticks
     to that address for all WOW64 processes, not only for the first one
     started from a 64 bit parent. */
  allocbase = (PVOID) 0x30000;
  /* Stack size.  The OS always rounds the size up to allocation granularity
     and it never allocates less than 256K. */
  size = roundup2 (ntheader->OptionalHeader.SizeOfStackReserve,
		   wincap.allocation_granularity ());
  if (size < 256 * 1024)
    size = 256 * 1024;
  /* If the stack doesn't fit in the area before the image, it's allocated
     right after the image, rounded up to allocation granularity boundary. */
  if (PTR_ADD (allocbase, size) > (PVOID) ntheader->OptionalHeader.ImageBase)
    allocbase = PTR_ADD (ntheader->OptionalHeader.ImageBase,
			 ntheader->OptionalHeader.SizeOfImage);
  allocbase = (PVOID) roundup2 ((uintptr_t) allocbase,
				wincap.allocation_granularity ());
  stackbase = PTR_ADD (allocbase, size);
  debug_printf ("expected allocbase: %p, stackbase: %p", allocbase, stackbase);
}

bool
wow64_test_for_64bit_parent ()
{
  /* On Windows XP 64 and 2003 64 there's a problem with processes running
     under WOW64.  The first process started from a 64 bit process has its
     main thread stack not where it should be.  Rather, it uses another
     stack while the original stack is used for other purposes.
     The problem is, the child has its stack in the usual spot again, thus
     we have to "alloc_stack_hard_way".  However, this fails in almost all
     cases because the stack slot of the parent process is taken by something
     else in the child process.
     What we do here is to check if the current stack is the excpected main
     thread stack and if not, if we really have been started from a 64 bit
     process here.  If so, we note this fact in wow64_needs_stack_adjustment
     so we can workaround the stack problem in _dll_crt0.  See there for how
     we go along. */
  NTSTATUS ret;
  PROCESS_BASIC_INFORMATION pbi;
  HANDLE parent;
  PVOID allocbase, stackbase;

  ULONG_PTR wow64 = TRUE;   /* Opt on the safe side. */

  /* First check if the current stack is where it belongs.  If so, we don't
     have to do anything special.  This is the case on Vista and later. */
  wow64_eval_expected_main_stack (allocbase, stackbase);
  if (&wow64 >= (PULONG) allocbase && &wow64 < (PULONG) stackbase)
    return false;

  /* Check if the parent is a native 64 bit process.  Unfortunately there's
     no simpler way to retrieve the parent process in NT, as far as I know.
     Hints welcome. */
  ret = NtQueryInformationProcess (NtCurrentProcess (),
				   ProcessBasicInformation,
				   &pbi, sizeof pbi, NULL);
  if (NT_SUCCESS (ret)
      && (parent = OpenProcess (PROCESS_QUERY_INFORMATION,
				FALSE,
				(DWORD) pbi.InheritedFromUniqueProcessId)))
    {
      NtQueryInformationProcess (parent, ProcessWow64Information,
				 &wow64, sizeof wow64, NULL);
      CloseHandle (parent);
    }
  return !wow64;
}

PVOID
wow64_revert_to_original_stack (PVOID &allocationbase)
{
  /* Test if the original stack exists and has been set up as usual.  Even
     though the stack of the WOW64 process is at an unusual address, it appears
     that the "normal" stack has been created as usual.  It's partially in use
     by the 32->64 bit transition layer of the OS to help along the WOW64
     process, but it's otherwise mostly unused. */
  MEMORY_BASIC_INFORMATION mbi;
  PVOID stackbase;

  wow64_eval_expected_main_stack (allocationbase, stackbase);

  /* The stack is allocated in a single call, so the entire stack has the
     same AllocationBase.  At the start we expect a reserved region big
     enough still to host as the main stack. The OS apparently reserves
     always at least 256K for the main thread stack.  We err on the side
     of caution so we test here for a reserved region of at least 256K.
     That should be enough (knock on wood). */
  VirtualQuery (allocationbase, &mbi, sizeof mbi);
  if (mbi.State != MEM_RESERVE || mbi.RegionSize < 256 * 1024)
    return NULL;

  /* Next we expect a guard page.  We fetch the size of the guard area to
     see how big it is.  Apparently the guard area on 64 bit systems spans
     2 pages, only for the main thread for some reason.  We better keep it
     that way. */
  PVOID addr = PTR_ADD (mbi.BaseAddress, mbi.RegionSize);
  VirtualQuery (addr, &mbi, sizeof mbi);
  if (mbi.AllocationBase != allocationbase
      || mbi.State != MEM_COMMIT
      || !(mbi.Protect & PAGE_GUARD))
    return NULL;
  PVOID guardaddr = mbi.BaseAddress;
  SIZE_T guardsize = mbi.RegionSize;

  /* Next we expect a committed R/W region, the in-use area of that stack.
     This is just a sanity check. */
  addr = PTR_ADD (mbi.BaseAddress, mbi.RegionSize);
  VirtualQuery (addr, &mbi, sizeof mbi);
  if (mbi.AllocationBase != allocationbase
      || PTR_ADD (mbi.BaseAddress, mbi.RegionSize) != stackbase
      || mbi.State != MEM_COMMIT
      || mbi.Protect != PAGE_READWRITE)
    return NULL;

  /* The original stack is used by the OS.  Leave enough space for the OS
     to be happy (another 64K) and constitute a second stack within the so
     far reserved stack area. */
  PVOID newbase = PTR_ADD (guardaddr, -wincap.allocation_granularity ());
  PVOID newtop = PTR_ADD (newbase, -wincap.allocation_granularity ());
  guardaddr = PTR_ADD (newtop, -guardsize);
  if (!VirtualAlloc (newtop, wincap.allocation_granularity (),
		     MEM_COMMIT, PAGE_READWRITE))
    return NULL;
  if (!VirtualAlloc (guardaddr, guardsize, MEM_COMMIT,
		     PAGE_READWRITE | PAGE_GUARD))
    return NULL;

  /* We're going to reuse the original stack.  Yay, no more respawn!
     Set the StackBase and StackLimit values in the TEB, set _main_tls
     accordingly, and return the new, 16 byte aligned address for the
     stack pointer.  The second half of the stack move is done by the
     caller _dll_crt0. */
  _tlsbase = (char *) newbase;
  _tlstop = (char *) newtop;
  _main_tls = &_my_tls;
  return PTR_ADD (_tlsbase, -16);
}

/* Respawn WOW64 process. This is only called if we can't reuse the original
   stack.  See comment in wow64_revert_to_original_stack for details.  See
   _dll_crt0 for the call of this function.

   Historical note:

   Originally we just always respawned, right from dll_entry.  This stopped
   working with Cygwin 1.7.10 on Windows 2003 R2 64.  Starting with Cygwin
   1.7.10 we don't link against advapi32.dll anymore.  However, any process
   linked against advapi32, directly or indirectly, failed to respawn when
   trying respawning during DLL_PROCESS_ATTACH initialization.  In that
   case CreateProcessW returns with ERROR_ACCESS_DENIED for some reason. */
void
wow64_respawn_process ()
{
  WCHAR path[PATH_MAX];
  PROCESS_INFORMATION pi;
  STARTUPINFOW si;
  DWORD ret = 0;

  GetModuleFileNameW (NULL, path, PATH_MAX);
  GetStartupInfoW (&si);
  if (!CreateProcessW (path, GetCommandLineW (), NULL, NULL, TRUE,
		       CREATE_DEFAULT_ERROR_MODE
		       | GetPriorityClass (GetCurrentProcess ()),
		       NULL, NULL, &si, &pi))
    api_fatal ("Failed to create process <%W> <%W>, %E",
	       path, GetCommandLineW ());
  CloseHandle (pi.hThread);
  if (WaitForSingleObject (pi.hProcess, INFINITE) == WAIT_FAILED)
    api_fatal ("Waiting for process %u failed, %E", pi.dwProcessId);
  GetExitCodeProcess (pi.hProcess, &ret);
  CloseHandle (pi.hProcess);
  TerminateProcess (GetCurrentProcess (), ret);
  ExitProcess (ret);
}

#endif /* !__x86_64__ */
