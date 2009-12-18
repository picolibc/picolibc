/* shared.cc: shared data area support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009 Red Hat, Inc.

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
#include "pwdgrp.h"
#include "ntdll.h"
#include <alloca.h>
#include <wchar.h>
#include <wingdi.h>
#include <winuser.h>

shared_info NO_COPY *cygwin_shared;
user_info NO_COPY *user_shared;
HANDLE NO_COPY cygwin_shared_h;
HANDLE NO_COPY cygwin_user_h;

WCHAR installation_root[PATH_MAX] __attribute__((section (".cygwin_dll_common"), shared));
UNICODE_STRING installation_key __attribute__((section (".cygwin_dll_common"), shared));
WCHAR installation_key_buf[18] __attribute__((section (".cygwin_dll_common"), shared));
static bool inst_root_inited;

/* Use absolute path of cygwin1.dll to derive the Win32 dir which
   is our installation_root.  Note that we can't handle Cygwin installation
   root dirs of more than 4K path length.  I assume that's ok...

   This function also generates the installation_key value.  It's a 64 bit
   hash value based on the path of the Cygwin DLL itself.  It's subsequently
   used when generating shared object names.  Thus, different Cygwin
   installations generate different object names and so are isolated from
   each other.
   
   Having this information, the installation key together with the
   installation root path is written to the registry.  The idea is that
   cygcheck can print the paths into which the Cygwin DLL has been
   installed for debugging purposes.
   
   Last but not least, the new cygwin properties datastrcuture is checked
   for the "disabled_key" value, which is used to determine whether the
   installation key is actually added to all object names or not.  This is
   used as a last resort for debugging purposes, usually.  However, there
   could be another good reason to re-enable object name collisions between
   multiple Cygwin DLLs, which we're just not aware of right now.  Cygcheck
   can be used to change the value in an existing Cygwin DLL binary. */

void
init_installation_root ()
{
  if (!GetModuleFileNameW (cygwin_hmodule, installation_root, PATH_MAX))
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "GetModuleFileNameW(%p, %p, %u), %E",
	       cygwin_hmodule, installation_root, PATH_MAX);
  PWCHAR p = installation_root;
  if (wcsncmp (p, L"\\\\?\\", 4))	/* No long path prefix. */
    {
      if (!wcsncasecmp (p, L"\\\\", 2))	/* UNC */
	{
	  p = wcpcpy (p, L"\\??\\UN");
	  GetModuleFileNameW (cygwin_hmodule, p, PATH_MAX - 6);
	  *p = L'C';
	}
      else
	{
	  p = wcpcpy (p, L"\\??\\");
	  GetModuleFileNameW (cygwin_hmodule, p, PATH_MAX - 4);
	}
    }
  installation_root[1] = L'?';

  RtlInitEmptyUnicodeString (&installation_key, installation_key_buf,
			     sizeof installation_key_buf);
  RtlInt64ToHexUnicodeString (hash_path_name (0, installation_root),
			      &installation_key, FALSE);

  PWCHAR w = wcsrchr (installation_root, L'\\');
  if (w)
    {
      *w = L'\0';
      w = wcsrchr (installation_root, L'\\');
    }
  if (!w)
    api_fatal ("Can't initialize Cygwin installation root dir.\n"
	       "Invalid DLL path");
  *w = L'\0';

  for (int i = 1; i >= 0; --i)
    {
      reg_key r (i, KEY_WRITE, CYGWIN_INFO_INSTALLATIONS_NAME, NULL);
      if (r.set_string (installation_key_buf, installation_root)
	  == ERROR_SUCCESS)
      	break;
    }

  if (cygwin_props.disable_key)
    {
      installation_key.Length = 0;
      installation_key.Buffer[0] = L'\0';
    }

  inst_root_inited = true;
}

