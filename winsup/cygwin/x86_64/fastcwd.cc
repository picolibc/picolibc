/* x86_64/fastcwd.cc: find fast cwd pointer on x86_64 hosts.

  This file is part of Cygwin.

  This software is a copyrighted work licensed under the terms of the
  Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
  details. */

#include "winsup.h"

class fcwd_access_t;

#define peek32(x)	(*(int32_t *)(x))

/* This function scans the code in ntdll.dll to find the address of the
   global variable used to access the CWD.  While the pointer is global,
   it's not exported from the DLL, unfortunately.  Therefore we have to
   use some knowledge to figure out the address. */

fcwd_access_t **
find_fast_cwd_pointer_x86_64 ()
{
  /* Fetch entry points of relevant functions in ntdll.dll. */
  HMODULE ntdll = GetModuleHandle ("ntdll.dll");
  if (!ntdll)
    return NULL;
  const uint8_t *get_dir = (const uint8_t *)
			   GetProcAddress (ntdll, "RtlGetCurrentDirectory_U");
  const uint8_t *ent_crit = (const uint8_t *)
			    GetProcAddress (ntdll, "RtlEnterCriticalSection");
  if (!get_dir || !ent_crit)
    return NULL;
  /* Search first relative call instruction in RtlGetCurrentDirectory_U. */
  const uint8_t *rcall = (const uint8_t *) memchr (get_dir, 0xe8, 80);
  if (!rcall)
    return NULL;
  /* Fetch offset from instruction and compute address of called function.
     This function actually fetches the current FAST_CWD instance and
     performs some other actions, not important to us. */
  const uint8_t *use_cwd = rcall + 5 + peek32 (rcall + 1);
  /* Next we search for the locking mechanism and perform a sanity check.
     On Pre-Windows 8 we basically look for the RtlEnterCriticalSection call.
     Windows 8 does not call RtlEnterCriticalSection.  The code manipulates
     the FastPebLock manually, probably because RtlEnterCriticalSection has
     been converted to an inline function.  Either way, we test if the code
     uses the FastPebLock. */
  const uint8_t *movrbx;
  const uint8_t *lock = (const uint8_t *)
                        memmem ((const char *) use_cwd, 80,
                                "\xf0\x0f\xba\x35", 4);
  if (lock)
    {
      /* The lock instruction tweaks the LockCount member, which is not at
	 the start of the PRTL_CRITICAL_SECTION structure.  So we have to
	 subtract the offset of LockCount to get the real address. */
      PRTL_CRITICAL_SECTION lockaddr =
        (PRTL_CRITICAL_SECTION) (lock + 9 + peek32 (lock + 4)
                                 - offsetof (RTL_CRITICAL_SECTION, LockCount));
      /* Test if lock address is FastPebLock. */
      if (lockaddr != NtCurrentTeb ()->Peb->FastPebLock)
        return NULL;
      /* Search `mov rel(%rip),%rbx'.  This is the instruction fetching the
         address of the current fcwd_access_t pointer, and it should be pretty
	 near to the locking stuff. */
      movrbx = (const uint8_t *) memmem ((const char *) lock, 40,
                                         "\x48\x8b\x1d", 3);
    }
  else
    {
      /* Usually the callq RtlEnterCriticalSection follows right after
	 fetching the lock address. */
      int call_rtl_offset = 7;
      /* Search `lea rel(%rip),%rcx'.  This loads the address of the lock into
         %rcx for the subsequent RtlEnterCriticalSection call. */
      lock = (const uint8_t *) memmem ((const char *) use_cwd, 80,
                                       "\x48\x8d\x0d", 3);
      if (!lock)
	{
	  /* Windows 8.1 Preview calls `lea rel(rip),%r12' then some unrelated
	     ops, then `mov %r12,%rcx', then `callq RtlEnterCriticalSection'. */
	  lock = (const uint8_t *) memmem ((const char *) use_cwd, 80,
					   "\x4c\x8d\x25", 3);
	  call_rtl_offset = 14;
	}

      if (!lock)
	{
	  /* A recent Windows 11 Preview calls `lea rel(rip),%r13' then
	     some unrelated instructions, then `callq RtlEnterCriticalSection'.
	     */
	  lock = (const uint8_t *) memmem ((const char *) use_cwd, 80,
					   "\x4c\x8d\x2d", 3);
	  call_rtl_offset = 24;
	}

      if (!lock)
	{
	  return NULL;
	}

      PRTL_CRITICAL_SECTION lockaddr =
        (PRTL_CRITICAL_SECTION) (lock + 7 + peek32 (lock + 3));
      /* Test if lock address is FastPebLock. */
      if (lockaddr != NtCurrentTeb ()->Peb->FastPebLock)
        return NULL;
      /* Next is the `callq RtlEnterCriticalSection'. */
      lock += call_rtl_offset;
      if (lock[0] != 0xe8)
        return NULL;
      const uint8_t *call_addr = (const uint8_t *)
                                 (lock + 5 + peek32 (lock + 1));
      if (call_addr != ent_crit)
        return NULL;
      /* In contrast to the above Windows 8 code, we don't have to search
	 for the `mov rel(%rip),%rbx' instruction.  It follows right after
	 the call to RtlEnterCriticalSection. */
      movrbx = lock + 5;
    }
  if (!movrbx)
    return NULL;
  /* Check that the next instruction tests if the fetched value is NULL. */
  const uint8_t *testrbx = (const uint8_t *)
			   memmem (movrbx + 7, 3, "\x48\x85\xdb", 3);
  if (!testrbx)
    return NULL;
  /* Compute address of the fcwd_access_t ** pointer. */
  return (fcwd_access_t **) (testrbx + peek32 (movrbx + 3));
}
