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

static const suffix_info exe_suffixes[] =
{
  suffix_info ("", 1),
  suffix_info (".exe", 1),
  suffix_info (".com"),
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
const char * __reg3
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
  else if (has_slash || isdrive (name) || !(path = getenv (search)) || !*path)
    goto errout;
  else
    *stpncpy (tmp, path, NT_MAX_PATH - 1) = '\0';

  path = tmp;
  debug_printf ("searchpath %s", path);

  tmp_path = tp.c_get ();
  do
    {
      char *eotmp = strccpy (tmp_path, &path, ':');
      /* An empty path or '.' means the current directory, but we've
	 already tried that.  */
      if ((opt & FE_CWD) && (tmp_path[0] == '\0'
			     || (tmp_path[0] == '.' && tmp_path[1] == '\0')))
	continue;

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
  while (*path && *++path);

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
    h = cfd->get_handle ();
  else
    h = cfd->get_output_handle ();

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

int
child_info_spawn::worker (const char *prog_arg, const char *const *argv,
			  const char *const envp[], int mode,
			  int in__stdin, int in__stdout)
{
  bool rc;
  pid_t cygpid;
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

  bool null_app_name = false;
  STARTUPINFOW si = {};
  int looped = 0;

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

      if (ac == 3 && argv[1][0] == '/' && tolower (argv[1][1]) == 'c' &&
	  (iscmd (argv[0], "command.com") || iscmd (argv[0], "cmd.exe")))
	{
	  real_path.check (prog_arg);
	  cmd.add ("\"");
	  if (!real_path.error)
	    cmd.add (real_path.get_win32 ());
	  else
	    cmd.add (argv[0]);
	  cmd.add ("\"");
	  cmd.add (" ");
	  cmd.add (argv[1]);
	  cmd.add (" ");
	  cmd.add (argv[2]);
	  real_path.set_path (argv[0]);
	  null_app_name = true;
	}
      else
	{
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
	}

      PROCESS_INFORMATION pi;
      pi.hProcess = pi.hThread = NULL;
      pi.dwProcessId = pi.dwThreadId = 0;

      /* Set up needed handles for stdio */
      si.dwFlags = STARTF_USESTDHANDLES;
      si.hStdInput = handle ((in__stdin < 0 ? 0 : in__stdin), false);
      si.hStdOutput = handle ((in__stdout < 0 ? 1 : in__stdout), true);
      si.hStdError = handle (2, true);

      si.cb = sizeof (si);

      c_flags = GetPriorityClass (GetCurrentProcess ());
      sigproc_printf ("priority class %d", c_flags);

      c_flags |= CREATE_SEPARATE_WOW_VDM | CREATE_UNICODE_ENVIRONMENT;

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

      if (null_app_name)
	runpath = NULL;
      else
	{
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
      if (!iscygwin () && fhandler_console::exists ()
	  && fhandler_console::tc_getpgid () != myself->pgid)
	c_flags |= CREATE_NEW_PROCESS_GROUP;
      refresh_cygheap ();

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

    loop:
      /* When ruid != euid we create the new process under the current original
	 account and impersonate in child, this way maintaining the different
	 effective vs. real ids.
	 FIXME: If ruid != euid and ruid != saved_uid we currently give
	 up on ruid. The new process will have ruid == euid. */
      ::cygheap->user.deimpersonate ();

      if (!real_path.iscygexec () && mode == _P_OVERLAY)
	myself->process_state |= PID_NOTCYGWIN;

      wchar_t wcmd[(size_t) cmd];
      if (!::cygheap->user.issetuid ()
	  || (::cygheap->user.saved_uid == ::cygheap->user.real_uid
	      && ::cygheap->user.saved_gid == ::cygheap->user.real_gid
	      && !::cygheap->user.groups.issetgroups ()
	      && !::cygheap->user.setuid_to_restricted))
	{
	  rc = CreateProcessW (runpath,	  /* image name - with full path */
			       cmd.wcs (wcmd),/* what was passed to exec */
			       &sec_none_nih, /* process security attrs */
			       &sec_none_nih, /* thread security attrs */
			       TRUE,	  /* inherit handles from parent */
			       c_flags,
			       envblock,	  /* environment */
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
		 session First of all we might not have a valid logon session
		 for the user (logon by create_token), and second, it doesn't
		 make sense in terms of security to create a new window
		 station for every logon of the same user.  It just fills up
		 the system with window stations for no good reason. */
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
			       runpath,	  /* image name - with full path */
			       cmd.wcs (wcmd),/* what was passed to exec */
			       &sec_none_nih, /* process security attrs */
			       &sec_none_nih, /* thread security attrs */
			       TRUE,	  /* inherit handles from parent */
			       c_flags,
			       envblock,	  /* environment */
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

      if (mode != _P_OVERLAY)
	cygpid = cygwin_pid (pi.dwProcessId);
      else
	cygpid = myself->pid;

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
	  /* FIXME: This introduces an unreferenced, open handle into the child.
	     The purpose is to keep the pid shared memory open so that all of
	     the fields filled out by child.remember do not disappear and so
	     there is not a brief period during which the pid is not available.
	     However, we should try to find another way to do this eventually. */
	  DuplicateHandle (GetCurrentProcess (), child.shared_handle (),
			   pi.hProcess, NULL, 0, 0, DUPLICATE_SAME_ACCESS);
	  child->start_time = time (NULL); /* Register child's starting time. */
	  child->nice = myself->nice;
	  postfork (child);
	  if (!child.remember (mode == _P_DETACH))
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
	  myself.exit (EXITCODE_NOSET);
	  break;
	case _P_WAIT:
	case _P_SYSTEM:
	  system_call.arm ();
	  if (waitpid (cygpid, &res, 0) != cygpid)
	    res = -1;
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

  return spawnve (mode, path, (char * const  *) argv, cur_environ ());
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
		  (char * const *) argv, cur_environ ());
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
  return spawnve (mode, path, argv, cur_environ ());
}

extern "C" int
spawnvp (int mode, const char *file, const char * const *argv)
{
  path_conv buf;
  return spawnve (mode | _P_PATH_TYPE_EXEC, find_exec (file, buf), argv,
		  cur_environ ());
}

extern "C" int
spawnvpe (int mode, const char *file, const char * const *argv,
	  const char * const *envp)
{
  path_conv buf;
  return spawnve (mode | _P_PATH_TYPE_EXEC, find_exec (file, buf), argv, envp);
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
	    if (ascii_strcasematch (ext, ".com"))
	      break;
	    pgm = (char *) "/bin/sh";
	    arg1 = NULL;
	  }

	/* Check if script is executable.  Otherwise we start non-executable
	   scripts successfully, which is incorrect behaviour. */
	if (real_path.has_acls ()
	    && check_file_access (real_path, X_OK, true) < 0)
	  return -1;	/* errno is already set. */

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
