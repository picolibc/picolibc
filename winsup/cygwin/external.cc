/* external.cc: Interface to Cygwin internals from external programs.

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
#include "pwdgrp.h"
#include "exception.h"
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
	  ep.ctty = (p->ctty < 0 || iscons_dev (p->ctty)) ? p->ctty : device::minor (p->ctty);
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
	  sys_wcstombs(ep.progname, MAX_PATH, p->progname);
	  ep.strace_mask = 0;
	  ep.version = EXTERNAL_PINFO_VERSION;

	  ep.process_state = p->process_state;

	  ep.uid32 = p->uid;
	  ep.gid32 = p->gid;

	  ep.progname_long = ep_progname_long_buf;
	  mount_table->conv_to_posix_path (p->progname, ep.progname_long, 0);
	  break;
	}
    }

  if (!ep.pid)
    {
      i = 0;
      pids.reset ();
      return NULL;
    }
  return &ep;
}

static inline uintptr_t
get_cygdrive_info (char *user, char *system, char *user_flags,
		   char *system_flags)
{
  int res = mount_table->get_cygdrive_info (user, system, user_flags,
					    system_flags);
  return (res == ERROR_SUCCESS) ? 1 : 0;
}

static bool
check_ntsec (const char *filename)
{
  if (!filename)
    return true;
  path_conv pc (filename);
  return pc.has_acls ();
}

/* Copy cygwin environment variables to the Windows environment. */
static PWCHAR
create_winenv (const char * const *env)
{
  int unused_envc;
  PWCHAR envblock = NULL;
  char **envp = build_env (env ?: cur_environ (), envblock, unused_envc, false,
			   NULL);
  PWCHAR p = envblock;

  if (envp)
    {
      for (char **e = envp; *e; e++)
	cfree (*e);
      cfree (envp);
    }
  /* If we got an env block, just return pointer to win env. */
  if (env)
    return envblock;
  /* Otherwise sync win env of current process with its posix env. */
  if (!p)
    return NULL;
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
  return NULL;
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
  external_pinfo *ep = fillout_pinfo (pid, 1);
  DWORD dwpid = ep ? ep->dwProcessId : pid;
  pinfo p (pid, PID_MAP_RW);
  if (ep)
    pid = ep->pid;
  if ((dwpid == GetCurrentProcessId()) && (p->pid == pid))
    p.set_exit_code ((DWORD)status);
  if (useTerminateProcess)
    TerminateProcess (GetCurrentProcess(), status);
  /* avoid 'else' clause to silence warning */
  ExitProcess (status);
}


