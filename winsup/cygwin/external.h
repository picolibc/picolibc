/* external.h: interface to Cygwin internals from external programs.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

typedef enum
  {
    CW_LOCK_PINFO,
    CW_UNLOCK_PINFO,
    CW_GETTHREADNAME,
    CW_GETPINFO,
    CW_SETPINFO,
    CW_SETTHREADNAME,
    CW_GETVERSIONINFO,
    CW_READ_V1_MOUNT_TABLES
  } cygwin_getinfo_types;

struct external_pinfo
  {
  pid_t pid;
  pid_t ppid;
  HANDLE hProcess;
  DWORD dwProcessId, dwSpawnedProcessId;
  uid_t uid;
  gid_t gid;
  pid_t pgid;
  pid_t sid;
  int ctty;
  mode_t umask;

  long start_time;
  struct rusage rusage_self;
  struct rusage rusage_children;

  char progname[MAX_PATH];

  DWORD strace_mask;
  HANDLE strace_file;

  DWORD process_state;
};

extern "C" DWORD cygwin_internal (cygwin_getinfo_types, ...);

#define CW_NEXTPID 0x80000000	// or with pid to get next one
