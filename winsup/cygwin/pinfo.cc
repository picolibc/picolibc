/* pinfo.cc: process table support

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

static char NO_COPY pinfo_dummy[sizeof(pinfo)] = {0};

pinfo NO_COPY myself ((_pinfo *)&pinfo_dummy);	// Avoid myself != NULL checks
static pinfo NO_COPY myself_identity ((_pinfo *)&pinfo_dummy);

/* Initialize the process table.
   This is done once when the dll is first loaded.  */

void __stdcall
set_myself (pid_t pid)
{
  DWORD winpid = GetCurrentProcessId ();
  if (pid == 1)
    pid = cygwin_pid (winpid);
  myself.init (pid, 1);
  myself->dwProcessId = winpid;
  myself->process_state |= PID_IN_USE;
  myself->start_time = time (NULL); /* Register our starting time. */
  pid_t myself_cyg_pid = cygwin_pid (myself->dwProcessId);
  if (pid != myself_cyg_pid)
    myself_identity.init (myself_cyg_pid, PID_EXECED);

  char buf[30];
  __small_sprintf (buf, "cYg%8x %x", _STRACE_INTERFACE_ACTIVATE_ADDR,
		   &strace.active);
  OutputDebugString (buf);

  (void) GetModuleFileName (NULL, myself->progname,
			    sizeof(myself->progname));
  if (strace.active)
    {
      extern char osname[];
      strace.prntf (1, NULL, "**********************************************");
      strace.prntf (1, NULL, "Program name: %s", myself->progname);
      strace.prntf (1, NULL, "App version:  %d.%d, api: %d.%d",
		       user_data->dll_major, user_data->dll_minor,
		       user_data->api_major, user_data->api_minor);
      strace.prntf (1, NULL, "DLL version:  %d.%d, api: %d.%d",
		       cygwin_version.dll_major, cygwin_version.dll_minor,
		       cygwin_version.api_major, cygwin_version.api_minor);
      strace.prntf (1, NULL, "DLL build:    %s", cygwin_version.dll_build_date);
      strace.prntf (1, NULL, "OS version:   Windows %s", osname);
      strace.prntf (1, NULL, "**********************************************");
    }

  return;
}

/* Initialize the process table entry for the current task.
   This is not called for fork'd tasks, only exec'd ones.  */
void __stdcall
pinfo_init (LPBYTE info)
{
  if (info != NULL)
    {
      /* The process was execed.  Reuse entry from the original
	 owner of this pid. */
      environ_init (0);	  /* Needs myself but affects calls below */

      /* spawn has already set up a pid structure for us so we'll use that */

      myself->process_state |= PID_CYGPARENT;

      /* Inherit file descriptor information from parent in info.
       */
      LPBYTE b = fdtab.de_linearize_fd_array (info);
      extern char title_buf[];
      if (b && *b)
	old_title = strcpy (title_buf, (char *)b);
    }
  else
    {
      /* Invent our own pid.  */

      set_myself (1);
      myself->ppid = 1;
      myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = USHRT_MAX;

      environ_init (0);		/* call after myself has been set up */
    }

  debug_printf ("pid %d, pgid %d", myself->pid, myself->pgid);
}

struct sigaction&
_pinfo::getsig(int sig)
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->sigs[sig];
  return sigs[sig];
#else
  return sigs[sig];
#endif
};

sigset_t&
_pinfo::getsigmask ()
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return *thread2signal->sigmask;
  return sig_mask;
#else
  return sig_mask;
#endif
};

void
_pinfo::setsigmask (sigset_t _mask)
{
#ifdef _MT_SAFE
  if ( thread2signal )
	*(thread2signal->sigmask) = _mask;
  sig_mask=_mask;
#else
  sig_mask=_mask;
#endif
}

LONG *
_pinfo::getsigtodo(int sig)
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->sigtodo + __SIGOFFSET + sig;
  return _sigtodo + __SIGOFFSET + sig;
#else
  return _sigtodo + __SIGOFFSET + sig;
#endif
}

extern HANDLE hMainThread;

HANDLE
_pinfo::getthread2signal()
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->win32_obj_id;
  return hMainThread;
#else
  return hMainThread;
#endif
}

void
_pinfo::setthread2signal(void *_thr)
{
#ifdef _MT_SAFE
   // assert has myself lock
   thread2signal=(ThreadItem*)_thr;
#else
#endif
}

void
_pinfo::copysigs(_pinfo *_other)
{
  sigs = _other->sigs;
}

void
_pinfo::record_death ()
{
  /* CGF FIXME - needed? */
  if (dwProcessId == GetCurrentProcessId () && !my_parent_is_alive ())
    {
      process_state = PID_NOT_IN_USE;
      hProcess = NULL;
    }
}

