/* ps.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

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
#include <sys/cygwin.h>
#include <tlhelp32.h>
#include <psapi.h>

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

int
main (int argc, char *argv[])
{
  external_pinfo *p;
  int aflag, lflag, fflag, sflag, uid;
  cygwin_getinfo_types query = CW_GETPINFO;
  const char *dtitle = "  PID TTY     STIME COMMAND\n";
  const char *dfmt   = "%5d%4d%10s %s\n";
  const char *ftitle = "     UID   PID  PPID TTY     STIME COMMAND\n";
  const char *ffmt   = "%8.8s%6d%6d%4d%10s %s\n";
  const char *ltitle = "    PID  PPID  PGID   WINPID TTY  UID    STIME COMMAND\n";
  const char *lfmt   = "%c %5d %5d %5d %8u %3d %4d %8s %s\n";
  char ch;

  aflag = lflag = fflag = sflag = 0;
  uid = getuid ();
  lflag = 1;

  while ((ch = getopt (argc, argv, "aelfsu:W")) != -1)
    switch (ch)
      {
      case 'a':
      case 'e':
        aflag = 1;
        break;
      case 'f':
        fflag = 1;
        break;
      case 'l':
        lflag = 1;
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
                fprintf (stderr, "user %s unknown\n", optarg);
                exit (1);
              }
          }
        break;
      case 'W':
	query = CW_GETPINFO_FULL;
	aflag = 1;
	break;

      default:
        fprintf (stderr, "Usage %s [-aefl] [-u uid]\n", argv[0]);
        fprintf (stderr, "-f = show process uids, ppids\n");
        fprintf (stderr, "-l = show process uids, ppids, pgids, winpids\n");
        fprintf (stderr, "-u uid = list processes owned by uid\n");
        fprintf (stderr, "-a, -e = show processes of all users\n");
	fprintf (stderr, "-s = show process summary\n");
	fprintf (stderr, "-W = show windows as well as cygwin processes\n");
        exit (1);
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
      if (p->process_state == PID_NOT_IN_USE)
        continue;
      if (!aflag && p->uid != uid)
        continue;
      char status = ' ';
      if (p->process_state & PID_STOPPED)
        status = 'S';
      else if (p->process_state & PID_TTYIN)
        status = 'I';
      else if (p->process_state & PID_TTYOU)
        status = 'O';

      char pname[MAX_PATH];
      if (p->process_state & PID_ZOMBIE)
        strcpy (pname, "<defunct>");
      else if (p->ppid)
	{
	  char *s;
	  pname[0] = '\0';
	  cygwin_conv_to_posix_path (p->progname, pname);
	  s = strchr (pname, '\0') - 4;
	  if (s > pname && strcasecmp (s, ".exe") == 0)
	    *s = '\0';
	}
      else if (query == CW_GETPINFO_FULL)
	{
	  HANDLE h = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, p->dwProcessId);
	  if (!h)
	    continue;
	  HMODULE hm[1000];
	  DWORD n = p->dwProcessId;
	  if (!myEnumProcessModules (h, hm, sizeof (hm), &n))
	    n = 0;
	  if (!n || !myGetModuleFileNameEx (h, hm[0], pname, MAX_PATH))
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

          if ((pw = getpwuid (p->uid)))
            strcpy (uname, pw->pw_name);
          else
            sprintf (uname, "%d", p->uid);
        }

      if (sflag)
        printf (dfmt, p->pid, p->ctty, start_time (p), pname);
      else if (fflag)
        printf (ffmt, uname, p->pid, p->ppid, p->ctty, start_time (p), pname);
      else if (lflag)
        printf (lfmt, status, p->pid, p->ppid, p->pgid,
                p->dwProcessId, p->ctty, p->uid, start_time (p), pname);

    }
  (void) cygwin_internal (CW_UNLOCK_PINFO);

  return 0;
}

