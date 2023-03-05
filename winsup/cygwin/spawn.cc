/* spawn.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <unistd.h>
#include <process.h>
#include <sys/wait.h>
#include <wchar.h>
#include <ctype.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "sigproc.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include "environ.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "winf.h"
#include "ntdll.h"
#include "shared_info.h"

static const suffix_info exe_suffixes[] =
{
  suffix_info ("", 1),
  suffix_info (".exe", 1),
  suffix_info (NULL)
};

/* Add .exe to PROG if not already present and see if that exists.
   If not, return PROG (converted from posix to win32 rules if necessary).
   The result is always BUF.

   Returns (possibly NULL) suffix */

static const char *
perhaps_suffix (const char *prog, path_conv& buf, int& err, unsigned opt)
{
  const char *ext;

  err = 0;
  debug_printf ("prog '%s'", prog);
  buf.check (prog, PC_SYM_FOLLOW | PC_NULLEMPTY | PC_POSIX,
	     (opt & FE_DLL) ? stat_suffixes : exe_suffixes);

  if (buf.isdir ())
    {
      err = EACCES;
      ext = NULL;
    }
  else if (!buf.exists ())
    {
      err = ENOENT;
      ext = NULL;
    }
  else if (buf.known_suffix ())
    ext = buf.get_win32 () + (buf.known_suffix () - buf.get_win32 ());
  else
    ext = strchr (buf.get_win32 (), '\0');

  debug_printf ("buf %s, suffix found '%s'", (char *) buf.get_win32 (), ext);
  return ext;
}

/* Find an executable name, possibly by appending known executable suffixes
   to it.  The path_conv struct 'buf' is filled and contains both, win32 and
   posix path of the target file.  Any found suffix is returned in known_suffix.
   Eventually the posix path in buf is overwritten with the exact path as it
   gets constructed for the path search.  The reason is that the path is used
   to create argv[0] in av::setup, and this requires that the filename stays
   intact, instead of being resolved if the file is a symlink.

   If the file is not found and !FE_NNF then the POSIX version of name is
   placed in buf and returned.  Otherwise the contents of buf is undefined
   and NULL is returned.  */
const char *
find_exec (const char *name, path_conv& buf, const char *search,
	   unsigned opt, const char **known_suffix)
{
  const char *suffix = "";
  const char *retval = NULL;
  tmp_pathbuf tp;
  char *tmp_path;
  char *tmp = tp.c_get ();
  bool has_slash = !!strpbrk (name, "/\\");
  int err = 0;
  bool eopath = false;

  debug_printf ("find_exec (%s)", name);

  /* Check to see if file can be opened as is first. */
  if ((has_slash || opt & FE_CWD)
      && (suffix = perhaps_suffix (name, buf, err, opt)) != NULL)
    {
      /* Overwrite potential symlink target with original path.
	 See comment preceeding this method. */
      tmp_path = tmp;
      if (!has_slash)
	tmp_path = stpcpy (tmp, "./");
      stpcpy (tmp_path, name);
      buf.set_posix (tmp);
      retval = buf.get_posix ();
      goto out;
    }

  const char *path;
  /* If it starts with a slash, it's a PATH-like pathlist.  Otherwise it's
     the name of an environment variable. */
  if (strchr (search, '/'))
    *stpncpy (tmp, search, NT_MAX_PATH - 1) = '\0';
  else if (has_slash || isdrive (name))
    goto errout;
  /* Search the current directory when PATH is absent. This feature is
     added for Linux compatibility, but it is deprecated. POSIX notes
     that a conforming application shall use an explicit path name to
     specify the current working directory. */
  else if (!(path = getenv (search)) || !*path)
    strcpy (tmp, ".");
  else
    *stpncpy (tmp, path, NT_MAX_PATH - 1) = '\0';

  path = tmp;
  debug_printf ("searchpath %s", path);

  tmp_path = tp.c_get ();
  do
    {
      char *eotmp = strccpy (tmp_path, &path, ':');
      if (*path)
	path++;
      else
	eopath = true;
      /* An empty path or '.' means the current directory, but we've
	 already tried that.  */
      if ((opt & FE_CWD) && (tmp_path[0] == '\0'
			     || (tmp_path[0] == '.' && tmp_path[1] == '\0')))
	continue;
      /* An empty path means the current directory. This feature is
	 added for Linux compatibility, but it is deprecated. POSIX
	 notes that a conforming application shall use an explicit
	 pathname to specify the current working directory. */
      else if (tmp_path[0] == '\0')
	eotmp = stpcpy (tmp_path, ".");

      *eotmp++ = '/';
      stpcpy (eotmp, name);

      debug_printf ("trying %s", tmp_path);

      int err1;

      if ((suffix = perhaps_suffix (tmp_path, buf, err1, opt)) != NULL)
	{
	  if (buf.has_acls () && check_file_access (buf, X_OK, true))
	    continue;
	  /* Overwrite potential symlink target with original path.
	     See comment preceeding this method. */
	  buf.set_posix (tmp_path);
	  retval = buf.get_posix ();
	  goto out;
	}

    }
  while (!eopath);

 errout:
  /* Couldn't find anything in the given path.
     Take the appropriate action based on FE_NNF. */
  if (!(opt & FE_NNF))
    {
      buf.check (name, PC_SYM_FOLLOW | PC_POSIX);
      retval = buf.get_posix ();
    }

 out:
  debug_printf ("%s = find_exec (%s)", (char *) buf.get_posix (), name);
  if (known_suffix)
    *known_suffix = suffix ?: strchr (buf.get_win32 (), '\0');
  if (!retval && err)
    set_errno (err);
  return retval;
}

/* Utility for child_info_spawn::worker.  */

