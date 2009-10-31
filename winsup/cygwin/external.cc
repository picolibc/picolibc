/* external.cc: Interface to Cygwin internals from external programs.

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009 Red Hat, Inc.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "sigproc.h"
#include "pinfo.h"
#include "shared_info.h"
#include "cygwin_version.h"
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "heap.h"
#include "cygtls.h"
#include "child_info.h"
#include "environ.h"
#include "cygserver_setpwd.h"
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <iptypes.h>

child_info *get_cygwin_startup_info ();
static void exit_process (UINT, bool) __attribute__((noreturn));

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
    return true;
  path_conv pc (filename);
  return pc.has_acls ();
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

/*
 * Cygwin-specific wrapper for win32 ExitProcess and TerminateProcess.
 * It ensures that the correct exit code, derived from the specified
 * status value, will be made available to this process's parent (if
 * that parent is also a cygwin process). If useTerminateProcess is
 * true, then TerminateProcess(GetCurrentProcess(),...) will be used;
 * otherwise, ExitProcess(...) is called.
 *
 * Used by startup code for cygwin processes which is linked statically
 * into applications, and is not part of the cygwin DLL -- which is why
 * this interface is exposed. "Normal" programs should use ANSI exit(),
 * ANSI abort(), or POSIX _exit(), rather than this function -- because
 * calling ExitProcess or TerminateProcess, even through this wrapper,
 * skips much of the cygwin process cleanup code.
 */
static void
exit_process (UINT status, bool useTerminateProcess)
{
  pid_t pid = getpid ();
  external_pinfo * ep = fillout_pinfo (pid, 1);
  DWORD dwpid = ep ? ep->dwProcessId : pid;
  pinfo p (pid, PID_MAP_RW);
  if ((dwpid == GetCurrentProcessId()) && (p->pid == ep->pid))
    p.set_exit_code ((DWORD)status);
  if (useTerminateProcess)
    TerminateProcess (GetCurrentProcess(), status);
  /* avoid 'else' clause to silence warning */
  ExitProcess (status);
}


