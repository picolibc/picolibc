/* shared.cc: shared data area support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "heap.h"
#include "shared_info_magic.h"
#include "registry.h"
#include "cygwin_version.h"
#include "child_info.h"
#include "mtinfo.h"

shared_info NO_COPY *cygwin_shared;
user_info NO_COPY *user_shared;
HANDLE NO_COPY cygwin_user_h;

char * __stdcall
shared_name (char *ret_buf, const char *str, int num)
{
  extern bool _cygwin_testing;

  __small_sprintf (ret_buf, "%s%s.%s.%d", cygheap->shared_prefix,
		   cygwin_version.shared_id, str, num);
  if (_cygwin_testing)
    strcat (ret_buf, cygwin_version.dll_build_date);
  return ret_buf;
}

#define page_const (65535)
#define pround(n) (((size_t) (n) + page_const) & ~page_const)

static char *offsets[] =
{
  (char *) cygwin_shared_address,
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (user_info)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (user_info))
    + pround (sizeof (console_state)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (user_info))
    + pround (sizeof (console_state))
    + pround (sizeof (_pinfo)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (user_info))
    + pround (sizeof (console_state))
    + pround (sizeof (_pinfo))
    + pround (sizeof (mtinfo))
};

void * __stdcall
open_shared (const char *name, int n, HANDLE& shared_h, DWORD size,
	     shared_locations& m, PSECURITY_ATTRIBUTES psa, DWORD access)
{
  void *shared;

  void *addr;
  if ((m == SH_JUSTCREATE || m == SH_JUSTOPEN)
      || !wincap.needs_memory_protection () && offsets[0])
    addr = NULL;
  else
    {
      addr = offsets[m];
      VirtualFree (addr, 0, MEM_RELEASE);
    }

  char map_buf[CYG_MAX_PATH];
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
	  shared_h = CreateFileMapping (INVALID_HANDLE_VALUE, psa, PAGE_READWRITE,
					0, size, mapname);
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
      /* Probably win95, so try without specifying the address.  */
      shared = (shared_info *) MapViewOfFileEx (shared_h,
				       FILE_MAP_READ|FILE_MAP_WRITE,
				       0, 0, 0, NULL);
#ifdef DEBUGGING
      if (wincap.is_winnt ())
	system_printf ("relocating shared object %s(%d) from %p to %p on Windows NT", name, n, addr, shared);
#endif
      offsets[0] = NULL;
    }

  if (!shared)
    api_fatal ("MapViewOfFileEx '%s'(%p), %E.  Terminating.", mapname, shared_h);

  if (m == SH_CYGWIN_SHARED && offsets[0] && wincap.needs_memory_protection ())
    {
      unsigned delta = (char *) shared - offsets[0];
      offsets[0] = (char *) shared;
      for (int i = SH_CYGWIN_SHARED + 1; i < SH_TOTAL_SIZE; i++)
	{
	  unsigned size = offsets[i + 1] - offsets[i];
	  offsets[i] += delta;
	  if (!VirtualAlloc (offsets[i], size, MEM_RESERVE, PAGE_NOACCESS))
	    continue;  /* oh well */
	}
      offsets[SH_TOTAL_SIZE] += delta;

#if 0
      if (!child_proc_info && wincap.needs_memory_protection ())
	for (DWORD s = 0x950000; s <= 0xa40000; s += 0x1000)
	  VirtualAlloc ((void *) s, 4, MEM_RESERVE, PAGE_NOACCESS);
#endif
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
      user_shared->delqueue.init (); /* Initialize the queue of deleted files.  */
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

  if (!sversion)
    {
      tty.init ();		/* Initialize tty table.  */
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
					       cygheap->shared_h,
					       sizeof (*cygwin_shared),
					       sh_cygwin_shared);

  cygwin_shared->initialize ();
  ProtectHandleINH (cygheap->shared_h);

  user_shared_initialize (false);
  mtinfo_init ();
}

unsigned
shared_info::heap_slop_size ()
{
  if (!heap_slop)
    {
      /* Fetch from registry, first user then local machine.  */
      for (int i = 0; i < 2; i++)
	{
	  reg_key reg (i, KEY_READ, NULL);

	  if ((heap_slop = reg.get_int ("heap_slop_in_mb", 0)))
	    break;
	  heap_slop = wincap.heapslop ();
	}

      if (heap_slop < 0)
	heap_slop = 0;
      else
	heap_slop <<= 20;
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
