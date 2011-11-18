/* shared.cc: shared data area support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

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
#include "spinlock.h"
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
static LONG installation_root_inited __attribute__((section (".cygwin_dll_common"), shared));

#define SPIN_WAIT 10000

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

void inline
init_installation_root ()
{
  spinlock iri (installation_root_inited);
  if (!iri)
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
	  reg_key r (i, KEY_WRITE, _WIDE (CYGWIN_INFO_INSTALLATIONS_NAME),
		     NULL);
	  if (NT_SUCCESS (r.set_string (installation_key_buf,
					installation_root)))
	    break;
	}

      if (cygwin_props.disable_key)
	{
	  installation_key.Length = 0;
	  installation_key.Buffer[0] = L'\0';
	}
    }
}

/* This function returns a handle to the top-level directory in the global
   NT namespace used to implement global objects including shared memory. */

static HANDLE NO_COPY shared_parent_dir;

HANDLE
get_shared_parent_dir ()
{
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!shared_parent_dir)
    {
      WCHAR bnoname[MAX_PATH];
      __small_swprintf (bnoname, L"\\BaseNamedObjects\\%s%s-%S",
			cygwin_version.shared_id,
			_cygwin_testing ? cygwin_version.dll_build_date : "",
			&installation_key);
      RtlInitUnicodeString (&uname, bnoname);
      InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF, NULL,
				  everyone_sd (CYG_SHARED_DIR_ACCESS));
      status = NtCreateDirectoryObject (&shared_parent_dir,
					CYG_SHARED_DIR_ACCESS, &attr);
      if (!NT_SUCCESS (status))
	api_fatal ("NtCreateDirectoryObject(%S): %p", &uname, status);
    }
  return shared_parent_dir;
}

static HANDLE NO_COPY session_parent_dir;

HANDLE
get_session_parent_dir ()
{
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!session_parent_dir)
    {
      PROCESS_SESSION_INFORMATION psi;
      status = NtQueryInformationProcess (NtCurrentProcess (),
					  ProcessSessionInformation,
					  &psi, sizeof psi, NULL);
      if (!NT_SUCCESS (status) || psi.SessionId == 0)
	session_parent_dir = get_shared_parent_dir ();
      else
	{
	  WCHAR bnoname[MAX_PATH];
	  __small_swprintf (bnoname,
			    L"\\Sessions\\BNOLINKS\\%d\\%s%s-%S",
			    psi.SessionId, cygwin_version.shared_id,
			    _cygwin_testing ? cygwin_version.dll_build_date : "",
			    &installation_key);
	  RtlInitUnicodeString (&uname, bnoname);
	  InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF, NULL,
				      everyone_sd(CYG_SHARED_DIR_ACCESS));
	  status = NtCreateDirectoryObject (&session_parent_dir,
					    CYG_SHARED_DIR_ACCESS, &attr);
	  if (!NT_SUCCESS (status))
	    api_fatal ("NtCreateDirectoryObject(%S): %p", &uname, status);
	}
    }
  return session_parent_dir;
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

/* The order in offsets is so that the constant blocks shared_info
   and user_info are right below the cygwin DLL, then the pinfo block
   which changes with each process.  Below that is the console_state,
   an optional block which only exists when running in a Windows console
   window.  Therefore, if we are not running in a console, we have 64K
   more of contiguous memory below the Cygwin DLL. */
static ptrdiff_t offsets[] =
{
  - pround (sizeof (shared_info)),		/* SH_CYGWIN_SHARED */
  - pround (sizeof (shared_info))		/* SH_USER_SHARED */
  - pround (sizeof (user_info)),
  - pround (sizeof (shared_info))		/* SH_MYSELF */
  - pround (sizeof (user_info))
  - pround (sizeof (_pinfo)),
  - pround (sizeof (shared_info))		/* SH_SHARED_CONSOLE */
  - pround (sizeof (user_info))
  - pround (sizeof (_pinfo))
  - pround (sizeof (fhandler_console::console_state)),
  0
};

#define off_addr(x)	((void *)((caddr_t) cygwin_hmodule + offsets[x]))

void * __stdcall
open_shared (const WCHAR *name, int n, HANDLE& shared_h, DWORD size,
	     shared_locations m, PSECURITY_ATTRIBUTES psa, DWORD access)
{
  return open_shared (name, n, shared_h, size, &m, psa, access);
}

