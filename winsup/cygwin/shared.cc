/* shared.cc: shared data area support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

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
#include "pinfo.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygerrno.h"
#include "cygheap.h"
#include "heap.h"
#include "shared_info_magic.h"
#include "registry.h"
#include "cygwin_version.h"
#include "child_info.h"

shared_info NO_COPY *cygwin_shared;
mount_info NO_COPY *mount_table;
HANDLE NO_COPY cygwin_mount_h;

char * __stdcall
shared_name (char *ret_buf, const char *str, int num)
{
  extern bool _cygwin_testing;

  __small_sprintf (ret_buf, "%s%s.%s.%d",
  		   wincap.has_terminal_services () ?  "Global\\" : "",
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
    + pround (sizeof (mount_info)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (mount_info))
    + pround (sizeof (console_state)),
  (char *) cygwin_shared_address
    + pround (sizeof (shared_info))
    + pround (sizeof (mount_info))
    + pround (sizeof (console_state))
    + pround (sizeof (_pinfo))
};

void * __stdcall
open_shared (const char *name, int n, HANDLE &shared_h, DWORD size,
	     shared_locations m, PSECURITY_ATTRIBUTES psa)
{
  void *shared;

  void *addr;
  if (!wincap.needs_memory_protection ())
    addr = NULL;
  else
    {
      addr = offsets[m];
      (void) VirtualFree (addr, 0, MEM_RELEASE);
    }

  if (!size)
    return addr;

  if (!shared_h)
    {
      char *mapname;
      char map_buf[MAX_PATH];
      if (!name)
	mapname = NULL;
      else
	{
	  mapname = shared_name (map_buf, name, n);
	  shared_h = OpenFileMappingA (FILE_MAP_READ | FILE_MAP_WRITE,
				       TRUE, mapname);
	}
      if (!shared_h &&
	  !(shared_h = CreateFileMapping (INVALID_HANDLE_VALUE, psa,
					  PAGE_READWRITE, 0, size, mapname)))
	api_fatal ("CreateFileMapping, %E.  Terminating.");
    }

  shared = (shared_info *)
    MapViewOfFileEx (shared_h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, addr);

  if (!shared)
    {
      /* Probably win95, so try without specifying the address.  */
      shared = (shared_info *) MapViewOfFileEx (shared_h,
				       FILE_MAP_READ|FILE_MAP_WRITE,
				       0, 0, 0, 0);
#ifdef DEBUGGING
      if (wincap.is_winnt ())
	system_printf ("relocating shared object %s(%d) from %p to %p on Windows NT", name, n, addr, shared);
#endif
    }

  if (!shared)
    api_fatal ("MapViewOfFileEx '%s'(%p), %E.  Terminating.", name, shared_h);

  if (m == SH_CYGWIN_SHARED && wincap.needs_memory_protection ())
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

  debug_printf ("name %s, shared %p (wanted %p), h %p", name, shared, addr, shared_h);

  return shared;
}

void
user_shared_initialize ()
{
  char name[UNLEN > 127 ? UNLEN + 1 : 128] = "";

  if (wincap.has_security ())
    {
      cygsid tu (cygheap->user.sid ());
      tu.string (name);
    }
  else
    strcpy (name, cygheap->user.name ());

  if (cygwin_mount_h) /* Reinit */
    {
      if (!UnmapViewOfFile (mount_table))
	debug_printf("UnmapViewOfFile %E");
      if (!ForceCloseHandle (cygwin_mount_h))
	debug_printf("CloseHandle %E");
      cygwin_mount_h = NULL;
    }

  mount_table = (mount_info *) open_shared (name, MOUNT_VERSION,
					    cygwin_mount_h, sizeof (mount_info),
					    SH_MOUNT_TABLE, &sec_none);
  debug_printf ("opening mount table for '%s' at %p", name,
		mount_table);
  ProtectHandleINH (cygwin_mount_h);
  debug_printf ("mount table version %x at %p", mount_table->version, mount_table);

  /* Initialize the Cygwin per-user mount table, if necessary */
  if (!mount_table->version)
    {
      mount_table->version = MOUNT_VERSION_MAGIC;
      debug_printf ("initializing mount table");
      mount_table->cb = sizeof (*mount_table);
      if (mount_table->cb != MOUNT_INFO_CB)
	system_printf ("size of mount table region changed from %u to %u",
		       MOUNT_INFO_CB, mount_table->cb);
      mount_table->init ();	/* Initialize the mount table.  */
    }
  else if (mount_table->version != MOUNT_VERSION_MAGIC)
    multiple_cygwin_problem ("mount", mount_table->version, MOUNT_VERSION);
  else if (mount_table->cb !=  MOUNT_INFO_CB)
    multiple_cygwin_problem ("mount table size", mount_table->cb, MOUNT_INFO_CB);
}

void
shared_info::initialize ()
{
  DWORD sversion = (DWORD) InterlockedExchange ((LONG *) &version, SHARED_VERSION_MAGIC);
  if (!sversion)
    {
      /* Initialize the queue of deleted files.  */
      delqueue.init ();

      /* Initialize tty table.  */
      tty.init ();
    }
  else
    {
      if (version != SHARED_VERSION_MAGIC)
	{
	  multiple_cygwin_problem ("shared", version, SHARED_VERSION_MAGIC);
	  InterlockedExchange ((LONG *) &version, sversion);
	}
      while (!cb)
	low_priority_sleep (0);	// Should be hit only very very rarely
    }

  /* Initialize the Cygwin heap, if necessary */
  if (!cygheap)
    {
      cygheap_init ();
      cygheap->user.init ();
    }

  heap_init ();

  if (!sversion)
    cb = sizeof (*this);	// Do last, after all shared memory initializion

  if (cb != SHARED_INFO_CB)
    system_printf ("size of shared memory region changed from %u to %u",
		   SHARED_INFO_CB, cb);
}

void __stdcall
memory_init ()
{
  getpagesize ();

  /* Initialize general shared memory */
  HANDLE shared_h = cygheap ? cygheap->shared_h : NULL;
  cygwin_shared = (shared_info *) open_shared ("shared",
					       CYGWIN_VERSION_SHARED_DATA,
					       shared_h,
					       sizeof (*cygwin_shared),
					       SH_CYGWIN_SHARED);

  cygwin_shared->initialize ();
  cygheap->shared_h = shared_h;
  ProtectHandleINH (cygheap->shared_h);

  user_shared_initialize ();
}

unsigned
shared_info::heap_chunk_size ()
{
  if (!heap_chunk)
    {
      /* Fetch misc. registry entries.  */

      reg_key reg (KEY_READ, NULL);

      /* Note that reserving a huge amount of heap space does not result in
      the use of swap since we are not committing it. */
      /* FIXME: We should not be restricted to a fixed size heap no matter
      what the fixed size is. */

      heap_chunk = reg.get_int ("heap_chunk_in_mb", 0);
      if (!heap_chunk) {
	reg_key r1 (HKEY_LOCAL_MACHINE, KEY_READ, "SOFTWARE",
		    CYGWIN_INFO_CYGNUS_REGISTRY_NAME,
		    CYGWIN_INFO_CYGWIN_REGISTRY_NAME, NULL);
	heap_chunk = r1.get_int ("heap_chunk_in_mb", 384);
      }

      if (heap_chunk < 4)
	heap_chunk = 4 * 1024 * 1024;
      else
	heap_chunk <<= 20;
      if (!heap_chunk)
	heap_chunk = 384 * 1024 * 1024;
      debug_printf ("fixed heap size is %u", heap_chunk);
    }

  return heap_chunk;
}
