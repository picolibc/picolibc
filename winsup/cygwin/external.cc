/* external.cc: Interface to Cygwin internals from external programs.

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008 Red Hat, Inc.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "security.h"
#include "sigproc.h"
#include "pinfo.h"
#include <exceptions.h>
#include "shared_info.h"
#include "cygwin_version.h"
#include "perprocess.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "wincap.h"
#include "heap.h"
#include "pwdgrp.h"
#include "cygtls.h"
#include "child_info.h"
#include "environ.h"
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>

child_info *get_cygwin_startup_info ();

static winpids pids;

static external_pinfo *
fillout_pinfo (pid_t pid, int winpid)
{
  BOOL nextpid;
  static external_pinfo ep;
  static char ep_progname_long_buf[NT_MAX_PATH];

  if ((nextpid = !!(pid & CW_NEXTPID)))
    pid ^= CW_NEXTPID;


  static unsigned int i;
  if (!pids.npids || !nextpid)
    {
      pids.set (winpid);
      i = 0;
    }

  if (!pid)
    i = 0;

  memset (&ep, 0, sizeof ep);
  while (i < pids.npids)
    {
      DWORD thispid = pids.winpid (i);
      _pinfo *p = pids[i];
      i++;

      if (!p)
	{
	  if (!nextpid && thispid != (DWORD) pid)
	    continue;
	  ep.pid = cygwin_pid (thispid);
	  ep.dwProcessId = thispid;
	  ep.process_state = PID_IN_USE;
	  ep.ctty = -1;
	  break;
	}
      else if (nextpid || p->pid == pid || (winpid && thispid == (DWORD) pid))
	{
	  ep.ctty = p->ctty;
	  ep.pid = p->pid;
	  ep.ppid = p->ppid;
	  ep.dwProcessId = p->dwProcessId;
	  ep.uid = p->uid;
	  ep.gid = p->gid;
	  ep.pgid = p->pgid;
	  ep.sid = p->sid;
	  ep.umask = 0;
	  ep.start_time = p->start_time;
	  ep.rusage_self = p->rusage_self;
	  ep.rusage_children = p->rusage_children;
	  ep.progname[0] = '\0';
	  strncat (ep.progname, p->progname, MAX_PATH - 1);
	  ep.strace_mask = 0;
	  ep.version = EXTERNAL_PINFO_VERSION;

	  ep.process_state = p->process_state;

	  ep.uid32 = p->uid;
	  ep.gid32 = p->gid;

	  ep.progname_long = ep_progname_long_buf;
	  strcpy (ep.progname_long, p->progname);
	  break;
	}
    }

  if (!ep.pid)
    {
      i = 0;
      pids.reset ();
      return 0;
    }
  return &ep;
}

static inline DWORD
get_cygdrive_info (char *user, char *system, char *user_flags,
		   char *system_flags)
{
  int res = mount_table->get_cygdrive_info (user, system, user_flags,
					    system_flags);
  return (res == ERROR_SUCCESS) ? 1 : 0;
}

static DWORD
check_ntsec (const char *filename)
{
  if (!filename)
    return allow_ntsec;
  path_conv pc (filename);
  return allow_ntsec && pc.has_acls ();
}

/* Copy cygwin environment variables to the Windows environment. */
static void
sync_winenv ()
{
  int unused_envc;
  PWCHAR envblock = NULL;
  char **envp = build_env (cur_environ (), envblock, unused_envc, false);
  PWCHAR p = envblock;

  if (envp)
    {
      for (char **e = envp; *e; e++)
	cfree (*e);
      cfree (envp);
    }
  if (!p)
    return;
  while (*p)
    {
      PWCHAR eq = wcschr (p, L'=');
      if (eq)
	{
	  *eq = L'\0';
	  SetEnvironmentVariableW (p, ++eq);
	  p = eq;
	}
      p = wcschr (p, L'\0') + 1;
    }
  free (envblock);
}