static HANDLE
handle (int fd, bool writing)
{
  HANDLE h;
  cygheap_fdget cfd (fd);

  if (cfd < 0)
    h = INVALID_HANDLE_VALUE;
  else if (cfd->close_on_exec ())
    h = INVALID_HANDLE_VALUE;
  else if (!writing)
    h = cfd->get_handle_nat ();
  else
    h = cfd->get_output_handle_nat ();

  return h;
}

int
iscmd (const char *argv0, const char *what)
{
  int n;
  n = strlen (argv0) - strlen (what);
  if (n >= 2 && argv0[1] != ':')
    return 0;
  return n >= 0 && strcasematch (argv0 + n, what) &&
	 (n == 0 || isdirsep (argv0[n - 1]));
}

#define ILLEGAL_SIG_FUNC_PTR ((_sig_func_ptr) (-2))
struct system_call_handle
{
  _sig_func_ptr oldint;
  _sig_func_ptr oldquit;
  sigset_t oldmask;
  bool is_system_call ()
  {
    return oldint != ILLEGAL_SIG_FUNC_PTR;
  }
  system_call_handle (bool issystem)
  {
    if (!issystem)
      oldint = ILLEGAL_SIG_FUNC_PTR;
    else
      {
	sig_send (NULL, __SIGHOLD);
	oldint = NULL;
      }
  }
  void arm()
  {
    if (is_system_call ())
      {
	sigset_t child_block;
	oldint = signal (SIGINT,  SIG_IGN);
	oldquit = signal (SIGQUIT, SIG_IGN);
	sigemptyset (&child_block);
	sigaddset (&child_block, SIGCHLD);
	sigprocmask (SIG_BLOCK, &child_block, &oldmask);
	sig_send (NULL, __SIGNOHOLD);
      }
  }
  ~system_call_handle ()
  {
    if (is_system_call ())
      {
	signal (SIGINT, oldint);
	signal (SIGQUIT, oldquit);
	sigprocmask (SIG_SETMASK, &oldmask, NULL);
      }
  }
# undef cleanup
};

child_info_spawn NO_COPY ch_spawn;

extern "C" void __posix_spawn_sem_release (void *sem, int error);

extern DWORD mutex_timeout; /* defined in fhandler_termios.cc */