extern "C" uintptr_t
cygwin_internal (cygwin_getinfo_types t, ...)
{
  va_list arg;
  uintptr_t res = (uintptr_t) -1;
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
	res = (uintptr_t) cygthread::name (va_arg (arg, DWORD));
	break;

      case CW_SETTHREADNAME:
	{
	  set_errno (ENOSYS);
	  res = 0;
	}
	break;

      case CW_GETPINFO:
	res = (uintptr_t) fillout_pinfo (va_arg (arg, DWORD), 0);
	break;

      case CW_GETVERSIONINFO:
	res = (uintptr_t) cygwin_version_strings;
	break;

      case CW_READ_V1_MOUNT_TABLES:
	set_errno (ENOSYS);
	res = 1;
	break;

      case CW_USER_DATA:
#ifndef __x86_64__
	/* This is a kludge to work around a version of _cygwin_common_crt0
	   which overwrote the cxx_malloc field with the local DLL copy.
	   Hilarity ensues if the DLL is not loaded like while the process
	   is forking. */
	__cygwin_user_data.cxx_malloc = &default_cygwin_cxx_malloc;
#endif
	res = (uintptr_t) &__cygwin_user_data;
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
	res = (uintptr_t) fillout_pinfo (va_arg (arg, pid_t), 1);
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
	      res = (uintptr_t) -1;
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
	  res = (uintptr_t) p->cmdline (n);
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
	  res = wincap.allocation_granularity ();
	}
	break;
      case CW_GET_UID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  res = psid.get_uid (NULL);
	}
	break;
      case CW_GET_GID_FROM_SID:
	{
	  cygpsid psid = va_arg (arg, PSID);
	  res = psid.get_gid (NULL);
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
	  res = (uintptr_t) hook_or_detect_cygwin (name, hookfn, subsys);
	}
	break;
      case CW_ARGV:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  res = (uintptr_t) (ci ? ci->moreinfo->argv : NULL);
	}
	break;
      case CW_ENVP:
	{
	  child_info_spawn *ci = (child_info_spawn *) get_cygwin_startup_info ();
	  res = (uintptr_t) (ci ? ci->moreinfo->envp : NULL);
	}
	break;
      case CW_DEBUG_SELF:
	error_start_init (va_arg (arg, const char *));
	res = try_to_debug ();
	break;
      case CW_SYNC_WINENV:
	create_winenv (NULL);
	res = 0;
	break;
      case CW_CYGTLS_PADSIZE:
	res = CYGTLS_PADSIZE;
	break;
      case CW_SET_DOS_FILE_WARNING:
	{
	  dos_file_warning = va_arg (arg, int);
	  res = 0;
	}
	break;
      case CW_SET_PRIV_KEY:
	{
	  const char *passwd = va_arg (arg, const char *);
	  const char *username = va_arg (arg, const char *);
	  res = setlsapwd (passwd, username);
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
	  PWCHAR dest = va_arg (arg, PWCHAR);
	  wcscpy (dest, cygheap->installation_key_buf);
	  res = 0;
	}
	break;
      case CW_INT_SETLOCALE:
	{
	  extern void internal_setlocale ();
	  internal_setlocale ();
	  res = 0;
	}
	break;
      case CW_CVT_MNT_OPTS:
	{
	  extern bool fstab_read_flags (char **, unsigned &, bool);
	  char **option_string = va_arg (arg, char **);
	  if (!option_string || !*option_string)
	    set_errno (EINVAL);
	  else
	    {
	      unsigned *pflags = va_arg (arg, unsigned *);
	      unsigned flags = pflags ? *pflags : 0;
	      if (fstab_read_flags (option_string, flags, true))
		{
		  if (pflags)
		    *pflags = flags;
		  res = 0;
		}
	    }
	}
	break;
      case CW_LST_MNT_OPTS:
	{
	  extern char *fstab_list_flags ();
	  char **option_string = va_arg (arg, char **);
	  if (!option_string)
	    set_errno (EINVAL);
	  else
	    {
	      *option_string = fstab_list_flags ();
	      if (*option_string)
		res = 0;
	    }
	}
	break;
      case CW_STRERROR:
	{
	  int err = va_arg (arg, int);
	  res = (uintptr_t) strerror (err);
	}
	break;

      case CW_CVT_ENV_TO_WINENV:
	{
	  char **posix_env = va_arg (arg, char **);
	  res = (uintptr_t) create_winenv (posix_env);
	}
	break;

      case CW_ALLOC_DRIVE_MAP:
      	{
	  dos_drive_mappings *ddm = new dos_drive_mappings ();
	  res = (uintptr_t) ddm;
	}
	break;

      case CW_MAP_DRIVE_MAP:
	{
	  dos_drive_mappings *ddm = va_arg (arg, dos_drive_mappings *);
	  wchar_t *pathbuf = va_arg (arg, wchar_t *);
	  if (ddm && pathbuf)
	    res = (uintptr_t) ddm->fixup_if_match (pathbuf);
	}
	break;

      case CW_FREE_DRIVE_MAP:
	{
	  dos_drive_mappings *ddm = va_arg (arg, dos_drive_mappings *);
	  if (ddm)
	    delete ddm;
	  res = 0;
	}
	break;

      case CW_SETENT:
	{
	  int group = va_arg (arg, int);
	  int enums = va_arg (arg, int);
	  PCWSTR enum_tdoms = va_arg (arg, PCWSTR);
	  if (group)
	    res = (uintptr_t) setgrent_filtered (enums, enum_tdoms);
	  else
	    res = (uintptr_t) setpwent_filtered (enums, enum_tdoms);
	}
	break;

      case CW_GETENT:
	{
	  int group = va_arg (arg, int);
	  void *obj = va_arg (arg, void *);
	  if (obj)
	    {
	      if (group)
		res = (uintptr_t) getgrent_filtered (obj);
	      else
		res = (uintptr_t) getpwent_filtered (obj);
	    }
	}
	break;

      case CW_ENDENT:
	{
	  int group = va_arg (arg, int);
	  void *obj = va_arg (arg, void *);
	  if (obj)
	    {
	      if (group)
		endgrent_filtered (obj);
	      else
		endpwent_filtered (obj);
	      res = 0;
	    }
	}
	break;

      case CW_GETNSSSEP:
	res = (uintptr_t) cygheap->pg.nss_separator ();
	break;

      case CW_GETNSS_PWD_SRC:
	res = (uintptr_t) cygheap->pg.nss_pwd_src ();
	break;

      case CW_GETNSS_GRP_SRC:
	res = (uintptr_t) cygheap->pg.nss_grp_src ();
	break;

      case CW_GETPWSID:
	{
	  int db_only = va_arg (arg, int);
	  PSID psid = va_arg (arg, PSID);
	  cygpsid sid (psid);
	  res = (uintptr_t) (db_only ? internal_getpwsid_from_db (sid)
				     : internal_getpwsid (sid));
	}
	break;

      case CW_GETGRSID:
	{
	  int db_only = va_arg (arg, int);
	  PSID psid = va_arg (arg, PSID);
	  cygpsid sid (psid);
	  res = (uintptr_t) (db_only ? internal_getgrsid_from_db (sid)
				     : internal_getgrsid (sid));
	}
	break;

      case CW_CYGNAME_FROM_WINNAME:
	{
	  /* This functionality has been added mainly for sshd.  Sshd
	     calls getpwnam() with the username of the non-privileged
	     user used for privilege separation.  This is usually a
	     fixed string "sshd".  However, when using usernames from
	     the Windows DBs, it's no safe bet anymore if the username
	     is "sshd", it could also be "DOMAIN+sshd".  So what we do
	     here is this:

	     Sshd calls cygwin_internal (CW_CYGNAME_FROM_WINNAME,
					 "sshd",
					 username_buffer,
					 sizeof username_buffer);
	     
	     If this call succeeds, sshd expects the correct Cygwin
	     username of the unprivileged sshd account in username_buffer.

	     The below code checks for a Windows username matching the
	     incoming username, and then fetches the Cygwin username with
	     the matching SID.  This is our username used for privsep then.

	     Of course, other applications with similar needs can use the
	     same method. */
	  const char *winname = va_arg (arg, const char *);
	  char *buffer = va_arg (arg, char *);
	  size_t buflen = va_arg (arg, size_t);

	  if (!winname || !buffer || !buflen)
	    break;

	  WCHAR name[UNLEN + 1];
	  sys_mbstowcs (name, sizeof name, winname);

	  cygsid sid;
	  DWORD slen = SECURITY_MAX_SID_SIZE;
	  WCHAR dom[DNLEN + 1];
	  DWORD dlen = DNLEN + 1;
	  SID_NAME_USE acc_type;

	  if (!LookupAccountNameW (NULL, name, sid, &slen, dom, &dlen,
				   &acc_type))
	    break;

	  struct passwd *pw = internal_getpwsid (sid);
	  if (!pw)
	    break;

	  buffer[0] = '\0';
	  strncat (buffer, pw->pw_name, buflen - 1);
	  res = 0;
	}
	break;

      case CW_FIXED_ATEXIT:
	res = 0;
	break;

      case CW_EXCEPTION_RECORD_FROM_SIGINFO_T:
	{
	  siginfo_t *si = va_arg(arg, siginfo_t *);
	  EXCEPTION_RECORD *er = va_arg(arg, EXCEPTION_RECORD *);
	  if (si && si->si_cyg && er)
	    {
	      memcpy(er, ((cygwin_exception *)si->si_cyg)->exception_record(),
		     sizeof(EXCEPTION_RECORD));
	      res = 0;
	    }
	}
	break;

      case CW_CYGHEAP_PROFTHR_ALL:
	{
	  typedef void (*func_t) (HANDLE);
	  extern void cygheap_profthr_all (func_t);

	  func_t profthr_byhandle = va_arg(arg, func_t);
	  cygheap_profthr_all (profthr_byhandle);
	  res = 0;
	}
	break;

      default:
	set_errno (ENOSYS);
    }
  va_end (arg);
  return res;
}
