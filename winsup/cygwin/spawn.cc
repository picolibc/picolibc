/* spawn.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <process.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <wingdi.h>
#include <winuser.h>
#include <ctype.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "sync.h"
#include "sigproc.h"
#include "child_info.h"
#include "pinfo.h"
#include "cygheap.h"
#include "perthread.h"

#define LINE_BUF_CHUNK (MAX_PATH * 2)

suffix_info std_suffixes[] =
{
  suffix_info (".exe", 1), suffix_info ("", 1),
  suffix_info (".com"), suffix_info (".cmd"),
  suffix_info (".bat"), suffix_info (".dll"),
  suffix_info (NULL)
};

/* Add .exe to PROG if not already present and see if that exists.
   If not, return PROG (converted from posix to win32 rules if necessary).
   The result is always BUF.

   Returns (possibly NULL) suffix */

static const char *
perhaps_suffix (const char *prog, path_conv &buf)
{
  char *ext;

  debug_printf ("prog '%s'", prog);
  buf.check (prog, PC_SYM_FOLLOW | PC_FULL, std_suffixes);

  if (buf.file_attributes () & FILE_ATTRIBUTE_DIRECTORY)
    ext = NULL;
  else if (buf.known_suffix)
    ext = buf + (buf.known_suffix - buf.get_win32 ());
  else
    ext = strchr (buf, '\0');

  debug_printf ("buf %s, suffix found '%s'", (char *) buf, ext);
  return ext;
}

/* Find an executable name, possibly by appending known executable
   suffixes to it.  The win32-translated name is placed in 'buf'.
   Any found suffix is returned in known_suffix.

   If the file is not found and !null_if_not_found then the win32 version
   of name is placed in buf and returned.  Otherwise the contents of buf
   is undefined and NULL is returned.  */

const char * __stdcall
find_exec (const char *name, path_conv& buf, const char *mywinenv,
	   int null_if_notfound, const char **known_suffix)
{
  const char *suffix = "";
  debug_printf ("find_exec (%s)", name);
  char *retval = buf;

  /* Check to see if file can be opened as is first.
     Win32 systems always check . first, but PATH may not be set up to
     do this. */
  if ((suffix = perhaps_suffix (name, buf)) != NULL)
    goto out;

  win_env *winpath;
  const char *path;
  char tmp[MAX_PATH];

  /* Return the error condition if this is an absolute path or if there
     is no PATH to search. */
  if (strchr (name, '/') || strchr (name, '\\') ||
      isdrive (name) ||
      !(winpath = getwinenv (mywinenv)) ||
      !(path = winpath->get_native ()) ||
      *path == '\0')
    goto errout;

  debug_printf ("%s%s", mywinenv, path);

  /* Iterate over the specified path, looking for the file with and
     without executable extensions. */
  do
    {
      char *eotmp = strccpy (tmp, &path, ';');
      /* An empty path or '.' means the current directory, but we've
	 already tried that.  */
      if (tmp[0] == '\0' || (tmp[0] == '.' && tmp[1] == '\0'))
	continue;

      *eotmp++ = '\\';
      strcpy (eotmp, name);

      debug_printf ("trying %s", tmp);

      if ((suffix = perhaps_suffix (tmp, buf)) != NULL)
	goto out;
    }
  while (*path && *++path);

errout:
  /* Couldn't find anything in the given path.
     Take the appropriate action based on null_if_not_found. */
  if (null_if_notfound)
    retval = NULL;
  else
    buf.check (name);

out:
  debug_printf ("%s = find_exec (%s)", (char *) buf, name);
  if (known_suffix)
    *known_suffix = suffix ?: strchr (buf, '\0');
  return retval;
}

/* Utility for spawn_guts.  */

static HANDLE
handle (int n, int direction)
{
  fhandler_base *fh = fdtab[n];

  if (!fh)
    return INVALID_HANDLE_VALUE;
  if (fh->get_close_on_exec ())
    return INVALID_HANDLE_VALUE;
  if (direction == 0)
    return fh->get_handle ();
  return fh->get_output_handle ();
}