int
child_info_spawn::worker (const char *prog_arg, const char *const *argv,
			  const char *const envp[], int mode,
			  int in__stdin, int in__stdout)
{
  bool rc;
  int res = -1;

  /* Check if we have been called from exec{lv}p or spawn{lv}p and mask
     mode to keep only the spawn mode. */
  bool p_type_exec = !!(mode & _P_PATH_TYPE_EXEC);
  mode = _P_MODE (mode);

  if (prog_arg == NULL)
    {
      syscall_printf ("prog_arg is NULL");
      set_errno (EFAULT);	/* As on Linux. */
      return -1;
    }
  if (!prog_arg[0])
    {
      syscall_printf ("prog_arg is empty");
      set_errno (ENOENT);	/* Per POSIX */
      return -1;
    }

  syscall_printf ("mode = %d, prog_arg = %.9500s", mode, prog_arg);

  /* FIXME: This is no error condition on Linux. */
  if (argv == NULL)
    {
      syscall_printf ("argv is NULL");
      set_errno (EINVAL);
      return -1;
    }

  av newargv;
  linebuf cmd;
  PWCHAR envblock = NULL;
  path_conv real_path;
  bool reset_sendsig = false;

  tmp_pathbuf tp;
  PWCHAR runpath = tp.w_get ();
  int c_flags;

  STARTUPINFOW si = {};
  int looped = 0;

  fhandler_termios::spawn_worker term_spawn_worker;

  system_call_handle system_call (mode == _P_SYSTEM);

  __try
    {
      child_info_types chtype;
      if (mode == _P_OVERLAY)
	chtype = _CH_EXEC;
      else
	chtype = _CH_SPAWN;

      moreinfo = cygheap_exec_info::alloc ();

      /* CreateProcess takes one long string that is the command line (sigh).
	 We need to quote any argument that has whitespace or embedded "'s.  */

      int ac;
      for (ac = 0; argv[ac]; ac++)
	/* nothing */;

      int err;
      const char *ext;
      if ((ext = perhaps_suffix (prog_arg, real_path, err, FE_NADA)) == NULL)
	{
	  set_errno (err);
	  res = -1;
	  __leave;
	}

      res = newargv.setup (prog_arg, real_path, ext, ac, argv, p_type_exec);

      if (res)
	__leave;

      if (!real_path.iscygexec () && ::cygheap->cwd.get_error ())
	{
	  small_printf ("Error: Current working directory %s.\n"
			"Can't start native Windows application from here.\n\n",
			::cygheap->cwd.get_error_desc ());
	  set_errno (::cygheap->cwd.get_error ());
	  res = -1;
	  __leave;
	}

      if (real_path.iscygexec ())
	{
	  moreinfo->argc = newargv.argc;
	  moreinfo->argv = newargv;
	}
      if ((wincmdln || !real_path.iscygexec ())
	   && !cmd.fromargv (newargv, real_path.get_win32 (),
			     real_path.iscygexec ()))
	{
	  res = -1;
	  __leave;
	}


      if (mode != _P_OVERLAY || !real_path.iscygexec ()
	  || !DuplicateHandle (GetCurrentProcess (), myself.shared_handle (),
			       GetCurrentProcess (), &moreinfo->myself_pinfo,
			       0, TRUE, DUPLICATE_SAME_ACCESS))
	moreinfo->myself_pinfo = NULL;
      else
	VerifyHandle (moreinfo->myself_pinfo);

      PROCESS_INFORMATION pi;
      pi.hProcess = pi.hThread = NULL;
      pi.dwProcessId = pi.dwThreadId = 0;

      c_flags = GetPriorityClass (GetCurrentProcess ());
      sigproc_printf ("priority class %d", c_flags);

      c_flags |= CREATE_SEPARATE_WOW_VDM | CREATE_UNICODE_ENVIRONMENT;

      /* Add CREATE_DEFAULT_ERROR_MODE flag for non-Cygwin processes so they
	 get the default error mode instead of inheriting the mode Cygwin
	 uses.  This allows things like Windows Error Reporting/JIT debugging
	 to work with processes launched from a Cygwin shell. */
      if (!real_path.iscygexec ())
	c_flags |= CREATE_DEFAULT_ERROR_MODE;

      /* We're adding the CREATE_BREAKAWAY_FROM_JOB flag here to workaround
	 issues with the "Program Compatibility Assistant (PCA) Service".
	 For some reason, when starting long running sessions from mintty(*),
	 the affected svchost.exe process takes more and more memory and at one
	 point takes over the CPU.  At this point the machine becomes
	 unresponsive.  The only way to get back to normal is to stop the
	 entire mintty session, or to stop the PCA service.  However, a process
	 which is controlled by PCA is part of a compatibility job, which
	 allows child processes to break away from the job.  This helps to
	 avoid this issue.

	 First we call IsProcessInJob.  It fetches the information whether or
	 not we're part of a job 20 times faster than QueryInformationJobObject.

	 (*) Note that this is not mintty's fault.  It has just been observed
	 with mintty in the first place.  See the archives for more info:
	 http://cygwin.com/ml/cygwin-developers/2012-02/msg00018.html */
      JOBOBJECT_BASIC_LIMIT_INFORMATION jobinfo;
      BOOL is_in_job;

      if (IsProcessInJob (GetCurrentProcess (), NULL, &is_in_job)
	  && is_in_job
	  && QueryInformationJobObject (NULL, JobObjectBasicLimitInformation,
				     &jobinfo, sizeof jobinfo, NULL)
	  && (jobinfo.LimitFlags & (JOB_OBJECT_LIMIT_BREAKAWAY_OK
				    | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK)))
	{
	  debug_printf ("Add CREATE_BREAKAWAY_FROM_JOB");
	  c_flags |= CREATE_BREAKAWAY_FROM_JOB;
	}

      if (mode == _P_DETACH)
	c_flags |= DETACHED_PROCESS;
      else
	fhandler_console::need_invisible ();

      if (mode != _P_OVERLAY)
	myself->exec_sendsig = NULL;
      else
	{
	  /* Reset sendsig so that any process which wants to send a signal
	     to this pid will wait for the new process to become active.
	     Save the old value in case the exec fails.  */
	  if (!myself->exec_sendsig)
	    {
	      myself->exec_sendsig = myself->sendsig;
	      myself->exec_dwProcessId = myself->dwProcessId;
	      myself->sendsig = NULL;
	      reset_sendsig = true;
	    }
	}

      USHORT len = real_path.get_nt_native_path ()->Length / sizeof (WCHAR);
      if (RtlEqualUnicodePathPrefix (real_path.get_nt_native_path (),
				     &ro_u_natp, FALSE))
	{
	  runpath = real_path.get_wide_win32_path (runpath);
	  /* If the executable path length is < MAX_PATH, make sure the long
	     path win32 prefix is removed from the path to make subsequent
	     not long path aware native Win32 child processes happy. */
	  if (len < MAX_PATH + 4)
	    {
	      if (runpath[5] == ':')
		runpath += 4;
	      else if (len < MAX_PATH + 6)
		*(runpath += 6) = L'\\';
	    }
	}
      else if (len < NT_MAX_PATH - ro_u_globalroot.Length / sizeof (WCHAR))
	{
	  UNICODE_STRING rpath;

	  RtlInitEmptyUnicodeString (&rpath, runpath,
				     (NT_MAX_PATH - 1) * sizeof (WCHAR));
	  RtlCopyUnicodeString (&rpath, &ro_u_globalroot);
	  RtlAppendUnicodeStringToString (&rpath,
					  real_path.get_nt_native_path ());
	}
      else
	{
	  set_errno (ENAMETOOLONG);
	  res = -1;
	  __leave;
	}

      cygbench ("spawn-worker");

      if (!real_path.iscygexec())
	::cygheap->fdtab.set_file_pointers_for_exec ();

      /* If we switch the user, merge the user's Windows environment. */
      bool switch_user = ::cygheap->user.issetuid ()
			 && (::cygheap->user.saved_uid
			     != ::cygheap->user.real_uid);
      moreinfo->envp = build_env (envp, envblock, moreinfo->envc,
				  real_path.iscygexec (),
				  switch_user ? ::cygheap->user.primary_token ()
					      : NULL);
      if (!moreinfo->envp || !envblock)
	{
	  set_errno (E2BIG);
	  res = -1;
	  __leave;
	}
      set (chtype, real_path.iscygexec ());
      __stdin = in__stdin;
      __stdout = in__stdout;
      record_children ();

      si.lpReserved2 = (LPBYTE) this;
      si.cbReserved2 = sizeof (*this);

      /* Depends on set call above.
	 Some file types might need extra effort in the parent after CreateProcess
	 and before copying the datastructures to the child.  So we have to start
	 the child in suspend state, unfortunately, to avoid a race condition. */
      if (!newargv.win16_exe
	  && (!iscygwin () || mode != _P_OVERLAY
	      || ::cygheap->fdtab.need_fixup_before ()))
	c_flags |= CREATE_SUSPENDED;
      /* If a native application should be spawned, we test here if the spawning
	 process is running in a console and, if so, if it's a foreground or
	 background process.  If it's a background process, we start the native
	 process with the CREATE_NEW_PROCESS_GROUP flag set.  This lets the native
	 process ignore Ctrl-C by default.  If we don't do that, pressing Ctrl-C
	 in a console will break native processes running in the background,
	 because the Ctrl-C event is sent to all processes in the console, unless
	 they ignore it explicitely.  CREATE_NEW_PROCESS_GROUP does that for us. */
      pid_t ctty_pgid =
	::cygheap->ctty ? ::cygheap->ctty->tc_getpgid () : 0;
      if (!iscygwin () && ctty_pgid && ctty_pgid != myself->pgid)
	c_flags |= CREATE_NEW_PROCESS_GROUP;
      refresh_cygheap ();

      if (c_flags & CREATE_NEW_PROCESS_GROUP)
	myself->process_state |= PID_NEW_PG;

      if (mode == _P_DETACH)
	/* all set */;
      else if (mode != _P_OVERLAY || !my_wr_proc_pipe)
	prefork ();
      else
	wr_proc_pipe = my_wr_proc_pipe;

      /* Don't allow child to inherit these handles if it's not a Cygwin program.
	 wr_proc_pipe will be injected later.  parent won't be used by the child
	 so there is no reason for the child to have it open as it can confuse
	 ps into thinking that children of windows processes are all part of
	 the same "execed" process.
	 FIXME: Someday, make it so that parent is never created when starting
	 non-Cygwin processes. */
      if (!iscygwin ())
	{
	  SetHandleInformation (wr_proc_pipe, HANDLE_FLAG_INHERIT, 0);
	  SetHandleInformation (parent, HANDLE_FLAG_INHERIT, 0);
	}
      /* FIXME: racy */
      if (mode != _P_OVERLAY)
	SetHandleInformation (my_wr_proc_pipe, HANDLE_FLAG_INHERIT, 0);
      parent_winpid = GetCurrentProcessId ();

      PSECURITY_ATTRIBUTES sa = (PSECURITY_ATTRIBUTES) alloca (1024);
      if (!sec_user_nih (sa, cygheap->user.sid (),
			 well_known_authenticated_users_sid,
			 PROCESS_QUERY_LIMITED_INFORMATION))
	sa = &sec_none_nih;

      int fileno_stdin = in__stdin < 0 ? 0 : in__stdin;
      int fileno_stdout = in__stdout < 0 ? 1 : in__stdout;
      int fileno_stderr = 2;

      if (!iscygwin ())
	{
	  bool need_send_sig = false;
	  int fd;
	  cygheap_fdenum cfd (false);
	  while ((fd = cfd.next ()) >= 0)
	    if (cfd->get_dev () == FH_PIPEW
		     && (fd == fileno_stdout || fd == fileno_stderr))
	      {
		fhandler_pipe *pipe = (fhandler_pipe *)(fhandler_base *) cfd;
		pipe->set_pipe_non_blocking (false);
		if (pipe->request_close_query_hdl ())
		  need_send_sig = true;
	      }
	    else if (cfd->get_dev () == FH_PIPER && fd == fileno_stdin)
	      {
		fhandler_pipe *pipe = (fhandler_pipe *)(fhandler_base *) cfd;
		pipe->set_pipe_non_blocking (false);
	      }

	  if (need_send_sig)
	    {
	      tty_min dummy_tty;
	      dummy_tty.ntty = (fh_devices) myself->ctty;
	      dummy_tty.pgid = myself->pgid;
	      tty_min *t = cygwin_shared->tty.get_cttyp ();
	      if (!t) /* If tty is not allocated, use dummy_tty instead. */
		t = &dummy_tty;
	      /* Emit __SIGNONCYGCHLD to let all processes in the
		 process group close query_hdl. */
	      t->kill_pgrp (__SIGNONCYGCHLD);
	    }
	}

      bool no_pcon = mode != _P_OVERLAY && mode != _P_WAIT;
      term_spawn_worker.setup (iscygwin (), handle (fileno_stdin, false),
			       runpath, no_pcon, reset_sendsig, envblock);

      /* Set up needed handles for stdio */
      si.dwFlags = STARTF_USESTDHANDLES;
      si.hStdInput = handle (fileno_stdin, false);
      si.hStdOutput = handle (fileno_stdout, true);
      si.hStdError = handle (fileno_stderr, true);

      si.cb = sizeof (si);

      if (!iscygwin ())
	init_console_handler (CTTY_IS_VALID (myself->ctty));

    loop:
      /* When ruid != euid we create the new process under the current original
	 account and impersonate in child, this way maintaining the different
	 effective vs. real ids.
	 FIXME: If ruid != euid and ruid != saved_uid we currently give
	 up on ruid. The new process will have ruid == euid. */
      ::cygheap->user.deimpersonate ();

      if (!real_path.iscygexec () && mode == _P_OVERLAY)
	myself->process_state |= PID_NOTCYGWIN;

      cygpid = (mode != _P_OVERLAY) ? create_cygwin_pid () : myself->pid;

      wchar_t wcmd[(size_t) cmd];
      if (!::cygheap->user.issetuid ()
	  || (::cygheap->user.saved_uid == ::cygheap->user.real_uid
	      && ::cygheap->user.saved_gid == ::cygheap->user.real_gid
	      && !::cygheap->user.groups.issetgroups ()
	      && !::cygheap->user.setuid_to_restricted))
	{
	  rc = CreateProcessW (runpath,		/* image name w/ full path */
			       cmd.wcs (wcmd),	/* what was passed to exec */
			       sa,		/* process security attrs */
			       sa,		/* thread security attrs */
			       TRUE,		/* inherit handles */
			       c_flags,
			       envblock,	/* environment */
			       NULL,
			       &si,
			       &pi);
	}
      else
	{
	  /* Give access to myself */
	  if (mode == _P_OVERLAY)
	    myself.set_acl();

	  HWINSTA hwst = NULL;
	  HWINSTA hwst_orig = GetProcessWindowStation ();
	  HDESK hdsk = NULL;
	  HDESK hdsk_orig = GetThreadDesktop (GetCurrentThreadId ());
	  /* Don't create WindowStation and Desktop for restricted child. */
	  if (!::cygheap->user.setuid_to_restricted)
	    {
	      PSECURITY_ATTRIBUTES sa;
	      WCHAR sid[128];
	      WCHAR wstname[1024] = { L'\0' };

	      sa = sec_user ((PSECURITY_ATTRIBUTES) alloca (1024),
			     ::cygheap->user.sid ());
	      /* We're creating a window station per user, not per logon
		 session.  It doesn't make sense in terms of security to
		 create a new window station for every logon of the same user.
		 It just fills up the system with window stations. */
	      hwst = CreateWindowStationW (::cygheap->user.get_windows_id (sid),
					   0, GENERIC_READ | GENERIC_WRITE, sa);
	      if (!hwst)
		system_printf ("CreateWindowStation failed, %E");
	      else if (!SetProcessWindowStation (hwst))
		system_printf ("SetProcessWindowStation failed, %E");
	      else if (!(hdsk = CreateDesktopW (L"Default", NULL, NULL, 0,
						GENERIC_ALL, sa)))
		system_printf ("CreateDesktop failed, %E");
	      else
		{
		  wcpcpy (wcpcpy (wstname, sid), L"\\Default");
		  si.lpDesktop = wstname;
		  debug_printf ("Desktop: %W", si.lpDesktop);
		}
	    }

	  rc = CreateProcessAsUserW (::cygheap->user.primary_token (),
			       runpath,		/* image name w/ full path */
			       cmd.wcs (wcmd),	/* what was passed to exec */
			       sa,		/* process security attrs */
			       sa,		/* thread security attrs */
			       TRUE,		/* inherit handles */
			       c_flags,
			       envblock,	/* environment */
			       NULL,
			       &si,
			       &pi);
	  if (hwst)
	    {
	      SetProcessWindowStation (hwst_orig);
	      CloseWindowStation (hwst);
	    }
	  if (hdsk)
	    {
	      SetThreadDesktop (hdsk_orig);
	      CloseDesktop (hdsk);
	    }
	}

      if (mode != _P_OVERLAY)
	SetHandleInformation (my_wr_proc_pipe, HANDLE_FLAG_INHERIT,
			      HANDLE_FLAG_INHERIT);

      /* Set errno now so that debugging messages from it appear before our
	 final debugging message [this is a general rule for debugging
	 messages].  */
      if (!rc)
	{
	  __seterrno ();
	  syscall_printf ("CreateProcess failed, %E");
	  /* If this was a failed exec, restore the saved sendsig. */
	  if (reset_sendsig)
	    {
	      myself->sendsig = myself->exec_sendsig;
	      myself->exec_sendsig = NULL;
	    }
	  myself->process_state &= ~PID_NOTCYGWIN;
	  /* Reset handle inheritance to default when the execution of a'
	     non-Cygwin process fails.  Only need to do this for _P_OVERLAY
	     since the handle will be closed otherwise.  Don't need to do
	     this for 'parent' since it will be closed in every case.
	     See FIXME above. */
	  if (!iscygwin () && mode == _P_OVERLAY)
	    SetHandleInformation (wr_proc_pipe, HANDLE_FLAG_INHERIT,
				  HANDLE_FLAG_INHERIT);
	  if (wr_proc_pipe == my_wr_proc_pipe)
	    wr_proc_pipe = NULL; /* We still own it: don't nuke in destructor */

	  /* Restore impersonation. In case of _P_OVERLAY this isn't
	     allowed since it would overwrite child data. */
	  if (mode != _P_OVERLAY)
	    ::cygheap->user.reimpersonate ();

	  res = -1;
	  __leave;
	}

      /* The CREATE_SUSPENDED case is handled below */
      if (iscygwin () && !(c_flags & CREATE_SUSPENDED))
	strace.write_childpid (pi.dwProcessId);

      /* Fixup the parent data structures if needed and resume the child's
	 main thread. */
      if (::cygheap->fdtab.need_fixup_before ())
	::cygheap->fdtab.fixup_before_exec (pi.dwProcessId);

      /* Print the original program name here so the user can see that too.  */
      syscall_printf ("pid %d, prog_arg %s, cmd line %.9500s)",
		      rc ? cygpid : (unsigned int) -1, prog_arg, (const char *) cmd);

      /* Name the handle similarly to proc_subproc. */
      ProtectHandle1 (pi.hProcess, childhProc);

      if (mode == _P_OVERLAY)
	{
	  myself->dwProcessId = pi.dwProcessId;
	  strace.execing = 1;
	  myself.hProcess = hExeced = pi.hProcess;
	  HANDLE old_winpid_hdl = myself.shared_winpid_handle ();
	  /* We have to create a new winpid symlink on behalf of the child
	     process. For Cygwin processes we also have to create a reference
	     in the child. */
	  myself.create_winpid_symlink ();
	  if (real_path.iscygexec ())
	    DuplicateHandle (GetCurrentProcess (),
			     myself.shared_winpid_handle (),
			     pi.hProcess, NULL, 0, 0, DUPLICATE_SAME_ACCESS);
	  NtClose (old_winpid_hdl);
	  real_path.get_wide_win32_path (myself->progname); // FIXME: race?
	  sigproc_printf ("new process name %W", myself->progname);
	  if (!iscygwin ())
	    close_all_files ();
	}
      else
	{
	  myself->set_has_pgid_children ();
	  ProtectHandle (pi.hThread);
	  pinfo child (cygpid,
		       PID_IN_USE | (real_path.iscygexec () ? 0 : PID_NOTCYGWIN));
	  if (!child)
	    {
	      syscall_printf ("pinfo failed");
	      if (get_errno () != ENOMEM)
		set_errno (EAGAIN);
	      res = -1;
	      __leave;
	    }
	  child->dwProcessId = pi.dwProcessId;
	  child.hProcess = pi.hProcess;

	  real_path.get_wide_win32_path (child->progname);
	  /* This introduces an unreferenced, open handle into the child.
	     The purpose is to keep the pid shared memory open so that all
	     of the fields filled out by child.remember do not disappear
	     and so there is not a brief period during which the pid is
	     not available. */
	  DuplicateHandle (GetCurrentProcess (), child.shared_handle (),
			   pi.hProcess, NULL, 0, 0, DUPLICATE_SAME_ACCESS);
	  if (!real_path.iscygexec ())
	    {
	      /* If the child process is not a Cygwin process, we have to
		 create a new winpid symlink and induce it into the child
		 process as well to keep it over the lifetime of the child. */
	      child.create_winpid_symlink ();
	      DuplicateHandle (GetCurrentProcess (),
			       child.shared_winpid_handle (),
			       pi.hProcess, NULL, 0, 0, DUPLICATE_SAME_ACCESS);
	    }
	  child->start_time = time (NULL); /* Register child's starting time. */
	  child->nice = myself->nice;
	  postfork (child);
	  if (mode != _P_DETACH
	      && (!child.remember () || !child.attach ()))
	    {
	      /* FIXME: Child in strange state now */
	      CloseHandle (pi.hProcess);
	      ForceCloseHandle (pi.hThread);
	      res = -1;
	      __leave;
	    }
	}

      /* Start the child running */
      if (c_flags & CREATE_SUSPENDED)
	{
	  /* Inject a non-inheritable wr_proc_pipe handle into child so that we
	     can accurately track when the child exits without keeping this
	     process waiting around for it to exit.  */
	  if (!iscygwin ())
	    DuplicateHandle (GetCurrentProcess (), wr_proc_pipe, pi.hProcess, NULL,
			     0, false, DUPLICATE_SAME_ACCESS);
	  ResumeThread (pi.hThread);
	  if (iscygwin ())
	    strace.write_childpid (pi.dwProcessId);
	}
      ForceCloseHandle (pi.hThread);

      sigproc_printf ("spawned windows pid %d", pi.dwProcessId);

      bool synced;
      if ((mode == _P_DETACH || mode == _P_NOWAIT) && !iscygwin ())
	synced = false;
      else
	/* Just mark a non-cygwin process as 'synced'.  We will still eventually
	   wait for it to exit in maybe_set_exit_code_from_windows(). */
	synced = iscygwin () ? sync (pi.dwProcessId, pi.hProcess, INFINITE) : true;

      switch (mode)
	{
	case _P_OVERLAY:
	  myself.hProcess = pi.hProcess;
	  if (!synced)
	    {
	      if (!proc_retry (pi.hProcess))
		{
		  looped++;
		  goto loop;
		}
	      close_all_files (true);
	    }
	  else
	    {
	      if (iscygwin ())
		close_all_files (true);
	      if (!my_wr_proc_pipe
		  && WaitForSingleObject (pi.hProcess, 0) == WAIT_TIMEOUT)
		wait_for_myself ();
	    }
	  if (sem)
	    __posix_spawn_sem_release (sem, 0);
	  if (term_spawn_worker.need_cleanup ())
	    {
	      LONG prev_sigExeced = sigExeced;
	      while (WaitForSingleObject (pi.hProcess, 100) == WAIT_TIMEOUT)
		/* If child process does not exit in predetermined time
		   period, the process does not seem to be terminated by
		   the signal sigExeced. Therefore, clear sigExeced here. */
		prev_sigExeced =
		  InterlockedCompareExchange (&sigExeced, 0, prev_sigExeced);
	      term_spawn_worker.cleanup ();
	      term_spawn_worker.close_handle_set ();
	    }
	  /* Make sure that ctrl_c_handler() is not on going. Calling
	     init_console_handler(false) locks until returning from
	     ctrl_c_handler(). This insures that setting sigExeced
	     on Ctrl-C key has been completed. */
	  init_console_handler (false);
	  myself.exit (EXITCODE_NOSET);
	  break;
	case _P_WAIT:
	case _P_SYSTEM:
	  system_call.arm ();
	  if (waitpid (cygpid, &res, 0) != cygpid)
	    res = -1;
	  term_spawn_worker.cleanup ();
	  break;
	case _P_DETACH:
	  res = 0;	/* Lost all memory of this child. */
	  break;
	case _P_NOWAIT:
	case _P_NOWAITO:
	case _P_VFORK:
	  res = cygpid;
	  break;
	default:
	  break;
	}
    }
  __except (NO_ERROR)
    {
      if (get_errno () == ENOMEM)
	set_errno (E2BIG);
      else
	set_errno (EFAULT);
      res = -1;
    }
  __endtry
  term_spawn_worker.close_handle_set ();
  this->cleanup ();
  if (envblock)
    free (envblock);

  return (int) res;
}

