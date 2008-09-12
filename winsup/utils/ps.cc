/* ps.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <limits.h>
#include <sys/cygwin.h>
#include <tlhelp32.h>
#include <psapi.h>

/* Maximum possible path length under NT.  There's no official define
   for that value.  Note that PATH_MAX is only 4K. */
#define NT_MAX_PATH 32768

static const char version[] = "$Revision$";
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
  {"version", no_argument, NULL, 'v'},
  {"windows", no_argument, NULL, 'W'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "aefhlp:su:vW";

typedef BOOL (WINAPI *ENUMPROCESSMODULES)(
  HANDLE hProcess,      // handle to the process
  HMODULE * lphModule,  // array to receive the module handles
  DWORD cb,             // size of the array
  LPDWORD lpcbNeeded    // receives the number of bytes returned
);

typedef DWORD (WINAPI *GETMODULEFILENAME)(
  HANDLE hProcess,
  HMODULE hModule,
  LPTSTR lpstrFileName,
  DWORD nSize
);

typedef HANDLE (WINAPI *CREATESNAPSHOT)(
    DWORD dwFlags,
    DWORD th32ProcessID
);

// Win95 functions
typedef BOOL (WINAPI *PROCESSWALK)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
);

ENUMPROCESSMODULES myEnumProcessModules;
GETMODULEFILENAME myGetModuleFileNameEx;
CREATESNAPSHOT myCreateToolhelp32Snapshot;
PROCESSWALK myProcess32First;
PROCESSWALK myProcess32Next;

static BOOL WINAPI dummyprocessmodules (
  HANDLE hProcess,      // handle to the process
  HMODULE * lphModule,  // array to receive the module handles
  DWORD cb,             // size of the array
  LPDWORD lpcbNeeded    // receives the number of bytes returned
)
{
  lphModule[0] = (HMODULE) *lpcbNeeded;
  *lpcbNeeded = 1;
  return 1;
}

static DWORD WINAPI GetModuleFileNameEx95 (
  HANDLE hProcess,
  HMODULE hModule,
  LPTSTR lpstrFileName,
  DWORD n
)
{
  HANDLE h;
  DWORD pid = (DWORD) hModule;

  h = myCreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
  if (!h)
    return 0;

  PROCESSENTRY32 proc;
  proc.dwSize = sizeof (proc);
  if (myProcess32First(h, &proc))
    do
      if (proc.th32ProcessID == pid)
	{
	  CloseHandle (h);
	  strcpy (lpstrFileName, proc.szExeFile);
	  return 1;
	}
    while (myProcess32Next (h, &proc));
  CloseHandle (h);
  return 0;
}

int
init_win ()
{
  OSVERSIONINFO os_version_info;

  memset (&os_version_info, 0, sizeof os_version_info);
  os_version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&os_version_info);

  HMODULE h;
  if (os_version_info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
      h = LoadLibrary ("psapi.dll");
      if (!h)
	return 0;
      myEnumProcessModules = (ENUMPROCESSMODULES) GetProcAddress (h, "EnumProcessModules");
      myGetModuleFileNameEx = (GETMODULEFILENAME) GetProcAddress (h, "GetModuleFileNameExA");
      if (!myEnumProcessModules || !myGetModuleFileNameEx)
	return 0;
      return 1;
    }

  h = GetModuleHandle("KERNEL32.DLL");
  myCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress (h, "CreateToolhelp32Snapshot");
  myProcess32First = (PROCESSWALK)GetProcAddress (h, "Process32First");
  myProcess32Next  = (PROCESSWALK)GetProcAddress (h, "Process32Next");
  if (!myCreateToolhelp32Snapshot || !myProcess32First || !myProcess32Next)
    return 0;

  myEnumProcessModules = dummyprocessmodules;
  myGetModuleFileNameEx = GetModuleFileNameEx95;
  return 1;
}

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
  static char buf[5];
  if (ntty < 0)
    return "   ?";
  if (ntty == TTY_CONSOLE)
    return " con";
  sprintf (buf, "%4d", ntty);
  return buf;
}

static void
usage (FILE * stream, int status)
{
  fprintf (stream, "\
Usage: %s [-aefls] [-u UID] [-p PID]\n\
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
 -v, --version   output version information and exit\n\
 -W, --windows   show windows as well as cygwin processes\n\
With no options, %s outputs the long format by default\n",
	   prog_name, prog_name);
  exit (status);
}

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
%s (cygwin) %.*s\n\
Process Statistics\n\
Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.\n\
Compiled on %s\n\
", prog_name, len, v, __DATE__);
}

int
main (int argc, char *argv[])
{
  external_pinfo *p;
  int aflag, lflag, fflag, sflag, uid, proc_id;
  cygwin_getinfo_types query = CW_GETPINFO;
  const char *dtitle = "    PID TTY     STIME COMMAND\n";
  const char *dfmt   = "%7d%4s%10s %s\n";
  const char *ftitle = "     UID     PID    PPID TTY     STIME COMMAND\n";
  const char *ffmt   = "%8.8s%8d%8d%4s%10s %s\n";
  const char *ltitle = "      PID    PPID    PGID     WINPID  TTY  UID    STIME COMMAND\n";
  const char *lfmt   = "%c %7d %7d %7d %10u %4s %4u %8s %s\n";
  char ch;

  aflag = lflag = fflag = sflag = 0;
  uid = getuid ();
  proc_id = -1;
  lflag = 1;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];
  else
    prog_name++;

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
      case 'v':
	print_version ();
	exit (0);
	break;
      case 'W':
	query = CW_GETPINFO_FULL;
	aflag = 1;
	break;

      default:
	usage (stderr, 1);
      }

  if (sflag)
    printf (dtitle);
  else if (fflag)
    printf (ftitle);
  else if (lflag)
    printf (ltitle);

  (void) cygwin_internal (CW_LOCK_PINFO, 1000);

  if (query == CW_GETPINFO_FULL && !init_win ())
    query = CW_GETPINFO;

  for (int pid = 0;
       (p = (external_pinfo *) cygwin_internal (query, pid | CW_NEXTPID));
       pid = p->pid)
    {
      if ((proc_id > 0) && (p->pid != proc_id))
	continue;

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
      char pname[NT_MAX_PATH];
      if (p->process_state & PID_EXITED || (p->exitcode & ~0xffff))
	strcpy (pname, "<defunct>");
      else if (p->ppid)
	{
	  char *s;
	  pname[0] = '\0';
	  if (p->version >= EXTERNAL_PINFO_VERSION_32_LP)
	    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE,
			      p->progname_long, pname, NT_MAX_PATH);
	  else
	    cygwin_conv_path (CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE,
			      p->progname, pname, NT_MAX_PATH);
	  s = strchr (pname, '\0') - 4;
	  if (s > pname && strcasecmp (s, ".exe") == 0)
	    *s = '\0';
	}
      else if (query == CW_GETPINFO_FULL)
	{
	  HANDLE h = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				  FALSE, p->dwProcessId);
	  if (!h)
	    continue;
	  HMODULE hm[1000];
	  DWORD n = p->dwProcessId;
	  if (!myEnumProcessModules (h, hm, sizeof (hm), &n))
	    n = 0;
	  if (!n || !myGetModuleFileNameEx (h, hm[0], pname, PATH_MAX))
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
  (void) cygwin_internal (CW_UNLOCK_PINFO);

  return 0;
}