/* Cover function for CreateProcess.

   This function is used by both the routines that search $PATH and those
   that do not.  This should work out ok as according to the documentation,
   CreateProcess only searches $PATH if PROG has no directory elements.

   Spawning doesn't fit well with Posix's fork/exec (one can argue the merits
   of either but that's beside the point).  If we're exec'ing we want to
   record the child pid for fork.  If we're spawn'ing we don't want to do
   this.  It is up to the caller to handle both cases.

   The result is the process id.  The handle of the created process is
   stored in H.
*/

HANDLE NO_COPY hExeced = NULL;

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

class linebuf
{
public:
  size_t ix;
  char *buf;
  size_t alloced;
  linebuf () : ix (0), buf (NULL), alloced (0)
  {
  }
  ~linebuf () {/* if (buf) free (buf);*/}
  void add (const char *what, int len);
  void add (const char *what) {add (what, strlen (what));}
  void prepend (const char *what, int len);
};

void
linebuf::add (const char *what, int len)
{
  size_t newix;
  if ((newix = ix + len) >= alloced || !buf)
    {
      alloced += LINE_BUF_CHUNK + newix;
      buf = (char *) realloc (buf, alloced + 1);
    }
  memcpy (buf + ix, what, len);
  ix = newix;
  buf[ix] = '\0';
}

void
linebuf::prepend (const char *what, int len)
{
  int buflen;
  size_t newix;
  if ((newix = ix + len) >= alloced)
    {
      alloced += LINE_BUF_CHUNK + newix;
      buf = (char *) realloc (buf, alloced + 1);
      buf[ix] = '\0';
    }
  if ((buflen = strlen (buf)))
      memmove (buf + len, buf, buflen + 1);
  else
      buf[newix] = '\0';
  memcpy (buf, what, len);
  ix = newix;
}

static HANDLE hexec_proc = NULL;

void __stdcall
exec_fixup_after_fork ()
{
  if (hexec_proc)
    CloseHandle (hexec_proc);
  hexec_proc = NULL;
}

struct av
{
  int argc;
  int calloced;
private:
  char **argv;
public:
  av (int ac, const char * const *av) : argc (ac), calloced (0)
  {
    argv = (char **) cmalloc (HEAP_ARGV, (argc + 1) * sizeof (char *));
    memcpy (argv, av, (argc + 1) * sizeof (char *));
  }
  ~av ()
  {
    for (int i = 0; i < calloced; i++)
      cfree (argv[i]);
    cfree (argv);
  }
  int unshift (const char *what, int conv = 0);
  operator char **() {return argv;}
};

int
av::unshift (const char *what, int conv)
{
  char **av;
  av = (char **) crealloc (argv, (argc + 2) * sizeof (char *));
  if (!av)
    return 0;

  argv = av;
  memmove (argv + 1, argv, (argc + 1) * sizeof (char *));
  char buf[MAX_PATH + 1];
  if (conv)
    {
      cygwin_conv_to_posix_path (what, buf);
      char *p = strchr (buf, '\0') - 4;
      if (p > buf && strcasematch (p, ".exe"))
	*p = '\0';
      what = buf;
    }
  *argv = cstrdup (what);
  argc++;
  calloced++;
  return 1;
}