extern "C" int
cwait (int *result, int pid, int)
{
  return waitpid (pid, result, 0);
}

/*
* Helper function for spawn runtime calls.
* Doesn't search the path.
*/

extern "C" int
spawnve (int mode, const char *path, const char *const *argv,
       const char *const *envp)
{
  static char *const empty_env[] = { NULL };

  int ret;

  syscall_printf ("spawnve (%s, %s, %p)", path, argv[0], envp);

  if (!envp)
    envp = empty_env;

  switch (_P_MODE (mode))
    {
    case _P_OVERLAY:
      ch_spawn.worker (path, argv, envp, mode);
      /* Errno should be set by worker.  */
      ret = -1;
      break;
    case _P_VFORK:
    case _P_NOWAIT:
    case _P_NOWAITO:
    case _P_WAIT:
    case _P_DETACH:
    case _P_SYSTEM:
      ret = ch_spawn.worker (path, argv, envp, mode);
      break;
    default:
      set_errno (EINVAL);
      ret = -1;
      break;
    }
  return ret;
}

/*
* spawn functions as implemented in the MS runtime library.
* Most of these based on (and copied from) newlib/libc/posix/execXX.c
*/

extern "C" int
spawnl (int mode, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[256];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  va_end (args);

  return spawnve (mode, path, (char * const  *) argv, environ);
}

