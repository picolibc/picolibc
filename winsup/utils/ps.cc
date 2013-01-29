/* ps.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <windows.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <limits.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <psapi.h>
#include <ntdef.h>
#include <ntdll.h>
#include "loadlib.h"

/* Maximum possible path length under NT.  There's no official define
   for that value.  Note that PATH_MAX is only 4K. */
#define NT_MAX_PATH 32768

static char *prog_name;

static struct option longopts[] =
{
  {"all", no_argument, NULL, 'a' },
  {"everyone", no_argument, NULL, 'e' },
  {"full", no_argument, NULL, 'f' },
  {"help", no_argument, NULL, 'h' },
  {"long", no_argument, NULL, 'l' },
  {"process", required_argument, NULL, 'p'},
  {"summary", no_argument, NULL, 's' },
  {"user", required_argument, NULL, 'u'},
  {"version", no_argument, NULL, 'V'},
  {"windows", no_argument, NULL, 'W'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "aefhlp:su:VW";

static char *
start_time (external_pinfo *child)
{
  time_t st = child->start_time;
  time_t t = time (NULL);
  static char stime[40] = {'\0'};
  char now[40];

  strncpy (stime, ctime (&st) + 4, 15);
  strcpy (now, ctime (&t) + 4);

  if ((t - st) < (24 * 3600))
    return (stime + 7);

  stime[6] = '\0';

  return stime;
}

#define FACTOR (0x19db1ded53ea710LL)
#define NSPERSEC 10000000LL

/* Convert a Win32 time to "UNIX" format. */
long __stdcall
to_time_t (FILETIME *ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A time_t is the number of seconds since jan 1 1970.  */

  long rem;
  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned)ptr->dwLowDateTime);
  x -= FACTOR;                  /* number of 100ns between 1601 and 1970 */
  rem = x % ((long long)NSPERSEC);
  rem += (NSPERSEC / 2);
  x /= (long long) NSPERSEC;            /* number of 100ns in a second */
  x += (long long) (rem / NSPERSEC);
  return x;
}

static const char *
ttynam (int ntty)
{
  static char buf[9];
  char buf0[9];
  if (ntty < 0)
    strcpy (buf0, "?");
  else if (ntty & 0xffff0000)
    sprintf (buf0, "cons%d", ntty & 0xff);
  else
    sprintf (buf0, "pty%d", ntty);
  sprintf (buf, " %-7s", buf0);
  return buf;
}

static void
usage (FILE * stream, int status)
{
  fprintf (stream, "\
Usage: %1$s [-aefls] [-u UID] [-p PID]\n\
\n\
Report process status\n\
\n\
 -a, --all       show processes of all users\n\
 -e, --everyone  show processes of all users\n\
 -f, --full      show process uids, ppids\n\
 -h, --help      output usage information and exit\n\
 -l, --long      show process uids, ppids, pgids, winpids\n\
 -p, --process   show information for specified PID\n\
 -s, --summary   show process summary\n\
 -u, --user      list processes owned by UID\n\
 -V, --version   output version information and exit\n\
 -W, --windows   show windows as well as cygwin processes\n\
\n\
With no options, %1$s outputs the long format by default\n\n",
	   prog_name);
  exit (status);
}