static int __stdcall
spawn_guts (HANDLE hToken, const char * prog_arg, const char *const *argv,
	    const char *const envp[], int mode)
{
  int i;
  BOOL rc;
  pid_t cygpid;

  hExeced = NULL;

  MALLOC_CHECK;

// if (strstr (prog_arg, "dopath")) try_to_debug ();
  if (prog_arg == NULL)
    {
      syscall_printf ("prog_arg is NULL");
      set_errno(EINVAL);
      return -1;
    }

  syscall_printf ("spawn_guts (%.132s)", prog_arg);

  if (argv == NULL)
    {
      syscall_printf ("argv is NULL");
      set_errno(EINVAL);
      return (-1);
    }

  path_conv real_path;

  linebuf one_line;

  STARTUPINFO si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL};

  child_info_spawn ciresrv;
  si.lpReserved2 = (LPBYTE) &ciresrv;
  si.cbReserved2 = sizeof (ciresrv);

  HANDLE spr = NULL;
  DWORD chtype;
  if (mode != _P_OVERLAY && mode != _P_VFORK)
    chtype = PROC_SPAWN;
  else
    {
      spr = CreateEvent(&sec_all, TRUE, FALSE, NULL);
      ProtectHandle (spr);
      chtype = PROC_EXEC;
    }

  init_child_info (chtype, &ciresrv, (mode == _P_OVERLAY) ? myself->pid : 1, spr);
  if (!DuplicateHandle (hMainProc, hMainProc, hMainProc, &ciresrv.parent, 0, 1,
			DUPLICATE_SAME_ACCESS))
     {
       system_printf ("couldn't create handle to myself for child, %E");
       return -1;
     }

  ciresrv.moreinfo = (cygheap_exec_info *) ccalloc (HEAP_EXEC, 1, sizeof (cygheap_exec_info));
  ciresrv.moreinfo->old_title = old_title ? cstrdup (old_title) : NULL;
  ciresrv.moreinfo->fds = fdtab;
  ciresrv.moreinfo->nfds = fdtab.size;

  /* CreateProcess takes one long string that is the command line (sigh).
     We need to quote any argument that has whitespace or embedded "'s.  */

  int ac;
  for (ac = 0; argv[ac]; ac++)
    /* nothing */;

  av newargv (ac, argv);

  if (ac == 3 && argv[1][0] == '/' && argv[1][1] == 'c' &&
      (iscmd (argv[0], "command.com") || iscmd (argv[0], "cmd.exe")))
    {
      one_line.add (argv[0]);
      one_line.add (" ");
      one_line.add (argv[1]);
      one_line.add (" ");
      one_line.add (argv[2]);
      strcpy (real_path, argv[0]);
      goto skip_arg_parsing;
    }

  const char *ext;
  if ((ext = perhaps_suffix (prog_arg, real_path)) == NULL)
    {
      set_errno (ENOENT);
      return -1;
    }

  MALLOC_CHECK;

  /* If the file name ends in either .exe, .com, .bat, or .cmd we assume
     that it is NOT a script file */
  while (*ext == '\0')
    {
      HANDLE hnd = CreateFileA (real_path,
				GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				&sec_none_nih,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0);
      if (hnd == INVALID_HANDLE_VALUE)
	{
	  __seterrno ();
	  return -1;
	}

      DWORD done;

      char buf[2 * MAX_PATH + 1];
      buf[0] = buf[1] = buf[2] = buf[sizeof(buf) - 1] = '\0';
      if (! ReadFile (hnd, buf, sizeof (buf) - 1, &done, 0))
	{
	  CloseHandle (hnd);
	  __seterrno ();
	  return -1;
	}

      CloseHandle (hnd);

      if (buf[0] == 'M' && buf[1] == 'Z')
	break;

      debug_printf ("%s is a script", prog_arg);

      char *ptr, *pgm, *arg1;

      if (buf[0] != '#' || buf[1] != '!')
	{
	  pgm = (char *) "/bin/sh";
	  ptr = buf + 2;
	  arg1 = NULL;
	}
      else
	{
	  pgm = buf + 2;
	  pgm += strspn (pgm, " \t");
	  for (ptr = pgm, arg1 = NULL;
	       *ptr && *ptr != '\r' && *ptr != '\n';
	       ptr++)
	    if (!arg1 && (*ptr == ' ' || *ptr == '\t'))
	      {
		/* Null terminate the initial command and step over
		   any additional white space.  If we've hit the
		   end of the line, exit the loop.  Otherwise, position
		   we've found the first argument. Position the current
		   pointer on the last known white space. */
		*ptr = '\0';
		char *newptr = ptr + 1;
		newptr += strspn (newptr, " \t");
		if (!*newptr || *newptr == '\r' || *newptr == '\n')
		  break;
		arg1 = newptr;
		ptr = newptr - 1;
	      }

	  *ptr = '\0';
	}

      /* pointers:
       * pgm	interpreter name
       * arg1	optional string
       * ptr	end of string
       */
      if (arg1)
	newargv.unshift (arg1);

      find_exec (pgm, real_path, "PATH=", 0, &ext);
      newargv.unshift (real_path, 1);
    }

  if (real_path.iscygexec ())
    for (int i = newargv.calloced; i < newargv.argc; i++)
      newargv[i] = cstrdup (newargv[i]);
  else
    {
      for (int i = 0; i < newargv.argc; i++)
	{
	  char *p = NULL;
	  const char *a;

	  if (i >= newargv.calloced)
	    newargv[i] = cstrdup (newargv[i]);
	  a = newargv[i];
	  int len = strlen (a);
	  if (len != 0 && !strpbrk (a, " \t\n\r\""))
	    one_line.add (a, len);
	  else
	    {
	      one_line.add ("\"", 1);
	      for (; (p = strpbrk (a, "\"\\")); a = ++p)
		{
		  one_line.add (a, p - a);
		  if (*p == '\\' || *p == '"')
		    one_line.add ("\\", 1);
		  one_line.add (p, 1);
		}
	      if (*a)
		one_line.add (a);
	      one_line.add ("\"", 1);
	    }
	  MALLOC_CHECK;
	  one_line.add (" ", 1);
	  MALLOC_CHECK;
	}

      MALLOC_CHECK;
      if (one_line.ix)
	one_line.buf[one_line.ix - 1] = '\0';
      else
	one_line.add ("", 1);
      MALLOC_CHECK;
    }
  ciresrv.moreinfo->argc = newargv.argc;
  ciresrv.moreinfo->argv = newargv;

  cygcwd.copy (ciresrv.moreinfo->cwd_posix, ciresrv.moreinfo->cwd_win32,
	       ciresrv.moreinfo->cwd_hash);

  ciresrv.moreinfo->environ = (char **) cmalloc (HEAP_ARGV, envsize (envp, 1));
  char **c;
  const char * const *e;
  for (c = ciresrv.moreinfo->environ, e = envp; *e;)
    *c++ = cstrdup (*e++);
  *c = NULL;
  if (mode != _P_OVERLAY ||
      !DuplicateHandle (hMainProc, myself.shared_handle (), hMainProc, &ciresrv.moreinfo->myself_pinfo, 0,
			TRUE, DUPLICATE_SAME_ACCESS))
    ciresrv.moreinfo->myself_pinfo = NULL;