extern "C" int
spawnle (int mode, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char * const *envp;
  const char *argv[256];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
    argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  envp = va_arg (args, const char * const *);
  va_end (args);

  return spawnve (mode, path, (char * const *) argv, (char * const *) envp);
}

extern "C" int
spawnlp (int mode, const char *file, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[256];
  path_conv buf;

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  va_end (args);

  return spawnve (mode | _P_PATH_TYPE_EXEC, find_exec (file, buf),
		  (char * const *) argv, environ);
}

extern "C" int
spawnlpe (int mode, const char *file, const char *arg0, ...)
{
  int i;
  va_list args;
  const char * const *envp;
  const char *argv[256];
  path_conv buf;

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
    argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  envp = va_arg (args, const char * const *);
  va_end (args);

  return spawnve (mode | _P_PATH_TYPE_EXEC, find_exec (file, buf),
		  (char * const *) argv, envp);
}

extern "C" int
spawnv (int mode, const char *path, const char * const *argv)
{
  return spawnve (mode, path, argv, environ);
}

extern "C" int
spawnvp (int mode, const char *file, const char * const *argv)
{
  path_conv buf;
  return spawnve (mode | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH", FE_NNF) ?: "",
		  argv, environ);
}

extern "C" int
spawnvpe (int mode, const char *file, const char * const *argv,
	  const char * const *envp)
{
  path_conv buf;
  return spawnve (mode | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH", FE_NNF) ?: "",
		  argv, envp);
}

