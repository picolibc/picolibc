/* external.cc: Interface to Cygwin internals from external programs.

   Copyright 1997, 1998, 1999, 2000 Cygnus Solutions.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

static external_pinfo *
fillout_pinfo (DWORD pid)
{
  BOOL nextpid;
  pinfo *p = NULL;
  int i;
  static external_pinfo ep;

  if ((nextpid = !!(pid & CW_NEXTPID)))
    pid ^= CW_NEXTPID;
  for (i = 0; i < cygwin_shared->p.size(); i++, p = NULL)
    {
      p = cygwin_shared->p.vec + i;
      if (!pid || (DWORD) p->pid == pid)
	{
	  if (nextpid && pid)
	    {
	      pid = 0;
	      nextpid = 0;
	    }
	  else if (p->pid && NOTSTATE(p, PID_CLEAR))
	    break;
	}
    }

  if (p == NULL)
    return 0;

  memset (&ep, 0, sizeof ep);
  ep.ctty = tty_attached (p) ? p->ctty : -1;
  ep.pid = p->pid;
  ep.ppid = p->ppid;
  ep.hProcess = p->hProcess;
  ep.dwProcessId = p->dwProcessId;
//ep.dwSpawnedProcessId = p->dwSpawnedProcessId;
  ep.uid = p->uid;
  ep.gid = p->gid;
  ep.pgid = p->pgid;
  ep.sid = p->sid;
  ep.umask = p->umask;
  ep.start_time = p->start_time;
  ep.rusage_self = p->rusage_self;
  ep.rusage_children = p->rusage_children;
  strcpy (ep.progname, p->progname);
  ep.strace_mask = 0;
  ep.strace_file = 0;

  ep.process_state = p->process_state;
  return &ep;
}

static DWORD
get_cygdrive_prefixes (char *user, char *system)
{
  shared_info *info = cygwin_getshared();
  int res = info->mount.get_cygdrive_prefixes(user, system);
  return (res == ERROR_SUCCESS) ? 1 : 0;
}

extern "C" DWORD
cygwin_internal (cygwin_getinfo_types t, ...)
{
  va_list arg;
  va_start (arg, t);

  switch (t)
    {
      case CW_LOCK_PINFO:
	return lock_pinfo_for_update (va_arg (arg, DWORD));
	break;

      case CW_UNLOCK_PINFO:
	unlock_pinfo ();
	return 1;

      case CW_GETTHREADNAME:
	return (DWORD) threadname (va_arg (arg, DWORD));

      case CW_SETTHREADNAME:
	{
	  char *name = va_arg (arg, char *);
	  regthread (name, va_arg (arg, DWORD));
	  return 1;
	}

      case CW_GETPINFO:
	return (DWORD) fillout_pinfo (va_arg (arg, DWORD));

      case CW_GETVERSIONINFO:
	return (DWORD) cygwin_version_strings;

      case CW_READ_V1_MOUNT_TABLES:
	/* Upgrade old v1 registry mounts to new location. */
	cygwin_shared->mount.import_v1_mounts ();
	return 0;

      case CW_USER_DATA:
	return (DWORD) &__cygwin_user_data;

      case CW_PERFILE:
	perfile_table = va_arg (arg, struct __cygwin_perfile *);
	return 0;

      case CW_GET_CYGDRIVE_PREFIXES:
	{
	  char *user = va_arg (arg, char *);
	  char *system = va_arg (arg, char *);
	  return get_cygdrive_prefixes (user, system);
	}

      default:
	return (DWORD) -1;
    }
}
