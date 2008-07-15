/* shared.cc: shared data area support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "heap.h"
#include "shared_info_magic.h"
#include "registry.h"
#include "cygwin_version.h"
#include "ntdll.h"
#include <alloca.h>

shared_info NO_COPY *cygwin_shared;
user_info NO_COPY *user_shared;
HANDLE NO_COPY cygwin_shared_h;
HANDLE NO_COPY cygwin_user_h;

/* This function returns a handle to the top-level directory in the global
   NT namespace used to implement global objects including shared memory. */
extern bool _cygwin_testing;

HANDLE
get_shared_parent_dir ()
{
  static HANDLE dir;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!dir)
    {
      WCHAR bnoname[MAX_PATH];
      __small_swprintf (bnoname, L"\\BaseNamedObjects\\%s%s",
			cygwin_version.shared_id,
			_cygwin_testing ? cygwin_version.dll_build_date : "");
      RtlInitUnicodeString (&uname, bnoname);
      InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
                                  NULL, everyone_sd (CYG_SHARED_DIR_ACCESS));
      status = NtCreateDirectoryObject (&dir, CYG_SHARED_DIR_ACCESS, &attr);
      if (!NT_SUCCESS (status))
        api_fatal ("NtCreateDirectoryObject(%S): %p", &uname, status);
    }
  return dir;
} 

HANDLE
get_session_parent_dir ()
{
  static HANDLE dir;
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!dir)
    {
      PROCESS_SESSION_INFORMATION psi;
      status = NtQueryInformationProcess (GetCurrentProcess (),
					  ProcessSessionInformation,
					  &psi, sizeof psi, NULL);
      if (!NT_SUCCESS (status) || psi.SessionId == 0)
	dir = get_shared_parent_dir ();
      else
        {
	  WCHAR bnoname[MAX_PATH];
	  __small_swprintf (bnoname,
			    L"\\Sessions\\BNOLINKS\\%d\\%s%s",
			    psi.SessionId, cygwin_version.shared_id,
			    _cygwin_testing ? cygwin_version.dll_build_date : "");
	  RtlInitUnicodeString (&uname, bnoname);
	  InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
				      NULL, everyone_sd(CYG_SHARED_DIR_ACCESS));
	  status = NtCreateDirectoryObject (&dir, CYG_SHARED_DIR_ACCESS, &attr);
	  if (!NT_SUCCESS (status))
	    api_fatal ("NtCreateDirectoryObject(%S): %p", &uname, status);
	}
    }
  return dir;
} 

char * __stdcall
shared_name (char *ret_buf, const char *str, int num)
{
  __small_sprintf (ret_buf, "%s.%d", str, num);
  return ret_buf;
}

#define page_const (65535)
#define pround(n) (((size_t) (n) + page_const) & ~page_const)

static ptrdiff_t offsets[] =
{
  - pround (sizeof (shared_info))
  - pround (sizeof (user_info))
  - pround (sizeof (console_state))
  - pround (sizeof (_pinfo)),
  - pround (sizeof (user_info))
  - pround (sizeof (console_state))
  - pround (sizeof (_pinfo)),
  - pround (sizeof (console_state))
  - pround (sizeof (_pinfo)),
  - pround (sizeof (_pinfo)),
  0
};

#define off_addr(x)	((void *)((caddr_t) cygwin_hmodule + offsets[x]))

void * __stdcall
open_shared (const char *name, int n, HANDLE& shared_h, DWORD size,
	     shared_locations& m, PSECURITY_ATTRIBUTES psa, DWORD access)
{
  void *shared;

  void *addr;
  if (m == SH_JUSTCREATE || m == SH_JUSTOPEN)
    addr = NULL;
  else
    {
      addr = off_addr (m);
      VirtualFree (addr, 0, MEM_RELEASE);
    }

  char map_buf[MAX_PATH];
  char *mapname = NULL;

  if (shared_h)
    m = SH_JUSTOPEN;
  else
    {
      if (name)
	mapname = shared_name (map_buf, name, n);
      if (m == SH_JUSTOPEN)
	shared_h = OpenFileMapping (access, FALSE, mapname);
      else
	{
	  shared_h = CreateFileMapping (INVALID_HANDLE_VALUE, psa,
	  				PAGE_READWRITE, 0, size, mapname);
	  if (GetLastError () == ERROR_ALREADY_EXISTS)
	    m = SH_JUSTOPEN;
	}
      if (shared_h)
	/* ok! */;
      else if (m != SH_JUSTOPEN)
	api_fatal ("CreateFileMapping %s, %E.  Terminating.", mapname);
      else
	return NULL;
    }

  shared = (shared_info *)
    MapViewOfFileEx (shared_h, access, 0, 0, 0, addr);

  if (!shared && addr)
    {
      shared = (shared_info *) MapViewOfFileEx (shared_h,
				       FILE_MAP_READ|FILE_MAP_WRITE,
				       0, 0, 0, NULL);
#ifdef DEBUGGING
      system_printf ("relocating shared object %s(%d) from %p to %p", name, n, addr, shared);
#endif
      offsets[0] = 0;
    }

  if (!shared)
    api_fatal ("MapViewOfFileEx '%s'(%p), %E.  Terminating.", mapname, shared_h);

  if (m == SH_CYGWIN_SHARED && offsets[0])
    {
      ptrdiff_t delta = (caddr_t) shared - (caddr_t) off_addr (0);
      offsets[0] = (caddr_t) shared - (caddr_t) cygwin_hmodule;
      for (int i = SH_USER_SHARED + 1; i < SH_TOTAL_SIZE; i++)
	{
	  unsigned size = offsets[i + 1] - offsets[i];
	  offsets[i] += delta;
	  if (!VirtualAlloc (off_addr (i), size, MEM_RESERVE, PAGE_NOACCESS))
	    continue;  /* oh well */
	}
      offsets[SH_TOTAL_SIZE] += delta;
    }

  debug_printf ("name %s, n %d, shared %p (wanted %p), h %p", mapname, n, shared, addr, shared_h);

  return shared;
}

