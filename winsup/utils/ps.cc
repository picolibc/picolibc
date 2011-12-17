/* ps.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2008, 2009, 2010, 2011 Red Hat, Inc.

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
#include <ddk/ntapi.h>
#include <ddk/winddk.h>
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

char dosdevs[32000];

int
main (int argc, char *argv[])
{
  external_pinfo *p;
  int aflag, lflag, fflag, sflag, uid, proc_id;
  bool found_proc_id = true;
  cygwin_getinfo_types query = CW_GETPINFO;
  const char *dtitle = "    PID  TTY        STIME COMMAND\n";
  const char *dfmt   = "%7d%4s%10s %s\n";
  const char *ftitle = "     UID     PID    PPID  TTY        STIME COMMAND\n";
  const char *ffmt   = "%8.8s%8d%8d%4s%10s %s\n";
  const char *ltitle = "      PID    PPID    PGID     WINPID   TTY     UID    STIME COMMAND\n";
  const char *lfmt   = "%c %7d %7d %7d %10u %4s %4u %8s %s\n";
  char ch;
  PUNICODE_STRING uni = NULL;

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
	  if (p->uid32 != (__uid32_t) uid)
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
	  HANDLE h = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				  FALSE, p->dwProcessId);
	  if (!h)
	    continue;
	  HMODULE hm[1000];
	  DWORD n = p->dwProcessId;
	  if (!EnumProcessModules (h, hm, sizeof (hm), &n))
	    n = 0;
	  /* This occurs when trying to enum modules of a 64 bit process.
	     GetModuleFileNameEx with a NULL module will return the same error.
	     Only NtQueryInformationProcess allows to fetch the process image
	     name in that case. */
	  if (!n && GetLastError () == ERROR_PARTIAL_COPY)
	    {
	      NTSTATUS status;
	      char pbuf[PATH_MAX];
	      char dev[256];
	      size_t len = sizeof (UNICODE_STRING) + PATH_MAX * sizeof (WCHAR);

	      if (!uni)
		{
		  uni = (PUNICODE_STRING) alloca (len);
		  QueryDosDevice (NULL, dosdevs, 32000);
		}
	      status = NtQueryInformationProcess (h, ProcessImageFileName, uni,
						  len, NULL);
	      if (NT_SUCCESS (status)
		  && (len = wcsnrtombs (pbuf, (const wchar_t **) &uni->Buffer,
					uni->Length / sizeof (WCHAR), PATH_MAX,
					NULL)) > 0)
		{
		  pbuf[len] = '\0';
		  if (!strncmp (pbuf, "\\Device\\Mup\\", 12))
		    {
		      strcpy (pname, "\\\\");
		      strcpy (pname + 2, pbuf + 12);
		    }
		  else
		    {
		      strcpy (pname, pbuf);
		      for (char *d = dosdevs; *d; d = strchr (d, '\0') + 1)
			if (QueryDosDevice (d, dev, 256)
			    && strlen (d) < 3
			    && !strncmp (dev, pbuf, strlen (dev)))
			  {
			    strcpy (pname, d);
			    strcat (pname, pbuf + strlen (dev));
			    break;
			  }
		    }
		}
	      else
		strcpy (pname, "*** unknown ***");
	    }
	  else
	    {
	      wchar_t pwname[NT_MAX_PATH];

	      if (!n || !GetModuleFileNameExW (h, hm[0], pwname, NT_MAX_PATH))
		strcpy (pname, "*** unknown ***");
	      else
		wcstombs (pname, pwname, NT_MAX_PATH);
	    }
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
  (void) cygwin_internal (CW_UNLOCK_PINFO);

  return found_proc_id ? 0 : 1;
}
