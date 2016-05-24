/* shared.cc: shared data area support.

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
#include "spinlock.h"
#include <alloca.h>
#include <wchar.h>

shared_info NO_COPY *cygwin_shared;
user_info NO_COPY *user_shared;
HANDLE NO_COPY cygwin_shared_h;
HANDLE NO_COPY cygwin_user_h;

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
			&cygheap->installation_key);
      RtlInitUnicodeString (&uname, bnoname);
      InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF, NULL,
				  everyone_sd (CYG_SHARED_DIR_ACCESS));
      status = NtCreateDirectoryObject (&shared_parent_dir,
					CYG_SHARED_DIR_ACCESS, &attr);
      if (!NT_SUCCESS (status))
	api_fatal ("NtCreateDirectoryObject(%S): %y", &uname, status);
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
			    &cygheap->installation_key);
	  RtlInitUnicodeString (&uname, bnoname);
	  InitializeObjectAttributes (&attr, &uname, OBJ_OPENIF, NULL,
				      everyone_sd(CYG_SHARED_DIR_ACCESS));
	  status = NtCreateDirectoryObject (&session_parent_dir,
					    CYG_SHARED_DIR_ACCESS, &attr);
	  if (!NT_SUCCESS (status))
	    api_fatal ("NtCreateDirectoryObject(%S): %y", &uname, status);
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

#define page_const ((ptrdiff_t) 65535)
#define pround(n) ((ptrdiff_t)(((n) + page_const) & ~page_const))

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
      cb = sizeof (*user_shared);
      /* Initialize mount table from system fstab prior to calling
         internal_getpwsid.  This allows to convert pw_dir and pw_shell
	 paths given in DOS notation to valid POSIX paths.  */
      mountinfo.init (false);
      cygpsid sid (cygheap->user.sid ());
      struct passwd *pw = internal_getpwsid (sid);
      /* Correct the user name with what's defined in /etc/passwd before
	 loading the user fstab file. */
      if (pw)
	cygheap->user.set_name (pw->pw_name);
      /* After fetching the user infos, add mount entries from user's fstab. */
      mountinfo.init (true);
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

/* Initialize obcaseinsensitive.*/
void
shared_info::init_obcaseinsensitive ()
{
  /* Instead of reading the obcaseinsensitive registry value, test the
     actual state of case sensitivity handling in the kernel. */
  UNICODE_STRING sysroot;
  OBJECT_ATTRIBUTES attr;
  HANDLE h;

  RtlInitUnicodeString (&sysroot, L"\\SYSTEMROOT");
  InitializeObjectAttributes (&attr, &sysroot, 0, NULL, NULL);
  /* NtOpenSymbolicLinkObject returns STATUS_ACCESS_DENIED when called
     with a 0 access mask.  However, if the kernel is case sensitive,
     it returns STATUS_OBJECT_NAME_NOT_FOUND because we used the incorrect
     case for the filename (It's actually "\\SystemRoot"). */
  obcaseinsensitive = NtOpenSymbolicLinkObject (&h, 0, &attr)
		      != STATUS_OBJECT_NAME_NOT_FOUND;
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
		    cygheap->installation_root, &cygheap->installation_key);
    }
  else if (sversion != CURR_SHARED_MAGIC)
    sversion.multiple_cygwin_problem ("system shared memory version",
				      sversion, CURR_SHARED_MAGIC);
  else if (cb != sizeof (*this))
    system_printf ("size of shared memory region changed from %lu to %u",
		   sizeof (*this), cb);
  /* FIXME? Shouldn't this be in memory_init? */
  cygheap->user_heap.init ();
}

void
memory_init ()
{
  shared_info::create ();	/* Initialize global shared memory */
  user_info::create (false);	/* Initialize per-user shared memory */
  /* Initialize tty list session stuff.  Doesn't really belong here but
     this needs to be initialized before any tty or console manipulation
     happens and it is a common location.  */
  tty_list::init_session ();
}