void
user_shared_initialize (bool reinit)
{
  char name[UNLEN + 1] = ""; /* Large enough for SID */

  if (reinit)
    {
      if (!UnmapViewOfFile (user_shared))
	debug_printf("UnmapViewOfFile %E");
      if (!ForceCloseHandle (cygwin_user_h))
	debug_printf("CloseHandle %E");
      cygwin_user_h = NULL;
    }

  if (!cygwin_user_h)
    cygheap->user.get_windows_id (name);

  shared_locations sh_user_shared = SH_USER_SHARED;
  user_shared = (user_info *) open_shared (name, USER_VERSION,
					    cygwin_user_h, sizeof (user_info),
					    sh_user_shared, &sec_none);
  debug_printf ("opening user shared for '%s' at %p", name, user_shared);
  ProtectHandleINH (cygwin_user_h);
  debug_printf ("user shared version %x", user_shared->version);

  DWORD sversion = (DWORD) InterlockedExchange ((LONG *) &user_shared->version, USER_VERSION_MAGIC);
  /* Initialize the Cygwin per-user shared, if necessary */
  if (!sversion)
    {
      debug_printf ("initializing user shared");
      user_shared->mountinfo.init ();	/* Initialize the mount table.  */
      user_shared->cb =  sizeof (*user_shared);
    }
  else
    {
      while (!user_shared->cb)
	low_priority_sleep (0);	// Should be hit only very very rarely
      if (user_shared->version != sversion)
	multiple_cygwin_problem ("user shared memory version", user_shared->version, sversion);
      else if (user_shared->cb != sizeof (*user_shared))
	multiple_cygwin_problem ("user shared memory size", user_shared->cb, sizeof (*user_shared));
    }
}

void
shared_info::initialize ()
{
  DWORD sversion = (DWORD) InterlockedExchange ((LONG *) &version, SHARED_VERSION_MAGIC);
  if (sversion)
    {
      if (sversion != SHARED_VERSION_MAGIC)
	{
	  InterlockedExchange ((LONG *) &version, sversion);
	  multiple_cygwin_problem ("system shared memory version", sversion, SHARED_VERSION_MAGIC);
	}
      while (!cb)
	low_priority_sleep (0);	// Should be hit only very very rarely
    }

  heap_init ();
  get_session_parent_dir ();	/* Create session dir if first process. */

  if (!sversion)
    {
      tty.init ();		/* Initialize tty table.  */
      mt.initialize ();		/* Initialize shared tape information. */
      cb = sizeof (*this);	/* Do last, after all shared memory initialization */
    }

  if (cb != SHARED_INFO_CB)
    system_printf ("size of shared memory region changed from %u to %u",
		   SHARED_INFO_CB, cb);
}

void __stdcall
memory_init ()
{
  getpagesize ();

  /* Initialize the Cygwin heap, if necessary */
  if (!cygheap)
    {
      cygheap_init ();
      cygheap->user.init ();
    }

  /* Initialize general shared memory */
  shared_locations sh_cygwin_shared = SH_CYGWIN_SHARED;
  cygwin_shared = (shared_info *) open_shared ("shared",
					       CYGWIN_VERSION_SHARED_DATA,
					       cygwin_shared_h,
					       sizeof (*cygwin_shared),
					       sh_cygwin_shared);

  cygwin_shared->initialize ();

  user_shared_initialize (false);
}

unsigned
shared_info::heap_slop_size ()
{
  if (!heap_slop_inited)
    {
      /* Fetch from registry, first user then local machine.  */
      for (int i = 0; i < 2; i++)
	{
	  reg_key reg (i, KEY_READ, NULL);

	  if ((heap_slop = reg.get_int ("heap_slop_in_mb", 0)))
	    break;
	  heap_slop = wincap.heapslop ();
	}
      heap_slop <<= 20;
      heap_slop_inited = true;
    }

  return heap_slop;
}

unsigned
shared_info::heap_chunk_size ()
{
  if (!heap_chunk)
    {
      /* Fetch from registry, first user then local machine.  */
      for (int i = 0; i < 2; i++)
	{
	  reg_key reg (i, KEY_READ, NULL);

	  /* Note that reserving a huge amount of heap space does not result in
	     the use of swap since we are not committing it. */
	  /* FIXME: We should not be restricted to a fixed size heap no matter
	     what the fixed size is. */

	  if ((heap_chunk = reg.get_int ("heap_chunk_in_mb", 0)))
	    break;
	  heap_chunk = 384; /* Default */
	}

      if (heap_chunk < 4)
	heap_chunk = 4 * 1024 * 1024;
      else
	heap_chunk <<= 20;
      if (!heap_chunk)
	heap_chunk = 384 * 1024 * 1024;
    }

  return heap_chunk;
}