extern "C" unsigned long
cygwin_internal (cygwin_getinfo_types t, ...)
{
  va_list arg;
  unsigned long res = -1;
  va_start (arg, t);

  switch (t)
    {
      case CW_LOCK_PINFO:
	res = 1;
	break;

      case CW_UNLOCK_PINFO:
	res = 1;
	break;

      case CW_GETTHREADNAME:
	res = (DWORD) cygthread::name (va_arg (arg, DWORD));
	break;

      case CW_SETTHREADNAME:
	{
	  set_errno (ENOSYS);
	  res = 0;
	}
	break;

      case CW_GETPINFO:
	res = (DWORD) fillout_pinfo (va_arg (arg, DWORD), 0);
	break;

      case CW_GETVERSIONINFO:
	res = (DWORD) cygwin_version_strings;
	break;

      case CW_READ_V1_MOUNT_TABLES:
	set_errno (ENOSYS);
	res = 1;
	break;

      case CW_USER_DATA:
	/* This is a kludge to work around a version of _cygwin_common_crt0
	   which overwrote the cxx_malloc field with the local DLL copy.
	   Hilarity ensues if the DLL is not loaded like while the process
	   is forking. */
	__cygwin_user_data.cxx_malloc = &default_cygwin_cxx_malloc;
	res = (DWORD) &__cygwin_user_data;
	break;

      case CW_PERFILE:
	perfile_table = va_arg (arg, struct __cygwin_perfile *);
	res = 0;
	break;

      case CW_GET_CYGDRIVE_PREFIXES:
	{
	  char *user = va_arg (arg, char *);
	  char *system = va_arg (arg, char *);
	  res = get_cygdrive_info (user, system, NULL, NULL);
	}
	break;

      case CW_GETPINFO_FULL:
	res = (DWORD) fillout_pinfo (va_arg (arg, pid_t), 1);
	break;

      case CW_INIT_EXCEPTIONS:
	/* noop */ /* init_exceptions (va_arg (arg, exception_list *)); */
	res = 0;
	break;

      case CW_GET_CYGDRIVE_INFO:
	{
	  char *user = va_arg (arg, char *);
	  char *system = va_arg (arg, char *);
	  char *user_flags = va_arg (arg, char *);
	  char *system_flags = va_arg (arg, char *);
	  res = get_cygdrive_info (user, system, user_flags, system_flags);
	}
	break;

      case CW_SET_CYGWIN_REGISTRY_NAME:
      case CW_GET_CYGWIN_REGISTRY_NAME:
	res = 0;
	break;

      case CW_STRACE_TOGGLE:
	{
	  pid_t pid = va_arg (arg, pid_t);
	  pinfo p (pid);
	  if (p)
	    {
	      sig_send (p, __SIGSTRACE);
	      res = 0;
	    }
	  else
	    {
	      set_errno (ESRCH);
	      res = (DWORD) -1;
	    }
	}
	break;

      case CW_STRACE_ACTIVE:
	{
	  res = strace.active ();
	}
	break;

      case CW_CYGWIN_PID_TO_WINPID:
	{
	  pinfo p (va_arg (arg, pid_t));
	  res = p ? p->dwProcessId : 0;
	}
	break;
      case CW_EXTRACT_DOMAIN_AND_USER:
	{
	  WCHAR nt_domain[MAX_DOMAIN_NAME_LEN + 1];
	  WCHAR nt_user[UNLEN + 1];

	  struct passwd *pw = va_arg (arg, struct passwd *);
	  char *domain = va_arg (arg, char *);
	  char *user = va_arg (arg, char *);
	  extract_nt_dom_user (pw, nt_domain, nt_user);
	  if (domain)
	    sys_wcstombs (domain, MAX_DOMAIN_NAME_LEN + 1, nt_domain);
	  if (user)
	    sys_wcstombs (user, UNLEN + 1, nt_user);
	  res = 0;
	}
	break;
      case CW_CMDLINE:
	{
	  size_t n;
	  pid_t pid = va_arg (arg, pid_t);
	  pinfo p (pid);
	  res = (DWORD) p->cmdline (n);
	}
	break;
      case CW_CHECK_NTSEC:
	{
	  char *filename = va_arg (arg, char *);
	  res = check_ntsec (filename);
	}
	break;
      case CW_GET_ERRNO_FROM_WINERROR:
	{
	  int error = va_arg (arg, int);
	  int deferrno = va_arg (arg, int);
	  res = geterrno_from_win_error (error, deferrno);
	}
	break;
      case CW_GET_POSIX_SECURITY_ATTRIBUTE:
	{
	  path_conv dummy;
	  security_descriptor sd;
	  int attribute = va_arg (arg, int);
	  PSECURITY_ATTRIBUTES psa = va_arg (arg, PSECURITY_ATTRIBUTES);
	  void *sd_buf = va_arg (arg, void *);
	  DWORD sd_buf_size = va_arg (arg, DWORD);
	  set_security_attribute (dummy, attribute, psa, sd);
	  if (!psa->lpSecurityDescriptor)
	    res = sd.size ();
	  else
	    {
	      psa->lpSecurityDescriptor = sd_buf;
	      res = sd.copy (sd_buf, sd_buf_size);
	    }
	}
	break;
      case CW_GET_SHMLBA:
	{
	  res = getpagesize ();
	}
	break;
      case CW_GET_UID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  res = psid.get_id (false, NULL);
	}
	break;
      case CW_GET_GID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  res = psid.get_id (true, NULL);
	}
	break;
      case CW_GET_BINMODE:
	{
	  const char *path = va_arg (arg, const char *);
	  path_conv p (path, PC_SYM_FOLLOW | PC_NULLEMPTY);
	  if (p.error)
	    {
	      set_errno (p.error);
	      res = (unsigned long) -1;
	    }
	  else
	    res = p.binmode ();
	}
	break;
      case CW_HOOK:
	{
	  const char *name = va_arg (arg, const char *);
	  const void *hookfn = va_arg (arg, const void *);
	  WORD subsys;
	  res = (unsigned long) hook_or_detect_cygwin (name, hookfn, subsys);
	}
	break;
      case CW_ARGV:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  res = (unsigned long) (ci ? ci->moreinfo->argv : NULL);
	}
	break;
      case CW_ENVP:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  res = (unsigned long) (ci ? ci->moreinfo->envp : NULL);
	}
	break;
      case CW_DEBUG_SELF:
	error_start_init (va_arg (arg, const char *));
	try_to_debug ();
	break;
      case CW_SYNC_WINENV:
	sync_winenv ();
	res = 0;
	break;
      case CW_CYGTLS_PADSIZE:
	res = CYGTLS_PADSIZE;
	break;
      case CW_SET_DOS_FILE_WARNING:
	{
	  extern bool dos_file_warning;
	  dos_file_warning = va_arg (arg, int);
	  res = 0;
	}
	break;
      case CW_SET_PRIV_KEY:
	{
	  const char *passwd = va_arg (arg, const char *);
	  res = setlsapwd (passwd);
	}
	break;
      case CW_SETERRNO:
	{
	  const char *file = va_arg (arg, const char *);
	  int line = va_arg (arg, int);
	  seterrno(file, line);
	  res = 0;
	}
	break;
      case CW_EXIT_PROCESS:
	{
	  UINT status = va_arg (arg, UINT);
	  int useTerminateProcess = va_arg (arg, int);
	  exit_process (status, !!useTerminateProcess); /* no return */
	}
      case CW_SET_EXTERNAL_TOKEN:
	{
	  HANDLE token = va_arg (arg, HANDLE);
	  int type = va_arg (arg, int);
	  set_imp_token (token, type);
	  res = 0;
	}
	break;
      case CW_GET_INSTKEY:
	{
	  extern WCHAR installation_key_buf[18];
	  PWCHAR dest = va_arg (arg, PWCHAR);
	  wcscpy (dest, installation_key_buf);
	  res = 0;
	}
	break;

      default:
	set_errno (ENOSYS);
    }
  va_end (arg);
  return res;
}