skip_arg_parsing:
  PROCESS_INFORMATION pi = {NULL, 0, 0, 0};
  si.lpReserved = NULL;
  si.lpDesktop = NULL;
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = handle (0, 0); /* Get input handle */
  si.hStdOutput = handle (1, 1); /* Get output handle */
  si.hStdError = handle (2, 1); /* Get output handle */
  si.cb = sizeof (si);

  /* Pass fd table to a child */

  syscall_printf ("spawn_guts (%s, %.132s)", (char *) real_path, one_line.buf);

  int flags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED |
	      GetPriorityClass (hMainProc);

  if (mode == _P_DETACH || !set_console_state_for_spawn ())
    flags |= DETACHED_PROCESS;

  /* Build windows style environment list */
  char *envblock;
  if (real_path.iscygexec ())
    envblock = NULL;
  else
    envblock = winenv (envp, 0);

  ciresrv.cygheap = cygheap;
  ciresrv.cygheap_max = cygheap_max;

  /* Preallocated buffer for `sec_user' call */
  char sa_buf[1024];

  if (!hToken && myself->token != INVALID_HANDLE_VALUE)
    hToken = myself->token;

  /* FIXME:  This leaves a handle to the process open so that the pid is not
     duplicated.  However, if a process execs another process two handles are
     left open, which is unnecessary. */
  if (mode == _P_OVERLAY && !hexec_proc &&
      !DuplicateHandle (hMainProc, hMainProc, hMainProc, &hexec_proc, 0,
			TRUE, DUPLICATE_SAME_ACCESS))
    system_printf ("couldn't save current process handle %p, %E", hMainProc);

  if (hToken)
    {
      /* allow the child to interact with our window station/desktop */
      HANDLE hwst, hdsk;
      SECURITY_INFORMATION dsi = DACL_SECURITY_INFORMATION;
      DWORD n;
      char wstname[1024];
      char dskname[1024];

      hwst = GetProcessWindowStation();
      SetUserObjectSecurity(hwst, &dsi, get_null_sd ());
      GetUserObjectInformation(hwst, UOI_NAME, wstname, 1024, &n);
      hdsk = GetThreadDesktop(GetCurrentThreadId());
      SetUserObjectSecurity(hdsk, &dsi, get_null_sd ());
      GetUserObjectInformation(hdsk, UOI_NAME, dskname, 1024, &n);
      strcat (wstname, "\\");
      strcat (wstname, dskname);
      si.lpDesktop = wstname;

      char tu[1024];
      PSID sid = NULL;
      DWORD ret_len;
      if (GetTokenInformation (hToken, TokenUser,
			       (LPVOID) &tu, sizeof tu,
			       &ret_len))
	sid = ((TOKEN_USER *) &tu)->User.Sid;
      else
	system_printf ("GetTokenInformation: %E");

      /* Retrieve security attributes before setting psid to NULL
	 since it's value is needed by `sec_user'. */
      PSECURITY_ATTRIBUTES sec_attribs = allow_ntsec && sid
					 ? sec_user (sa_buf, sid)
					 : &sec_all_nih;

      /* Remove impersonation */
      uid_t uid = geteuid();
      if (myself->impersonated && myself->token != INVALID_HANDLE_VALUE)
	seteuid (myself->orig_uid);

      /* Load users registry hive. */
      load_registry_hive (sid);

      rc = CreateProcessAsUser (hToken,
		       real_path,	/* image name - with full path */
		       one_line.buf,	/* what was passed to exec */
		       sec_attribs,     /* process security attrs */
		       sec_attribs,     /* thread security attrs */
		       TRUE,	/* inherit handles from parent */
		       flags,
		       envblock,/* environment */
		       0,	/* use current drive/directory */
		       &si,
		       &pi);
      /* Restore impersonation. In case of _P_OVERLAY this isn't
	 allowed since it would overwrite child data. */
      if (mode != _P_OVERLAY && mode != _P_VFORK
	  && myself->impersonated && myself->token != INVALID_HANDLE_VALUE)
	seteuid (uid);
    }
  else
    rc = CreateProcessA (real_path,	/* image name - with full path */
		       one_line.buf,	/* what was passed to exec */
					/* process security attrs */
		       allow_ntsec ? sec_user (sa_buf) : &sec_all_nih,
					/* thread security attrs */
		       allow_ntsec ? sec_user (sa_buf) : &sec_all_nih,
		       TRUE,	/* inherit handles from parent */
		       flags,
		       envblock,/* environment */
		       0,	/* use current drive/directory */
		       &si,
		       &pi);

  MALLOC_CHECK;
  if (envblock)
    free (envblock);
  MALLOC_CHECK;

  /* Set errno now so that debugging messages from it appear before our
     final debugging message [this is a general rule for debugging
     messages].  */
  if (!rc)
    {
      if (spr)
	ForceCloseHandle (spr);
      __seterrno ();
      syscall_printf ("CreateProcess failed, %E");
      return -1;
    }

  if (mode == _P_OVERLAY)
    cygpid = myself->pid;
  else
    cygpid = cygwin_pid (pi.dwProcessId);

  /* We print the original program name here so the user can see that too.  */
  syscall_printf ("%d = spawn_guts (%s, %.132s)",
		  rc ? cygpid : (unsigned int) -1,
		  prog_arg, one_line.buf);

  MALLOC_CHECK;
  /* Name the handle similarly to proc_subproc. */
  ProtectHandle1 (pi.hProcess, childhProc);
  ProtectHandle (pi.hThread);
  MALLOC_CHECK;

  if (mode == _P_OVERLAY)
    {
      strcpy (myself->progname, real_path);
      // close_all_files ();
      proc_terminate ();
      hExeced = pi.hProcess;

      /* Set up child's signal handlers */
      /* CGF FIXME - consolidate with signal stuff below */
      for (i = 0; i < NSIG; i++)
	{
	  myself->getsig(i).sa_mask = 0;
	  if (myself->getsig(i).sa_handler != SIG_IGN || (mode != _P_OVERLAY))
	    myself->getsig(i).sa_handler = SIG_DFL;
	}
    }
  else
    {
      pinfo child (cygpid, 1);
      if (!child)
	{
	  set_errno (EAGAIN);
	  syscall_printf ("-1 = spawnve (), process table full");
	  return -1;
	}
      child->username[0] = '\0';
      child->progname[0] = '\0';
      child->ppid = myself->pid;
      child->uid = myself->uid;
      child->gid = myself->gid;
      child->pgid = myself->pgid;
      child->sid = myself->sid;
      child->ctty = myself->ctty;
      child->umask = myself->umask;
      child->process_state |= PID_INITIALIZING;
      if (myself->use_psid)
	{
	  child->use_psid = 1;
	  memcpy (child->psid, myself->psid, MAX_SID_LEN);
	}
      memcpy (child->logsrv, myself->logsrv, MAX_HOST_NAME);
      memcpy (child->domain, myself->domain, MAX_COMPUTERNAME_LENGTH+1);
      memcpy (child->root, myself->root, MAX_PATH+1);
      child->rootlen = myself->rootlen;
      child->dwProcessId = pi.dwProcessId;
      child->hProcess = pi.hProcess;
      for (i = 0; i < NSIG; i++)
	{
	  child->getsig(i).sa_mask = 0;
	  if (child->getsig(i).sa_handler != SIG_IGN || (mode != _P_OVERLAY))
	    child->getsig(i).sa_handler = SIG_DFL;
	}
      if (hToken)
	{
	  /* Set child->uid to USHRT_MAX to force calling internal_getlogin()
	     from child process. Clear username and psid to play it safe. */
	  child->uid = USHRT_MAX;
	  child->use_psid = 0;
	}
      child.remember ();
    }

  sigproc_printf ("spawned windows pid %d", pi.dwProcessId);
  /* Start the child running */
  ResumeThread (pi.hThread);
  ForceCloseHandle (pi.hThread);

  if (hToken && hToken != myself->token)
    CloseHandle (hToken);

  DWORD res;

  if (mode == _P_OVERLAY || mode == _P_VFORK)
    {
      BOOL exited;

      HANDLE waitbuf[3] = {pi.hProcess, signal_arrived, spr};
      int nwait = 3;

      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
      res = 0;
      exited = FALSE;
      MALLOC_CHECK;
      for (int i = 0; i < 100; i++)
	{
	  switch (WaitForMultipleObjects (nwait, waitbuf, FALSE, INFINITE))
	    {
	    case WAIT_OBJECT_0:
	      sigproc_printf ("subprocess exited");
	      if (!GetExitCodeProcess (pi.hProcess, &res))
		res = 1;
	      exited = TRUE;

	      if (nwait <= 2 || mode != _P_OVERLAY)
		/* nothing to do */;
	      else if (WaitForSingleObject (spr, 1) == WAIT_OBJECT_0)
		goto reparent;
	      else if (!(res & EXIT_REPARENTING))
		{
		  MALLOC_CHECK;
		  close_all_files ();
		  MALLOC_CHECK;
		}
	      break;
	    case WAIT_OBJECT_0 + 1:
	      sigproc_printf ("signal arrived");
	      ResetEvent (signal_arrived);
	      continue;
	    case WAIT_OBJECT_0 + 2:
	      if (mode == _P_OVERLAY)
		{
	      reparent:
		  res |= EXIT_REPARENTING;
		  close_all_files ();
		  if (!parent_alive)
		    {
		      nwait = 1;
		      sigproc_terminate ();
		      continue;
		    }
		}
	      break;
	    case WAIT_FAILED:
	      DWORD r;
	      system_printf ("wait failed: nwait %d, pid %d, winpid %d, %E",
			     nwait, myself->pid, myself->dwProcessId);
	      system_printf ("waitbuf[0] %p %d", waitbuf[0],
			     GetHandleInformation (waitbuf[0], &r));
	      system_printf ("waitbuf[1] %p = %d", waitbuf[1],
			     GetHandleInformation (waitbuf[1], &r));
	      set_errno (ECHILD);
	      return -1;
	    }
	  break;
	}

      ForceCloseHandle (spr);

      sigproc_printf ("res = %x", res);

      if (res & EXIT_REPARENTING)
	{
	  /* Try to reparent child process.
	   * Make handles to child available to parent process and exit with
	   * EXIT_REPARENTING status. Wait() syscall in parent will then wait
	   * for newly created child.
	   */
	  pinfo parent (myself->ppid);
	  if (!parent)
	    /* nothing */;
	  else
	    {
	      int rc;
	      HANDLE hP = OpenProcess (PROCESS_ALL_ACCESS, FALSE,
				       parent->dwProcessId);
	      sigproc_printf ("parent handle %p, pid %d", hP, parent->dwProcessId);
	      if (hP == NULL && GetLastError () == ERROR_INVALID_PARAMETER)
		rc = 1;
	      else if (hP)
		{
		  ProtectHandle (hP);
		  rc = DuplicateHandle (hMainProc, pi.hProcess, hP,
					&myself->hProcess, 0, FALSE,
					DUPLICATE_SAME_ACCESS);
		  sigproc_printf ("Dup hP %d", rc);
		  ForceCloseHandle (hP);
		}
	      if (!res)
		{
		  system_printf ("Reparent failed, parent handle %p, %E", hP);
		  system_printf ("my dwProcessId %d, myself->dwProcessId %d",
				 GetCurrentProcessId(), myself->dwProcessId);
		  system_printf ("myself->process_state %x",
				 myself->process_state);
		  system_printf ("myself->hProcess %x", myself->hProcess);
		}
	    }
	  ForceCloseHandle1 (hExeced, childhProc);
	  hExeced = INVALID_HANDLE_VALUE;
	}
      else if (exited)
	{
	  ForceCloseHandle1 (hExeced, childhProc);
	  hExeced = INVALID_HANDLE_VALUE; // stop do_exit from attempting to terminate child
	}

      MALLOC_CHECK;
      if (mode == _P_OVERLAY)
	do_exit (res | EXIT_NOCLOSEALL);
    }

  switch (mode)
    {
    case _P_WAIT:
      waitpid (cygpid, (int *) &res, 0);
      break;
    case _P_DETACH:
      res = 0;	/* Lose all memory of this child. */
      break;
    case _P_NOWAIT:
    case _P_NOWAITO:
    case _P_VFORK:
      res = cygpid;
      break;
    default:
      break;
    }

  return (int) res;
}

