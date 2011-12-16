/* wow64.cc

   Copyright 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include "ntdll.h"

#define PTR_ADD(p,o)	((PVOID)((PBYTE)(p)+(o)))

bool NO_COPY wow64_has_64bit_parent = false;

bool
wow64_test_for_64bit_parent ()
{
  /* On Windows XP 64 and 2003 64 there's a problem with processes running
     under WOW64.  The first process started from a 64 bit process has an
     unusual stack address for the main thread.  That is, an address which
     is in the usual space occupied by the process image, but below the auto
     load address of DLLs.  If this process forks, the child has its stack
     in the usual memory slot again, thus we have to "alloc_stack_hard_way".
     However, this fails in almost all cases because the stack slot of the
     parent process is taken by something else in the child process.

     If we encounter this situation, check if we really have been started
     from a 64 bit process here.  If so, we note this fact in
     wow64_has_64bit_parent so we can workaround the stack problem in
     _dll_crt0.  See there for how we go along. */
  NTSTATUS ret;
  PROCESS_BASIC_INFORMATION pbi;
  HANDLE parent;

  ULONG wow64 = TRUE;   /* Opt on the safe side. */

  /* First check if the stack is where it belongs.  If so, we don't have to
     do anything special.  This is the case on Vista and later. */
  if (&wow64 < (PULONG) 0x400000)
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
                                pbi.InheritedFromUniqueProcessId)))
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
     process, but it's otherwise mostly unused.
     The original stack is expected to be located at 0x30000, up to 0x230000.
     The assumption here is that the default main thread stack size is 2 Megs,
     but we expect lower stacksizes up to 1 Megs.  What we do here is to start
     about in the middle, but below the 1 Megs stack size.  The stack is
     allocated in a single call, so the entire stack has the same
     AllocationBase. */
  MEMORY_BASIC_INFORMATION mbi;
  PVOID addr = (PVOID) 0x100000;

  /* First fetch the AllocationBase. */
  VirtualQuery (addr, &mbi, sizeof mbi);
  allocationbase = mbi.AllocationBase;
  /* At the start we expect a reserved region big enough still to host as
     the main stack. 512K should be ok (knock on wood). */
  VirtualQuery (allocationbase, &mbi, sizeof mbi);
  if (mbi.State != MEM_RESERVE || mbi.RegionSize < 512 * 1024)
    return NULL;

  addr = PTR_ADD (mbi.BaseAddress, mbi.RegionSize);
  /* Next we expect a guard page. */
  VirtualQuery (addr, &mbi, sizeof mbi);
  if (mbi.AllocationBase != allocationbase
      || mbi.State != MEM_COMMIT
      || !(mbi.Protect & PAGE_GUARD))
    return NULL;

  PVOID guardaddr = mbi.BaseAddress;
  SIZE_T guardsize = mbi.RegionSize;
  addr = PTR_ADD (mbi.BaseAddress, mbi.RegionSize);
  /* Next we expect a committed R/W region, the in-use area of that stack. */
  VirtualQuery (addr, &mbi, sizeof mbi);
  if (mbi.AllocationBase != allocationbase
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
     accordingly, and return the new address for the stack pointer.
     The second half of the stack move is done by the caller _dll_crt0. */
  _tlsbase = (char *) newbase;
  _tlstop = (char *) newtop;
  _main_tls = &_my_tls;
  return PTR_ADD (_main_tls, -4);
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
    api_fatal ("Waiting for process %d failed, %E", pi.dwProcessId);
  GetExitCodeProcess (pi.hProcess, &ret);
  CloseHandle (pi.hProcess);
  TerminateProcess (GetCurrentProcess (), ret);
  ExitProcess (ret);
}