int
av::setup (const char *prog_arg, path_conv& real_path, const char *ext,
	   int argc, const char *const *argv, bool p_type_exec)
{
  const char *p;
  bool exeext = ascii_strcasematch (ext, ".exe");
  new (this) av (argc, argv);
  if ((exeext && real_path.iscygexec ()) || ascii_strcasematch (ext, ".bat")
      || (!*ext && ((p = ext - 4) > real_path.get_win32 ())
	  && (ascii_strcasematch (p, ".bat") || ascii_strcasematch (p, ".cmd")
	      || ascii_strcasematch (p, ".btm"))))
    /* no extra checks needed */;
  else
    while (1)
      {
	char *pgm = NULL;
	char *arg1 = NULL;
	char *ptr, *buf;
	OBJECT_ATTRIBUTES attr;
	IO_STATUS_BLOCK io;
	HANDLE h;
	NTSTATUS status;
	LARGE_INTEGER size;

	status = NtOpenFile (&h, SYNCHRONIZE | GENERIC_READ,
			     real_path.get_object_attr (attr, sec_none_nih),
			     &io, FILE_SHARE_VALID_FLAGS,
			     FILE_SYNCHRONOUS_IO_NONALERT
			     | FILE_OPEN_FOR_BACKUP_INTENT
			     | FILE_NON_DIRECTORY_FILE);
	if (status == STATUS_IO_REPARSE_TAG_NOT_HANDLED)
	  {
	    /* This is most likely an app execution alias (such as the
	       Windows Store version of Python, i.e. not a Cygwin program */
	    real_path.set_cygexec (false);
	    break;
	  }
	if (!NT_SUCCESS (status))
	  {
	    /* File is not readable?  Doesn't mean it's not executable.
	       Test for executability and if so, just assume the file is
	       a cygwin executable and go ahead. */
	    if (status == STATUS_ACCESS_DENIED && real_path.has_acls ()
		&& check_file_access (real_path, X_OK, true) == 0)
	      {
		real_path.set_cygexec (true);
		break;
	      }
	    SetLastError (RtlNtStatusToDosError (status));
	    goto err;
	  }
	if (!GetFileSizeEx (h, &size))
	  {
	    NtClose (h);
	    goto err;
	  }
	if (size.QuadPart > (LONGLONG) wincap.allocation_granularity ())
	  size.LowPart = wincap.allocation_granularity ();

	HANDLE hm = CreateFileMapping (h, &sec_none_nih, PAGE_READONLY,
				       0, 0, NULL);
	NtClose (h);
	if (!hm)
	  {
	    /* ERROR_FILE_INVALID indicates very likely an empty file. */
	    if (GetLastError () == ERROR_FILE_INVALID)
	      {
		debug_printf ("zero length file, treat as script.");
		goto just_shell;
	      }
	    goto err;
	  }
	/* Try to map the first 64K of the image.  That's enough for the local
	   tests, and it's enough for hook_or_detect_cygwin to compute the IAT
	   address. */
	buf = (char *) MapViewOfFile (hm, FILE_MAP_READ, 0, 0, size.LowPart);
	if (!buf)
	  {
	    CloseHandle (hm);
	    goto err;
	  }

	{
	  __try
	    {
	      if (buf[0] == 'M' && buf[1] == 'Z')
		{
		  WORD subsys;
		  unsigned off = (unsigned char) buf[0x18] | (((unsigned char) buf[0x19]) << 8);
		  win16_exe = off < sizeof (IMAGE_DOS_HEADER);
		  if (!win16_exe)
		    real_path.set_cygexec (hook_or_detect_cygwin (buf, NULL,
								  subsys, hm));
		  else
		    real_path.set_cygexec (false);
		  UnmapViewOfFile (buf);
		  CloseHandle (hm);
		  break;
		}
	    }
	  __except (NO_ERROR)
	    {
	      UnmapViewOfFile (buf);
	      CloseHandle (hm);
	      real_path.set_cygexec (false);
	      break;
	    }
	  __endtry
	}
	CloseHandle (hm);

	debug_printf ("%s is possibly a script", real_path.get_win32 ());

	ptr = buf;
	if (*ptr++ == '#' && *ptr++ == '!')
	  {
	    ptr += strspn (ptr, " \t");
	    size_t len = strcspn (ptr, "\r\n");
	    while (ptr[len - 1] == ' ' || ptr[len - 1] == '\t')
	      len--;
	    if (len)
	      {
		char *namebuf = (char *) alloca (len + 1);
		memcpy (namebuf, ptr, len);
		namebuf[len] = '\0';
		for (ptr = pgm = namebuf; *ptr; ptr++)
		  if (!arg1 && (*ptr == ' ' || *ptr == '\t'))
		    {
		      /* Null terminate the initial command and step over any
			 additional white space.  If we've hit the end of the
			 line, exit the loop.  Otherwise, we've found the first
			 argument. Position the current pointer on the last known
			 white space. */
		      *ptr = '\0';
		      char *newptr = ptr + 1;
		      newptr += strspn (newptr, " \t");
		      if (!*newptr)
			break;
		      arg1 = newptr;
		      ptr = newptr - 1;
		    }
	      }
	  }
	UnmapViewOfFile (buf);
  just_shell:
	/* Check if script is executable.  Otherwise we start non-executable
	   scripts successfully, which is incorrect behaviour. */
	if (real_path.has_acls ()
	    && check_file_access (real_path, X_OK, true) < 0)
	  return -1;	/* errno is already set. */

	if (!pgm)
	  {
	    if (!p_type_exec)
	      {
		/* Not called from exec[lv]p.  Don't try to treat as script. */
		debug_printf ("%s is not a valid executable",
			      real_path.get_win32 ());
		set_errno (ENOEXEC);
		return -1;
	      }
	    pgm = (char *) "/bin/sh";
	    arg1 = NULL;
	  }

	/* Replace argv[0] with the full path to the script if this is the
	   first time through the loop. */
	replace0_maybe (prog_arg);

	/* pointers:
	 * pgm	interpreter name
	 * arg1	optional string
	 */
	if (arg1)
	  unshift (arg1);

	find_exec (pgm, real_path, "PATH", FE_NADA, &ext);
	unshift (real_path.get_posix ());
      }
  if (real_path.iscygexec ())
    dup_all ();
  return 0;

err:
  __seterrno ();
  return -1;
}