static void
print_version ()
{
  printf ("ps (cygwin) %d.%d.%d\n"
	  "Show process statistics\n"
	  "Copyright (C) 1996 - %s Red Hat, Inc.\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

char unicode_buf[sizeof (UNICODE_STRING) + NT_MAX_PATH];

int
main (int argc, char *argv[])
{
  external_pinfo *p;
  int aflag, lflag, fflag, sflag, proc_id;
  uid_t uid;
  bool found_proc_id = true;
  DWORD proc_access = PROCESS_QUERY_LIMITED_INFORMATION;
  cygwin_getinfo_types query = CW_GETPINFO;
  const char *dtitle = "    PID  TTY        STIME COMMAND\n";
  const char *dfmt   = "%7d%4s%10s %s\n";
  const char *ftitle = "     UID     PID    PPID  TTY        STIME COMMAND\n";
  const char *ffmt   = "%8.8s%8d%8d%4s%10s %s\n";
  const char *ltitle = "      PID    PPID    PGID     WINPID   TTY     UID    STIME COMMAND\n";
  const char *lfmt   = "%c %7d %7d %7d %10u %4s %4u %8s %s\n";
  char ch;
  PUNICODE_STRING uni = (PUNICODE_STRING) unicode_buf;
  void *drive_map = NULL;

  aflag = lflag = fflag = sflag = 0;
  uid = getuid ();
  proc_id = -1;
  lflag = 1;

  setlocale (LC_ALL, "");

  prog_name = program_invocation_short_name;

  while ((ch = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (ch)
      {
      case 'a':
      case 'e':
	aflag = 1;
	break;
      case 'f':
	fflag = 1;
	break;
      case 'h':
	usage (stdout, 0);
      case 'l':
	lflag = 1;
	break;
      case 'p':
	proc_id = atoi (optarg);
	aflag = 1;
	found_proc_id = false;
	break;
      case 's':
	sflag = 1;
	break;
      case 'u':
	uid = atoi (optarg);
	if (uid == 0)
	  {
	    struct passwd *pw;

	    if ((pw = getpwnam (optarg)))
	      uid = pw->pw_uid;
	    else
	      {
		fprintf (stderr, "%s: user %s unknown\n", prog_name, optarg);
		exit (1);
	      }
	  }
	break;
      case 'V':
	print_version ();
	exit (0);
	break;
      case 'W':
	query = CW_GETPINFO_FULL;
	aflag = 1;
	break;

      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", prog_name);
	exit (1);
      }

  if (sflag)
    printf (dtitle);
  else if (fflag)
    printf (ftitle);
  else if (lflag)
    printf (ltitle);

  (void) cygwin_internal (CW_LOCK_PINFO, 1000);

  if (query == CW_GETPINFO_FULL)
    {
      /* Enable debug privilege to allow to enumerate all processes,
	 not only processes in current session. */
      HANDLE tok;
      if (OpenProcessToken (GetCurrentProcess (),
			    TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
			    &tok))
	{
	  TOKEN_PRIVILEGES priv;

	  priv.PrivilegeCount = 1;
	  if (LookupPrivilegeValue (NULL, SE_DEBUG_NAME,
				    &priv.Privileges[0].Luid))
	    {
	      priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	      AdjustTokenPrivileges (tok, FALSE, &priv, 0, NULL, NULL);
	    }
	}

      /* Check process query capabilities. */
      OSVERSIONINFO version;
      version.dwOSVersionInfoSize = sizeof version;
      GetVersionEx (&version);
      if (version.dwMajorVersion <= 5)	/* pre-Vista */
	{
	  proc_access = PROCESS_QUERY_INFORMATION;
	  if (version.dwMinorVersion < 1)	/* Windows 2000 */
	    proc_access |= PROCESS_VM_READ;
	  else
	    {
	    }
	}

      /* Except on Windows 2000, fetch an opaque drive mapping object from the
	 Cygwin DLL.  This is used to map NT device paths to Win32 paths. */
      if (!(proc_access & PROCESS_VM_READ))
	{
	  drive_map = (void *) cygwin_internal (CW_ALLOC_DRIVE_MAP);
	  /* Check old Cygwin version. */
	  if (drive_map == (void *) -1)
	    drive_map = NULL;
	  /* Allow fallback to GetModuleFileNameEx for post-W2K. */
	  if (!drive_map)
	    proc_access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
	}
    }

  for (int pid = 0;
       (p = (external_pinfo *) cygwin_internal (query, pid | CW_NEXTPID));
       pid = p->pid)
    {
      if ((proc_id > 0) && (p->pid != proc_id))
	continue;
      else
	found_proc_id = true;

      if (aflag)
	/* nothing to do */;
      else if (p->version >= EXTERNAL_PINFO_VERSION_32_BIT)
	{
	  if (p->uid32 != uid)
	    continue;
	}
      else if (p->uid != uid)
	continue;
      char status = ' ';
      if (p->process_state & PID_STOPPED)
	status = 'S';
      else if (p->process_state & PID_TTYIN)
	status = 'I';
      else if (p->process_state & PID_TTYOU)
	status = 'O';

      /* Maximum possible path length under NT.  There's no official define
	 for that value. */
      char pname[NT_MAX_PATH + sizeof (" <defunct>")];
      if (p->ppid)
	{
	  char *s;
	  pname[0] = '\0';
	  strncat (pname, p->progname_long, NT_MAX_PATH);
	  s = strchr (pname, '\0') - 4;
	  if (s > pname && strcasecmp (s, ".exe") == 0)
	    *s = '\0';
	  if (p->process_state & PID_EXITED || (p->exitcode & ~0xffff))
	    strcat (pname, " <defunct>");
	}
      else if (query == CW_GETPINFO_FULL)
	{
	  HANDLE h;
	  NTSTATUS status;
	  wchar_t *win32path = NULL;

	  h = OpenProcess (proc_access, FALSE, p->dwProcessId);
	  if (!h)
	    continue;
	  /* We use NtQueryInformationProcess in the first place, because
	     GetModuleFileNameEx does not work on 64 bit systems when trying
	     to fetch module names of 64 bit processes. */
	  if (!(proc_access & PROCESS_VM_READ))	/* Windows 2000 */
	    {
	      status = NtQueryInformationProcess (h, ProcessImageFileName, uni,
						  sizeof unicode_buf, NULL);
	      if (NT_SUCCESS (status))
		{
		  /* NtQueryInformationProcess returns a native NT device path.
		     Call CW_MAP_DRIVE_MAP to convert the path to an ordinary
		     Win32 path.  The returned pointer is a pointer into the
		     incoming buffer given as third argument.  It's expected
		     to be big enough, which we made sure by defining
		     unicode_buf to have enough space for a maximum sized
		     UNICODE_STRING. */
		  if (uni->Length == 0)	/* System process */
		    win32path = (wchar_t *) L"System";
		  else
		    {
		      uni->Buffer[uni->Length / sizeof (WCHAR)] = L'\0';
		      win32path = (wchar_t *) cygwin_internal (CW_MAP_DRIVE_MAP,
							       drive_map,
							       uni->Buffer);
		    }
		}
	    }
	  else
	    {
	      if (GetModuleFileNameExW (h, NULL, (PWCHAR) unicode_buf,
					NT_MAX_PATH))
		win32path = (wchar_t *) unicode_buf;
	    }
	  if (win32path)
	    wcstombs (pname, win32path, sizeof pname);
	  else
	    strcpy (pname, "*** unknown ***");
	  FILETIME ct, et, kt, ut;
	  if (GetProcessTimes (h, &ct, &et, &kt, &ut))
	    p->start_time = to_time_t (&ct);
	  CloseHandle (h);
	}

      char uname[128];

      if (fflag)
	{
	  struct passwd *pw;

	  if ((pw = getpwuid (p->version >= EXTERNAL_PINFO_VERSION_32_BIT ?
			      p->uid32 : p->uid)))
	    strcpy (uname, pw->pw_name);
	  else
	    sprintf (uname, "%u", (unsigned)
		     (p->version >= EXTERNAL_PINFO_VERSION_32_BIT ?
		      p->uid32 : p->uid));
	}

      if (sflag)
	printf (dfmt, p->pid, ttynam (p->ctty), start_time (p), pname);
      else if (fflag)
	printf (ffmt, uname, p->pid, p->ppid, ttynam (p->ctty), start_time (p),
		pname);
      else if (lflag)
	printf (lfmt, status, p->pid, p->ppid, p->pgid,
		p->dwProcessId, ttynam (p->ctty),
		p->version >= EXTERNAL_PINFO_VERSION_32_BIT ? p->uid32 : p->uid,
		start_time (p), pname);

    }
  if (drive_map)
    cygwin_internal (CW_FREE_DRIVE_MAP, drive_map);
  (void) cygwin_internal (CW_UNLOCK_PINFO);

  return found_proc_id ? 0 : 1;
}