void * __stdcall
open_shared (const WCHAR *name, int n, HANDLE& shared_h, DWORD size,
	     shared_locations *m, PSECURITY_ATTRIBUTES psa, DWORD access)
{
  void *shared;

  void *addr;
  if (*m == SH_JUSTCREATE || *m == SH_JUSTOPEN)
    addr = NULL;
  else
    {
      addr = off_addr (*m);
      VirtualFree (addr, 0, MEM_RELEASE);
    }

  WCHAR map_buf[MAX_PATH];
  WCHAR *mapname = NULL;

  if (shared_h)
    *m = SH_JUSTOPEN;
  else
    {
      if (name)
	mapname = shared_name (map_buf, name, n);
      if (*m == SH_JUSTOPEN)
	shared_h = OpenFileMappingW (access, FALSE, mapname);
      else
	{
	  shared_h = CreateFileMappingW (INVALID_HANDLE_VALUE, psa,
					PAGE_READWRITE, 0, size, mapname);
	  if (GetLastError () == ERROR_ALREADY_EXISTS)
	    *m = SH_JUSTOPEN;
	}
      if (shared_h)
	/* ok! */;
      else if (*m != SH_JUSTOPEN)
	api_fatal ("CreateFileMapping %W, %E.  Terminating.", mapname);
      else
	return NULL;
    }

  shared = (shared_info *) MapViewOfFileEx (shared_h, access, 0, 0, 0, addr);

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

  if (*m == SH_CYGWIN_SHARED && offsets[0])
    {
      /* Reserve subsequent shared memory areas in non-relocated case only.
	 There's no good reason to reserve the console shmem, because it's
	 not yet known if we will allocate it at all. */
      for (int i = SH_USER_SHARED; i < SH_SHARED_CONSOLE; i++)
	{
	  DWORD size = offsets[i - 1] - offsets[i];
	  if (!VirtualAlloc (off_addr (i), size, MEM_RESERVE, PAGE_NOACCESS))
	    continue;  /* oh well */
	}
    }

  debug_printf ("name %W, n %d, shared %p (wanted %p), h %p, *m %d",
		mapname, n, shared, addr, shared_h, *m);

  return shared;
}

/* Second half of user shared initialization: Initialize content. */
void
user_info::initialize ()
{
  /* Wait for initialization of the Cygwin per-user shared, if necessary */
  spinlock sversion (version, CURR_USER_MAGIC);
  if (!sversion)
    {
      cb =  sizeof (*user_shared);
      cygpsid sid (cygheap->user.sid ());
      struct passwd *pw = internal_getpwsid (sid);
      /* Correct the user name with what's defined in /etc/passwd before
	 loading the user fstab file. */
      if (pw)
	cygheap->user.set_name (pw->pw_name);
      mountinfo.init ();	/* Initialize the mount table.  */
    }
  else if (sversion != CURR_USER_MAGIC)
    sversion.multiple_cygwin_problem ("user shared memory version", version,
				      sversion);
  else if (user_shared->cb != sizeof (*user_shared))
    sversion.multiple_cygwin_problem ("user shared memory size", cb,
				      sizeof (*user_shared));
}

/* First half of user shared initialization: Create shared mem region. */
void
user_info::create (bool reinit)
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

  user_shared = (user_info *) open_shared (name, USER_VERSION,
					   cygwin_user_h, sizeof (user_info),
					   SH_USER_SHARED, &sec_none);
  debug_printf ("opening user shared for '%W' at %p", name, user_shared);
  ProtectHandleINH (cygwin_user_h);
  debug_printf ("user shared version %x", user_shared->version);
  if (reinit)
    user_shared->initialize ();
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
  NTSTATUS status;
  DWORD def_obcaseinsensitive = 1;

  obcaseinsensitive = def_obcaseinsensitive;
  RTL_QUERY_REGISTRY_TABLE tab[2] = {
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
      L"obcaseinsensitive", &obcaseinsensitive, REG_DWORD,
      &def_obcaseinsensitive, sizeof (DWORD) },
    { NULL, 0, NULL, NULL, 0, NULL, 0 }
  };
  status = RtlQueryRegistryValues (RTL_REGISTRY_CONTROL,
				   L"Session Manager\\kernel",
				   tab, NULL, NULL);
}

void inline
shared_info::create ()
{
  cygwin_shared = (shared_info *) open_shared (L"shared",
					       CYGWIN_VERSION_SHARED_DATA,
					       cygwin_shared_h,
					       sizeof (*cygwin_shared),
					       SH_CYGWIN_SHARED,
					       &sec_all_nih);
  cygwin_shared->initialize ();
}

void
shared_info::initialize ()
{
  spinlock sversion (version, CURR_SHARED_MAGIC);
  if (!sversion)
    {
      cb = sizeof (*this);
      get_session_parent_dir ();	/* Create session dir if first process. */
      init_obcaseinsensitive ();	/* Initialize obcaseinsensitive */
      tty.init ();			/* Initialize tty table  */
      mt.initialize ();			/* Initialize shared tape information */
      /* Defer debug output printing the installation root and installation key
	 up to this point.  Debug output except for system_printf requires
	 the global shared memory to exist. */
      debug_printf ("Installation root: <%W> key: <%S>",
		    installation_root, &installation_key);
    }
  else if (sversion != (LONG) CURR_SHARED_MAGIC)
    sversion.multiple_cygwin_problem ("system shared memory version",
				      sversion, CURR_SHARED_MAGIC);
  else if (cb != sizeof (*this))
    system_printf ("size of shared memory region changed from %u to %u",
		   sizeof (*this), cb);
  heap_init ();
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

  init_installation_root ();	/* Initialize installation root dir */
  shared_info::create ();	/* Initialize global shared memory */
  user_info::create (false);	/* Initialize per-user shared memory */
}