/* The following __posix_spawn_* functions are called from newlib's posix_spawn
   implementation.  The original code in newlib has been taken from FreeBSD,
   and the core code relies on specific, non-portable behaviour of vfork(2).
   Our replacement implementation uses a semaphore to synchronize parent and
   child process.  Note: __posix_spawn_fork in fork.cc is part of the set. */

/* Create an inheritable semaphore.  Set it to 0 (== non-signalled), so the
   parent can wait on the semaphore immediately. */
extern "C" int
__posix_spawn_sem_create (void **semp)
{
  HANDLE sem;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!semp)
    return EINVAL;
  InitializeObjectAttributes (&attr, NULL, OBJ_INHERIT, NULL, NULL);
  status = NtCreateSemaphore (&sem, SEMAPHORE_ALL_ACCESS, &attr, 0, INT_MAX);
  if (!NT_SUCCESS (status))
    return geterrno_from_nt_status (status);
  *semp = sem;
  return 0;
}

/* Signal the semaphore.  "error" should be 0 if all went fine and the
   exec'd child process is up and running, a useful POSIX error code otherwise.
   After releasing the semaphore, the value of the semaphore reflects
   the error code + 1.  Thus, after WFMO in__posix_spawn_sem_wait_and_close,
   querying the value of the semaphore returns either 0 if all went well,
   or a value > 0 equivalent to the POSIX error code. */