extern "C"
int
cwait (int *result, int pid, int)
{
  return waitpid (pid, result, 0);
}

/*
 * Helper function for spawn runtime calls.
 * Doesn't search the path.
 */

extern "C" int
_spawnve (HANDLE hToken, int mode, const char *path, const char *const *argv,
	  const char *const *envp)
{
  int ret;
  vfork_save *vf = vfork_storage.val ();

  if (vf != NULL && (vf->pid < 0) && mode == _P_OVERLAY)
    mode = _P_NOWAIT;
  else
    vf = NULL;

  syscall_printf ("_spawnve (%s, %s, %x)", path, argv[0], envp);

  switch (mode)
    {
    case _P_OVERLAY:
      /* We do not pass _P_SEARCH_PATH here. execve doesn't search PATH.*/
      /* Just act as an exec if _P_OVERLAY set. */
      spawn_guts (hToken, path, argv, envp, mode);
      /* Errno should be set by spawn_guts.  */
      ret = -1;
      break;
    case _P_VFORK:
    case _P_NOWAIT:
    case _P_NOWAITO:
    case _P_WAIT:
    case _P_DETACH:
      subproc_init ();
      ret = spawn_guts (hToken, path, argv, envp, 0);
      if (vf && ret > 0)
	{
	  vf->pid = ret;
	  longjmp (vf->j, 1);
	}
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

extern "C"
int
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

  return _spawnve (NULL, mode, path, (char * const  *) argv, cur_environ ());
}

extern "C"
int
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

  return _spawnve (NULL, mode, path, (char * const *) argv,
		   (char * const *) envp);
}

extern "C"
int
spawnlp (int mode, const char *path, const char *arg0, ...)
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

  return spawnvpe (mode, path, (char * const *) argv, cur_environ ());
}

extern "C"
int
spawnlpe (int mode, const char *path, const char *arg0, ...)
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

  return spawnvpe (mode, path, (char * const *) argv, envp);
}

extern "C"
int
spawnv (int mode, const char *path, const char * const *argv)
{
  return _spawnve (NULL, mode, path, argv, cur_environ ());
}

extern "C"
int
spawnve (int mode, const char *path, char * const *argv,
					     const char * const *envp)
{
  return _spawnve (NULL, mode, path, argv, envp);
}

extern "C"
int
spawnvp (int mode, const char *path, const char * const *argv)
{
  return spawnvpe (mode, path, argv, cur_environ ());
}

extern "C"
int
spawnvpe (int mode, const char *file, const char * const *argv,
					     const char * const *envp)
{
  path_conv buf;
  return _spawnve (NULL, mode, find_exec (file, buf), argv, envp);
}