void
pinfo::init (pid_t n, DWORD create)
{
  if (n == myself->pid)
    {
      child = myself;
      destroy = 0;
      h = NULL;
      return;
    }

  int created;
  char mapname[MAX_PATH];
  __small_sprintf (mapname, "cygpid.%x", n);

  int mapsize;
  if (create & PID_EXECED)
    mapsize = PINFO_REDIR_SIZE;
  else
    mapsize = sizeof (_pinfo);

  if (!create)
    {
      /* CGF FIXME -- deal with inheritance after an exec */
      h = OpenFileMappingA (FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapname);
      created = 0;
    }
  else
    {
      h = CreateFileMapping ((HANDLE) 0xffffffff, &sec_none_nih,
			      PAGE_READWRITE, 0, mapsize, mapname);
      created = h && GetLastError () != ERROR_ALREADY_EXISTS;
    }

  if (!h)
    {
      if (create)
	__seterrno ();
      child = NULL;
      return;
    }

  child = (_pinfo *) MapViewOfFile (h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

  if (child->process_state & PID_EXECED)
    {
      pid_t realpid = child->pid;
      release ();
      if (realpid == n)
	api_fatal ("retrieval of execed process info for pid %d failed due to recursion.", n);
      return init (realpid);
    }

  if (created)
    {
      if (!(create & PID_EXECED))
	child->pid = n;
      else
	{
	  child->pid = myself->pid;
	  child->process_state |= PID_IN_USE | PID_EXECED;
	}
    }

  destroy = 1;
}

/* DOCTOOL-START

<sect1 id="func-cygwin-winpid-to-pid">
  <title>cygwin_winpid_to_pid</title>

  <funcsynopsis>
    <funcdef>extern "C" pid_t
      <function>cygwin_winpid_to_pid</function>
      </funcdef>
      <paramdef>int <parameter>winpid</parameter></paramdef>
  </funcsynopsis>

  <para>Given a windows pid, converts to the corresponding Cygwin
pid, if any.  Returns -1 if windows pid does not correspond to
a cygwin pid.</para>
  <example>
    <title>Example use of cygwin_winpid_to_pid</title>
    <programlisting>
      extern "C" cygwin_winpid_to_pid (int winpid);
      pid_t mypid;
      mypid = cygwin_winpid_to_pid (windows_pid);
    </programlisting>
  </example>
</sect1>

   DOCTOOL-END */

extern "C" pid_t
cygwin_winpid_to_pid (int winpid)
{
  pinfo p (winpid);
  if (p)
    return p->pid;

  set_errno (ESRCH);
  return (pid_t) -1;
}

#include <tlhelp32.h>
#include <psapi.h>

typedef BOOL (WINAPI * ENUMPROCESSES) (DWORD *, DWORD, DWORD *);
typedef HANDLE (WINAPI * CREATESNAPSHOT) (DWORD, DWORD);
typedef BOOL (WINAPI * PROCESSWALK) (HANDLE, LPPROCESSENTRY32);
typedef BOOL (WINAPI * CLOSESNAPSHOT) (HANDLE);

static NO_COPY CREATESNAPSHOT myCreateToolhelp32Snapshot = NULL;
static NO_COPY PROCESSWALK myProcess32First = NULL;
static NO_COPY PROCESSWALK myProcess32Next  = NULL;
static BOOL WINAPI enum_init (DWORD *lpidProcess, DWORD cb, DWORD *cbneeded);

static NO_COPY ENUMPROCESSES myEnumProcesses = enum_init;

static BOOL WINAPI
EnumProcessesW95 (DWORD *lpidProcess, DWORD cb, DWORD *cbneeded)
{
  HANDLE h;

  *cbneeded = 0;
  h = myCreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
  if (!h)
    return 0;

  PROCESSENTRY32 proc;
  int i = 0;
  proc.dwSize = sizeof (proc);
  if (myProcess32First(h, &proc))
    do
      lpidProcess[i++] = cygwin_pid (proc.th32ProcessID);
    while (myProcess32Next (h, &proc));
  CloseHandle (h);
  if (i == 0)
    return 0;
  *cbneeded = i * sizeof (DWORD);
  return 1;
}

void
winpids::init ()
{
  DWORD n;
  if (!myEnumProcesses (pidlist, sizeof (pidlist) / sizeof (pidlist[0]), &n))
    npids = 0;
  else
    npids = n / sizeof (pidlist[0]);

  pidlist[npids] = 0;
}

static BOOL WINAPI
enum_init (DWORD *lpidProcess, DWORD cb, DWORD *cbneeded)
{
  HINSTANCE h;
  if (os_being_run == winNT)
    {
      h = LoadLibrary ("psapi.dll");
      if (!h)
	return 0;
      myEnumProcesses = (ENUMPROCESSES) GetProcAddress (h, "EnumProcesses");
      if (!myEnumProcesses)
	return 0;
    }
  else
    {
      h = GetModuleHandle("kernel32.dll");
      myCreateToolhelp32Snapshot = (CREATESNAPSHOT)
		  GetProcAddress(h, "CreateToolhelp32Snapshot");
      myProcess32First = (PROCESSWALK)
	      GetProcAddress(h, "Process32First");
      myProcess32Next  = (PROCESSWALK)
	      GetProcAddress(h, "Process32Next");
      if (!myCreateToolhelp32Snapshot || !myProcess32First || !myProcess32Next)
	return 0;

      myEnumProcesses = EnumProcessesW95;
    }

  return myEnumProcesses (lpidProcess, cb, cbneeded);
}