extern "C" void
__posix_spawn_sem_release (void *sem, int error)
{
  ReleaseSemaphore (sem, error + 1, NULL);
}

/* Helper to check the semaphore value. */
static inline int
__posix_spawn_sem_query (void *sem)
{
  SEMAPHORE_BASIC_INFORMATION sbi;

  NtQuerySemaphore (sem, SemaphoreBasicInformation, &sbi, sizeof sbi, NULL);
  return sbi.CurrentCount;
}

/* Called from parent to wait for fork/exec completion.  We're waiting for
   the semaphore as well as the child's process handle, so even if the
   child crashes without signalling the semaphore, we won't wait infinitely. */
extern "C" int
__posix_spawn_sem_wait_and_close (void *sem, void *proc)
{
  int ret = 0;
  HANDLE w4[2] = { sem, proc };

  switch (WaitForMultipleObjects (2, w4, FALSE, INFINITE))
    {
    case WAIT_OBJECT_0:
      ret = __posix_spawn_sem_query (sem);
      break;
    case WAIT_OBJECT_0 + 1:
      /* If we return here due to the child process dying, the semaphore is
	 very likely not signalled.  Check this here and return a valid error
	 code. */
      ret = __posix_spawn_sem_query (sem);
      if (ret == 0)
	ret = ECHILD;
      break;
    default:
      ret = geterrno_from_win_error ();
      break;
    }

  CloseHandle (sem);
  return ret;
}

/* Replacement for execve/execvpe, called from forked child in newlib's
   posix_spawn.  The relevant difference is the additional semaphore
   so the worker method (which is not supposed to return on success)
   can signal the semaphore after sync'ing with the exec'd child. */
extern "C" int
__posix_spawn_execvpe (const char *path, char * const *argv, char *const *envp,
		       HANDLE sem, int use_env_path)
{
  path_conv buf;

  static char *const empty_env[] = { NULL };
  if (!envp)
    envp = empty_env;
  ch_spawn.set_sem (sem);
  ch_spawn.worker (use_env_path ? (find_exec (path, buf, "PATH", FE_NNF) ?: "")
				: path,
		   argv, envp,
		   _P_OVERLAY | (use_env_path ? _P_PATH_TYPE_EXEC : 0));
  __posix_spawn_sem_release (sem, errno);
  return -1;
}