/* This function returns a handle to the top-level directory in the global
   NT namespace used to implement global objects including shared memory. */

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
      __small_swprintf (bnoname, L"\\BaseNamedObjects\\%s%s-%S",
			cygwin_version.shared_id,
			_cygwin_testing ? cygwin_version.dll_build_date : "",
			&installation_key);
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
      status = NtQueryInformationProcess (NtCurrentProcess (),
					  ProcessSessionInformation,
					  &psi, sizeof psi, NULL);
      if (!NT_SUCCESS (status) || psi.SessionId == 0)
	dir = get_shared_parent_dir ();
      else
	{
	  WCHAR bnoname[MAX_PATH];
	  __small_swprintf (bnoname,
			    L"\\Sessions\\BNOLINKS\\%d\\%s%s-%S",
			    psi.SessionId, cygwin_version.shared_id,
			    _cygwin_testing ? cygwin_version.dll_build_date : "",
			    &installation_key);
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

WCHAR * __stdcall
shared_name (WCHAR *ret_buf, const WCHAR *str, int num)
{
  __small_swprintf (ret_buf, L"%W.%d", str, num);
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
open_shared (const WCHAR *name, int n, HANDLE& shared_h, DWORD size,
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

  WCHAR map_buf[MAX_PATH];
  WCHAR *mapname = NULL;

  if (shared_h)
    m = SH_JUSTOPEN;
  else
    {
      if (name)
	mapname = shared_name (map_buf, name, n);
      if (m == SH_JUSTOPEN)
	shared_h = OpenFileMappingW (access, FALSE, mapname);
      else
	{
	  shared_h = CreateFileMappingW (INVALID_HANDLE_VALUE, psa,
					PAGE_READWRITE, 0, size, mapname);
	  if (GetLastError () == ERROR_ALREADY_EXISTS)
	    m = SH_JUSTOPEN;
	}
      if (shared_h)
	/* ok! */;
      else if (m != SH_JUSTOPEN)
	api_fatal ("CreateFileMapping %W, %E.  Terminating.", mapname);
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
      system_printf ("relocating shared object %W(%d) from %p to %p", name, n, addr, shared);
#endif
      offsets[0] = 0;
    }

  if (!shared)
    api_fatal ("MapViewOfFileEx '%W'(%p), %E.  Terminating.", mapname, shared_h);

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

  debug_printf ("name %W, n %d, shared %p (wanted %p), h %p", mapname, n, shared, addr, shared_h);

  return shared;
}

/* Second half of user shared initialization: Initialize content. */
void
user_shared_initialize ()
{
  DWORD sversion = (DWORD) InterlockedExchange ((LONG *) &user_shared->version, USER_VERSION_MAGIC);
  /* Wait for initialization of the Cygwin per-user shared, if necessary */
  if (!sversion)
    {
      cygpsid sid (cygheap->user.sid ());
      struct passwd *pw = internal_getpwsid (sid);
      /* Correct the user name with what's defined in /etc/passwd before
	 loading the user fstab file. */
      if (pw)
	cygheap->user.set_name (pw->pw_name);
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

/* First half of user shared initialization: Create shared mem region. */
void
user_shared_create (bool reinit)
{
  WCHAR name[UNLEN + 1] = L""; /* Large enough for SID */

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
  debug_printf ("opening user shared for '%W' at %p", name, user_shared);
  ProtectHandleINH (cygwin_user_h);
  debug_printf ("user shared version %x", user_shared->version);
  if (reinit)
    user_shared_initialize ();
}

void __stdcall
shared_destroy ()
{
  ForceCloseHandle (cygwin_shared_h);
  UnmapViewOfFile (cygwin_shared);
  ForceCloseHandle (cygwin_user_h);
  UnmapViewOfFile (user_shared);
}

/* Initialize obcaseinsensitive.  Default to case insensitive on pre-XP. */
void
shared_info::init_obcaseinsensitive ()
{
  HKEY key;
  DWORD size = sizeof (DWORD);

  obcaseinsensitive = 1;
  if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
		  "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\kernel",
		  0, KEY_READ, &key) == ERROR_SUCCESS)
    {
      RegQueryValueEx (key, "obcaseinsensitive", NULL, NULL,
		       (LPBYTE) &obcaseinsensitive, &size);
      RegCloseKey (key);
    }
  debug_printf ("obcaseinsensitive set to %d", obcaseinsensitive);
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
      init_obcaseinsensitive ();/* Initialize obcaseinsensitive. */
      tty.init ();		/* Initialize tty table.  */
      mt.initialize ();		/* Initialize shared tape information. */
      cb = sizeof (*this);	/* Do last, after all shared memory initialization */
    }

  if (cb != SHARED_INFO_CB)
    system_printf ("size of shared memory region changed from %u to %u",
		   SHARED_INFO_CB, cb);
}

void
memory_init (bool init_cygheap)
{
  getpagesize ();

  /* Initialize the Cygwin heap, if necessary */
  if (init_cygheap)
    {
      cygheap_init ();
      cygheap->user.init ();
    }

  /* Initialize installation root dir. */
  if (!installation_root[0])
    init_installation_root ();

  /* Initialize general shared memory */
  shared_locations sh_cygwin_shared;
  cygwin_shared = (shared_info *) open_shared (L"shared",
					       CYGWIN_VERSION_SHARED_DATA,
					       cygwin_shared_h,
					       sizeof (*cygwin_shared),
					       sh_cygwin_shared = SH_CYGWIN_SHARED);
  /* Defer debug output printing the installation root and installation key
     up to this point.  Debug output except for system_printf requires
     the global shared memory to exist. */
  if (inst_root_inited)
    debug_printf ("Installation root: <%W> key: <%S>",
		  installation_root, &installation_key);
  cygwin_shared->initialize ();
  user_shared_create (false);
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