extern "C" unsigned long
cygwin_internal (cygwin_getinfo_types t, ...)
{
  va_list arg;
  va_start (arg, t);

  switch (t)
    {
      case CW_LOCK_PINFO:
	return 1;

      case CW_UNLOCK_PINFO:
	return 1;

      case CW_GETTHREADNAME:
	return (DWORD) cygthread::name (va_arg (arg, DWORD));

      case CW_SETTHREADNAME:
	{
	  set_errno (ENOSYS);
	  return 0;
	}

      case CW_GETPINFO:
	return (DWORD) fillout_pinfo (va_arg (arg, DWORD), 0);

      case CW_GETVERSIONINFO:
	return (DWORD) cygwin_version_strings;

      case CW_READ_V1_MOUNT_TABLES:
	set_errno (ENOSYS);
	return 1;

      case CW_USER_DATA:
	return (DWORD) &__cygwin_user_data;

      case CW_PERFILE:
	perfile_table = va_arg (arg, struct __cygwin_perfile *);
	return 0;

      case CW_GET_CYGDRIVE_PREFIXES:
	{
	  char *user = va_arg (arg, char *);
	  char *system = va_arg (arg, char *);
	  return get_cygdrive_info (user, system, NULL, NULL);
	}

      case CW_GETPINFO_FULL:
	return (DWORD) fillout_pinfo (va_arg (arg, pid_t), 1);

      case CW_INIT_EXCEPTIONS:
	/* noop */ /* init_exceptions (va_arg (arg, exception_list *)); */
	return 0;

      case CW_GET_CYGDRIVE_INFO:
	{
	  char *user = va_arg (arg, char *);
	  char *system = va_arg (arg, char *);
	  char *user_flags = va_arg (arg, char *);
	  char *system_flags = va_arg (arg, char *);
	  return get_cygdrive_info (user, system, user_flags, system_flags);
	}

      case CW_SET_CYGWIN_REGISTRY_NAME:
      case CW_GET_CYGWIN_REGISTRY_NAME:
	return 0;

      case CW_STRACE_TOGGLE:
	{
	  pid_t pid = va_arg (arg, pid_t);
	  pinfo p (pid);
	  if (p)
	    {
	      sig_send (p, __SIGSTRACE);
	      return 0;
	    }
	  else
	    {
	      set_errno (ESRCH);
	      return (DWORD) -1;
	    }
	}

      case CW_STRACE_ACTIVE:
	{
	  return strace.active ();
	}

      case CW_CYGWIN_PID_TO_WINPID:
	{
	  pinfo p (va_arg (arg, pid_t));
	  return p ? p->dwProcessId : 0;
	}
      case CW_EXTRACT_DOMAIN_AND_USER:
	{
	  struct passwd *pw = va_arg (arg, struct passwd *);
	  char *domain = va_arg (arg, char *);
	  char *user = va_arg (arg, char *);
	  extract_nt_dom_user (pw, domain, user);
	  return 0;
	}
      case CW_CMDLINE:
	{
	  size_t n;
	  pid_t pid = va_arg (arg, pid_t);
	  pinfo p (pid);
	  return (DWORD) p->cmdline (n);
	}
      case CW_CHECK_NTSEC:
	{
	  char *filename = va_arg (arg, char *);
	  return check_ntsec (filename);
	}
      case CW_GET_ERRNO_FROM_WINERROR:
	{
	  int error = va_arg (arg, int);
	  int deferrno = va_arg (arg, int);
	  return geterrno_from_win_error (error, deferrno);
	}
      case CW_GET_POSIX_SECURITY_ATTRIBUTE:
	{
	  security_descriptor sd;
	  int attribute = va_arg (arg, int);
	  PSECURITY_ATTRIBUTES psa = va_arg (arg, PSECURITY_ATTRIBUTES);
	  void *sd_buf = va_arg (arg, void *);
	  DWORD sd_buf_size = va_arg (arg, DWORD);
	  set_security_attribute (attribute, psa, sd);
	  if (!psa->lpSecurityDescriptor)
	    return sd.size ();
	  psa->lpSecurityDescriptor = sd_buf;
	  return sd.copy (sd_buf, sd_buf_size);
	}
      case CW_GET_SHMLBA:
	{
	  return getpagesize ();
	}
      case CW_GET_UID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  return psid.get_id (false, NULL);
	}
      case CW_GET_GID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  return psid.get_id (true, NULL);
	}
      case CW_GET_BINMODE:
	{
	  const char *path = va_arg (arg, const char *);
	  path_conv p (path, PC_SYM_FOLLOW | PC_NULLEMPTY);
	  if (p.error)
	    {
	      set_errno (p.error);
	      return (unsigned long) -1;
	    }
	  return p.binmode ();
	}
      case CW_HOOK:
	{
	  const char *name = va_arg (arg, const char *);
	  const void *hookfn = va_arg (arg, const void *);
	  WORD subsys;
	  return (unsigned long) hook_or_detect_cygwin (name, hookfn, subsys);
	}
      case CW_ARGV:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  return (unsigned long) (ci ? ci->moreinfo->argv : NULL);
	}
      case CW_ENVP:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  return (unsigned long) (ci ? ci->moreinfo->envp : NULL);
	}
      case CW_DEBUG_SELF:
	error_start_init (va_arg (arg, const char *));
	try_to_debug ();
	break;
      case CW_SYNC_WINENV:
	sync_winenv ();
	return 0;
      case CW_CYGTLS_PADSIZE:
	return CYGTLS_PADSIZE;
      case CW_SET_DOS_FILE_WARNING:
	{
	  extern bool dos_file_warning;
	  dos_file_warning = va_arg (arg, int);
	}
	break;

      default:
	break;
    }
  set_errno (ENOSYS);
  return (unsigned long) -1;
}
