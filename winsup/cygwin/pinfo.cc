/* pinfo.cc: process table support

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygwin_version.h"
#include "perprocess.h"
#include "environ.h"
#include <assert.h>
#include <sys/wait.h>
#include <ntdef.h>
#include "ntdll.h"
#include "shared_info.h"
#include "cygheap.h"
#include "fhandler.h"
#include "cygmalloc.h"
#include "cygtls.h"
#include "child_info.h"

static char NO_COPY pinfo_dummy[sizeof (_pinfo)] = {0};

pinfo NO_COPY myself ((_pinfo *)&pinfo_dummy);	// Avoid myself != NULL checks

bool is_toplevel_proc;

/* Initialize the process table.
   This is done once when the dll is first loaded.  */

void __stdcall
set_myself (HANDLE h)
{
  if (!h)
    cygheap->pid = cygwin_pid (GetCurrentProcessId ());
  myself.init (cygheap->pid, PID_IN_USE, h ?: INVALID_HANDLE_VALUE);
  myself->process_state |= PID_IN_USE;
  myself->dwProcessId = GetCurrentProcessId ();

  (void) GetModuleFileName (NULL, myself->progname, sizeof (myself->progname));
  if (!strace.active)
    strace.hello ();
  debug_printf ("myself->dwProcessId %u", myself->dwProcessId);
  myself.initialize_lock ();
  if (h)
    {
      /* here if execed */
      static pinfo NO_COPY myself_identity;
      myself_identity.init (cygwin_pid (myself->dwProcessId), PID_EXECED, NULL);
      myself->start_time = time (NULL); /* Register our starting time. */
      myself->exec_sendsig = NULL;
      myself->exec_dwProcessId = 0;
    }
  else if (!myself->wr_proc_pipe)
    myself->start_time = time (NULL); /* Register our starting time. */
  else if (cygheap->pid_handle)
    {
      ForceCloseHandle (cygheap->pid_handle);
      cygheap->pid_handle = NULL;
    }
}

/* Initialize the process table entry for the current task.
   This is not called for forked tasks, only execed ones.  */
void __stdcall
pinfo_init (char **envp, int envc)
{
  if (envp)
    {
      environ_init (envp, envc);
      /* spawn has already set up a pid structure for us so we'll use that */
      myself->process_state |= PID_CYGPARENT;
    }
  else
    {
      /* Invent our own pid.  */

      set_myself (NULL);
      myself->ppid = 1;
      myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = ILLEGAL_UID;
      myself->gid = UNKNOWN_GID;
      environ_init (NULL, 0);	/* call after myself has been set up */
      myself->nice = winprio_to_nice (GetPriorityClass (hMainProc));
      debug_printf ("Set nice to %d", myself->nice);
    }

  debug_printf ("pid %d, pgid %d", myself->pid, myself->pgid);
}

# define self (*this)
void
pinfo::maybe_set_exit_code_from_windows ()
{
  DWORD x = 0xdeadbeef;
  DWORD oexitcode = self->exitcode;
  if (hProcess && !(self->exitcode & EXITCODE_SET))
    {
      WaitForSingleObject (hProcess, INFINITE);	// just to be safe, in case
      						// process hasn't quite exited
						// after closing pipe
      GetExitCodeProcess (hProcess, &x);
      self->exitcode = EXITCODE_SET | (x & 0xff) << 8;
    }
  sigproc_printf ("pid %d, exit value - old %p, windows %p, cygwin %p",
		  self->pid, oexitcode, x, self->exitcode);
}

void
pinfo::zap_cwd ()
{
  extern char windows_system_directory[];
  /* Move to an innocuous location to avoid a race with other processes
     that may want to manipulate the current directory before this
     process has completely exited.  */
  (void) SetCurrentDirectory (windows_system_directory);
}

void
pinfo::exit (DWORD n)
{
  exit_state = ES_FINAL;
  cygthread::terminate ();
  if (n != EXITCODE_NOSET)
    {
      sigproc_terminate ();		/* Just terminate signal and process stuff */
      self->exitcode = EXITCODE_SET | n;/* We're really exiting.  Record the UNIX exit code. */
    }

  /* FIXME:  There is a potential race between an execed process and its
     parent here.  I hated to add a mutex just for this, though.  */
  struct rusage r;
  fill_rusage (&r, hMainProc);
  add_rusage (&self->rusage_self, &r);

  maybe_set_exit_code_from_windows ();

  if (n != EXITCODE_NOSET)
    {
      zap_cwd ();
      self->alert_parent (0);		/* Shave a little time by telling our
					   parent that we have now exited.  */
    }
  int exitcode = self->exitcode;
  release ();

  _my_tls.stacklock = 0;
  _my_tls.stackptr = _my_tls.stack;
  sigproc_printf ("Calling ExitProcess hProcess %p, n %p, exitcode %p",
		  hProcess, n, exitcode);
  ExitProcess (exitcode & 0xffff);
}
# undef self

void
pinfo::init (pid_t n, DWORD flag, HANDLE h0)
{
  h = NULL;
  if (myself && n == myself->pid)
    {
      procinfo = myself;
      destroy = 0;
      return;
    }

  void *mapaddr;
  bool createit = !!(flag & (PID_IN_USE | PID_EXECED));
  bool created = false;
  DWORD access = FILE_MAP_READ
		 | (flag & (PID_IN_USE | PID_EXECED | PID_MAP_RW)
		    ? FILE_MAP_WRITE : 0);
  if (!h0)
    mapaddr = NULL;
  else
    {
      /* Try to enforce that myself is always created in the same place */
      mapaddr = open_shared (NULL, 0, h0, 0, SH_MYSELF);
      if (h0 == INVALID_HANDLE_VALUE)
	h0 = NULL;
    }

  procinfo = NULL;
  for (int i = 0; i < 20; i++)
    {
      if (!h0)
	{
	  char mapname[CYG_MAX_PATH];
	  shared_name (mapname, "cygpid", n);

	  int mapsize;
	  if (flag & PID_EXECED)
	    mapsize = PINFO_REDIR_SIZE;
	  else
	    mapsize = sizeof (_pinfo);

	  if (!createit)
	    {
	      h0 = OpenFileMapping (access, FALSE, mapname);
	      created = false;
	    }
	  else
	    {
	      char sa_buf[1024];
	      PSECURITY_ATTRIBUTES sec_attribs =
		sec_user_nih (sa_buf, cygheap->user.sid(), well_known_world_sid,
			      FILE_MAP_READ);
	      h0 = CreateFileMapping (INVALID_HANDLE_VALUE, sec_attribs,
				      PAGE_READWRITE, 0, mapsize, mapname);
	      created = GetLastError () != ERROR_ALREADY_EXISTS;
	    }

	  if (!h0)
	    {
	      if (createit)
		__seterrno ();
	      return;
	    }
	}

      procinfo = (_pinfo *) MapViewOfFileEx (h0, access, 0, 0, 0, mapaddr);

      if (!procinfo)
	{
	  if (exit_state)
	    return;

	  if (GetLastError () == ERROR_INVALID_HANDLE)
	    api_fatal ("MapViewOfFileEx h0 %p, i %d failed, %E", h0, i);

	  debug_printf ("MapViewOfFileEx h0 %p, i %d failed, %E", h0, i);
	  continue;
	}

      if ((procinfo->process_state & PID_INITIALIZING) && (flag & PID_NOREDIR)
	  && cygwin_pid (procinfo->dwProcessId) != procinfo->pid)
	{
	  set_errno (ESRCH);
	  break;
	}

      if (procinfo->process_state & PID_EXECED)
	{
	  assert (!i);
	  pid_t realpid = procinfo->pid;
	  debug_printf ("execed process windows pid %d, cygwin pid %d", n, realpid);
	  if (realpid == n)
	    api_fatal ("retrieval of execed process info for pid %d failed due to recursion.", n);

	  if ((flag & PID_ALLPIDS))
	    {
	      set_errno (ESRCH);
	      break;
	    }
	  n = realpid;
	  CloseHandle (h0);
	  h0 = NULL;
	  goto loop;
	}

	/* In certain rare cases, it is possible for the shared memory region to
	   exist for a while after a process has exited.  This should only be a
	   brief occurrence, so rather than introduce some kind of locking
	   mechanism, just loop.  */
      if (!created && createit && (procinfo->process_state & PID_EXITED))
	goto loop;

      if (!created)
	/* nothing */;
      else if (!(flag & PID_EXECED))
	procinfo->pid = n;
      else
	{
	  procinfo->process_state |= PID_IN_USE | PID_EXECED;
	  procinfo->pid = myself->pid;
	}

      h = h0;	/* Success! */
      break;

    loop:
      release ();
      low_priority_sleep (0);
    }

  if (h)
    {
      destroy = 1;
      ProtectHandle1 (h, pinfo_shared_handle);
    }
  else
    {
      h = h0;
      release ();
    }
}

void
pinfo::set_acl()
{
  char sa_buf[1024];
  SECURITY_DESCRIPTOR sd;

  sec_acl ((PACL) sa_buf, true, true, cygheap->user.sid (),
	   well_known_world_sid, FILE_MAP_READ);
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    debug_printf ("InitializeSecurityDescriptor %E");
  else if (!SetSecurityDescriptorDacl (&sd, TRUE, (PACL) sa_buf, FALSE))
    debug_printf ("SetSecurityDescriptorDacl %E");
  else if (!SetKernelObjectSecurity (h, DACL_SECURITY_INFORMATION, &sd))
    debug_printf ("SetKernelObjectSecurity %E");
}

void
_pinfo::set_ctty (tty_min *tc, int flags, fhandler_tty_slave *arch)
{
  debug_printf ("checking if /dev/tty%d changed", ctty);
  if ((ctty < 0 || ctty == tc->ntty) && !(flags & O_NOCTTY))
    {
      ctty = tc->ntty;
      syscall_printf ("attached tty%d sid %d, pid %d, tty->pgid %d, tty->sid %d",
		      tc->ntty, sid, pid, pgid, tc->getsid ());

      pinfo p (tc->getsid ());
      if (sid == pid && (!p || p->pid == pid || !proc_exists (p)))
	{
	  paranoid_printf ("resetting tty%d sid.  Was %d, now %d.  pgid was %d, now %d.",
			   tc->ntty, tc->getsid (), sid, tc->getpgid (), pgid);
	  /* We are the session leader */
	  tc->setsid (sid);
	  tc->setpgid (pgid);
	}
      else
	sid = tc->getsid ();
      if (tc->getpgid () == 0)
	tc->setpgid (pgid);
      if (cygheap->ctty != arch)
	{
	  debug_printf ("cygheap->ctty %p, arch %p", cygheap->ctty, arch);
	  if (!cygheap->ctty)
	    syscall_printf ("ctty NULL");
	  else
	    {
	      syscall_printf ("ctty %p, usecount %d", cygheap->ctty,
			      cygheap->ctty->usecount);
	      cygheap->ctty->close ();
	    }
	  cygheap->ctty = arch;
	  if (arch)
	    {
	      arch->usecount++;
	      cygheap->open_fhs++;
	      report_tty_counts (cygheap->ctty, "ctty", "incremented ", "");
	    }
	}
    }
}

bool
_pinfo::alive ()
{
  HANDLE h = OpenProcess (PROCESS_QUERY_INFORMATION, false, dwProcessId);
  if (h)
    CloseHandle (h);
  return !!h;
}

extern char **__argv;

void
_pinfo::commune_recv ()
{
  char path[CYG_MAX_PATH + 1];
  DWORD nr;
  DWORD code;
  HANDLE hp;
  HANDLE __fromthem = NULL;
  HANDLE __tothem = NULL;

  hp = OpenProcess (PROCESS_DUP_HANDLE, false, dwProcessId);
  if (!hp)
    {
      sigproc_printf ("couldn't open handle for pid %d(%u)", pid, dwProcessId);
      hello_pid = -1;
      return;
    }
  if (!DuplicateHandle (hp, fromthem, hMainProc, &__fromthem, 0, false, DUPLICATE_SAME_ACCESS))
    {
      sigproc_printf ("couldn't duplicate fromthem, %E");
      CloseHandle (hp);
      hello_pid = -1;
      return;
    }

  if (!DuplicateHandle (hp, tothem, hMainProc, &__tothem, 0, false, DUPLICATE_SAME_ACCESS))
    {
      sigproc_printf ("couldn't duplicate tothem, %E");
      CloseHandle (__fromthem);
      CloseHandle (hp);
      hello_pid = -1;
      return;
    }

  hello_pid = 0;

  if (!ReadFile (__fromthem, &code, sizeof code, &nr, NULL) || nr != sizeof code)
    {
      CloseHandle (hp);
      /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
      goto out;
    }

  switch (code)
    {
    case PICOM_CMDLINE:
      {
	unsigned n = 1;
	CloseHandle (__fromthem); __fromthem = NULL;
	extern int __argc_safe;
	const char *argv[__argc_safe + 1];

	CloseHandle (hp);
	for (int i = 0; i < __argc_safe; i++)
	  {
	    if (IsBadStringPtr (__argv[i], INT32_MAX))
	      argv[i] = "";
	    else
	      argv[i] = __argv[i];
	    n += strlen (argv[i]) + 1;
	  }
	argv[__argc_safe] = NULL;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WriteFile sizeof argv failed, %E");
	  }
	else
	  for (const char **a = argv; *a; a++)
	    if (!WriteFile (__tothem, *a, strlen (*a) + 1, &nr, NULL))
	      {
		sigproc_printf ("WriteFile arg %d failed, %E", a - argv);
		break;
	      }
	if (!WriteFile (__tothem, "", 1, &nr, NULL))
	  {
	    sigproc_printf ("WriteFile null failed, %E");
	    break;
	  }
	break;
      }
    case PICOM_CWD:
      {
        CloseHandle (__fromthem); __fromthem = NULL;
	CloseHandle (hp);
	unsigned int n = strlen (cygheap->cwd.get (path, 1, 1,
						   CYG_MAX_PATH)) + 1;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  sigproc_printf ("WriteFile sizeof cwd failed, %E");
	else if (!WriteFile (__tothem, path, n, &nr, NULL))
	  sigproc_printf ("WriteFile cwd failed, %E");
	break;
      }
    case PICOM_ROOT:
      {
        CloseHandle (__fromthem); __fromthem = NULL;
	CloseHandle (hp);
	unsigned int n;
	if (cygheap->root.exists ())
	  n = strlen (strcpy (path, cygheap->root.posix_path ())) + 1;
	else
	  n = strlen (strcpy (path, "/")) + 1;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  sigproc_printf ("WriteFile sizeof root failed, %E");
	else if (!WriteFile (__tothem, path, n, &nr, NULL))
	  sigproc_printf ("WriteFile root failed, %E");
	break;
      }
    case PICOM_FDS:
      {
        CloseHandle (__fromthem); __fromthem = NULL;
	CloseHandle (hp);
	unsigned int n = 0;
	int fd;
	cygheap_fdenum cfd;
	while ((fd = cfd.next ()) >= 0)
	  n += sizeof (int);
	cfd.rewind ();
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  sigproc_printf ("WriteFile sizeof fds failed, %E");
	else
	  while ((fd = cfd.next ()) >= 0)
	    if (!WriteFile (__tothem, &fd, sizeof fd, &nr, NULL))
	      {
		sigproc_printf ("WriteFile fd %d failed, %E", fd);
		break;
	      }
        break;
      }
    case PICOM_PIPE_FHANDLER:
	{
	  HANDLE hdl;
	  if (!ReadFile (__fromthem, &hdl, sizeof hdl, &nr, NULL)
	      || nr != sizeof hdl)
	    {
	      sigproc_printf ("ReadFile hdl failed, %E");
	      CloseHandle (hp);
	      goto out;
	    }
	  CloseHandle (__fromthem); __fromthem = NULL;
	  CloseHandle (hp);
	  unsigned int n = 0;
	  cygheap_fdenum cfd;
	  while (cfd.next () >= 0)
	    if (cfd->get_handle () == hdl)
	      {
		fhandler_pipe *fh = cfd;
		n = sizeof *fh;
		if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
		  sigproc_printf ("WriteFile sizeof hdl failed, %E");
		else if (!WriteFile (__tothem, fh, n, &nr, NULL))
		  sigproc_printf ("WriteFile hdl failed, %E");
	      }
	  if (!n && !WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	    sigproc_printf ("WriteFile sizeof hdl failed, %E");
	  break;
	}
    case PICOM_FD:
      {
	int fd;
	if (!ReadFile (__fromthem, &fd, sizeof fd, &nr, NULL)
	    || nr != sizeof fd)
	  {
	    sigproc_printf ("ReadFile fd failed, %E");
	    CloseHandle (hp);
	    goto out;
	  }
	CloseHandle (__fromthem); __fromthem = NULL;
	CloseHandle (hp);
	unsigned int n = 0;
	cygheap_fdget cfd (fd);
	if (cfd < 0)
	  n = strlen (strcpy (path, "")) + 1;
	else
	  n = strlen (cfd->get_proc_fd_name (path)) + 1;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  sigproc_printf ("WriteFile sizeof fd failed, %E");
	else if (!WriteFile (__tothem, path, n, &nr, NULL))
	  sigproc_printf ("WriteFile fd failed, %E");
        break;
      }
    case PICOM_FIFO:
      {
	unsigned len;
	if (!ReadFile (__fromthem, &len, sizeof len, &nr, NULL)
	    || nr != sizeof len)
	  {
	    CloseHandle (hp);
	    /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
	    goto out;
	  }
	/* Get null-terminated path */
	if (!ReadFile (__fromthem, path, len, &nr, NULL)
	    || nr != len)
	  {
	    CloseHandle (hp);
	    /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
	    goto out;
	  }

	fhandler_fifo *fh = cygheap->fdtab.find_fifo (path);
	HANDLE it[2];
	if (fh == NULL)
	  it[0] = it[1] = NULL;
	else
	  {
	    it[0] = fh->get_handle ();
	    it[1] = fh->get_output_handle ();
	    for (int i = 0; i < 2; i++)
	      if (!DuplicateHandle (hMainProc, it[i], hp, &it[i], 0, false,
				    DUPLICATE_SAME_ACCESS))
		{
		  it[0] = it[1] = NULL;	/* FIXME: possibly left a handle open in child? */
		  break;
		}
	  }

	CloseHandle (hp);
	if (!WriteFile (__tothem, it, sizeof (it), &nr, NULL))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WriteFile read handle failed, %E");
	  }

	(void) ReadFile (__fromthem, &nr, sizeof (nr), &nr, NULL);
	break;
      }
    }

out:
  if (__fromthem)
    CloseHandle (__fromthem);
  if (__tothem)
    CloseHandle (__tothem);
}

#define PIPEBUFSIZE (4096 * sizeof (DWORD))

commune_result
_pinfo::commune_send (DWORD code, ...)
{
  HANDLE fromthem = NULL, tome = NULL;
  HANDLE fromme = NULL, tothem = NULL;
  DWORD nr;
  commune_result res;
  va_list args;

  va_start (args, code);

  res.s = NULL;
  res.n = 0;

  if (!this || !pid)
    {
      set_errno (ESRCH);
      goto err;
    }
  if (!CreatePipe (&fromthem, &tome, &sec_all_nih, PIPEBUFSIZE))
    {
      sigproc_printf ("first CreatePipe failed, %E");
      __seterrno ();
      goto err;
    }
  if (!CreatePipe (&fromme, &tothem, &sec_all_nih, PIPEBUFSIZE))
    {
      sigproc_printf ("second CreatePipe failed, %E");
      __seterrno ();
      goto err;
    }
  myself.lock ();
  myself->tothem = tome;
  myself->fromthem = fromme;
  myself->hello_pid = pid;
  if (!WriteFile (tothem, &code, sizeof code, &nr, NULL) || nr != sizeof code)
    {
      __seterrno ();
      goto err;
    }

  if (sig_send (this, __SIGCOMMUNE))
    goto err;

  /* FIXME: Need something better than an busy loop here */
  bool isalive;
  for (int i = 0; (isalive = alive ()) && (i < 10000); i++)
    if (myself->hello_pid <= 0)
      break;
    else
      low_priority_sleep (0);

  CloseHandle (tome);
  tome = NULL;
  CloseHandle (fromme);
  fromme = NULL;

  if (!isalive)
    {
      set_errno (ESRCH);
      goto err;
    }

  if (myself->hello_pid < 0)
    {
      set_errno (ENOSYS);
      goto err;
    }

  size_t n;
  switch (code)
    {
    case PICOM_PIPE_FHANDLER:
      {
	HANDLE hdl = va_arg (args, HANDLE);
	if (!WriteFile (tothem, &hdl, sizeof hdl, &nr, NULL)
	    || nr != sizeof hdl)
	  {
	    __seterrno ();
	    goto err;
	  }
      }
      goto business_as_usual;
    case PICOM_FD:
      {
	int fd = va_arg (args, int);
	if (!WriteFile (tothem, &fd, sizeof fd, &nr, NULL)
	    || nr != sizeof fd)
	  {
	    __seterrno ();
	    goto err;
	  }
      }
      goto business_as_usual;
    case PICOM_CMDLINE:
    case PICOM_CWD:
    case PICOM_ROOT:
    case PICOM_FDS:
  business_as_usual:
      if (!ReadFile (fromthem, &n, sizeof n, &nr, NULL) || nr != sizeof n)
	{
	  __seterrno ();
	  goto err;
	}
      if (!n)
        res.s = NULL;
      else
        {
	  res.s = (char *) malloc (n);
	  char *p;
	  for (p = res.s; ReadFile (fromthem, p, n, &nr, NULL); p += nr)
	    continue;
	  if ((unsigned) (p - res.s) != n)
	    {
	      __seterrno ();
	      goto err;
	    }
        }
      res.n = n;
      break;
    case PICOM_FIFO:
      {
	char *path = va_arg (args, char *);
	size_t len = strlen (path) + 1;
	if (!WriteFile (tothem, &len, sizeof (len), &nr, NULL)
	    || nr != sizeof (len))
	  {
	    __seterrno ();
	    goto err;
	  }
	if (!WriteFile (tothem, path, len, &nr, NULL) || nr != len)
	  {
	    __seterrno ();
	    goto err;
	  }

	DWORD x = ReadFile (fromthem, res.handles, sizeof (res.handles), &nr, NULL);
	(void) WriteFile (tothem, &x, sizeof (x), &x, NULL);
	if (!x)
	  goto err;

	if (nr != sizeof (res.handles))
	  {
	    set_errno (EPIPE);
	    goto err;
	  }
	break;
      }
    }
  CloseHandle (tothem);
  CloseHandle (fromthem);
  goto out;

err:
  if (tome)
    CloseHandle (tome);
  if (fromthem)
    CloseHandle (fromthem);
  if (tothem)
    CloseHandle (tothem);
  if (fromme)
    CloseHandle (fromme);
  memset (&res, 0, sizeof (res));

out:
  myself->hello_pid = 0;
  myself.unlock ();
  return res;
}

fhandler_pipe *
_pinfo::pipe_fhandler (HANDLE hdl, size_t &n)
{
  if (!this || !pid)
    return NULL;
  if (pid == myself->pid)
    return NULL;
  commune_result cr = commune_send (PICOM_PIPE_FHANDLER, hdl);
  n = cr.n;
  return (fhandler_pipe *) cr.s;
}

char *
_pinfo::fd (int fd, size_t &n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_FD, fd);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	s = strdup ("");
      else
	s = cfd->get_proc_fd_name ((char *) malloc (CYG_MAX_PATH + 1));
      n = strlen (s) + 1;
    }
  return s;
}

char *
_pinfo::fds (size_t &n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_FDS);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      n = 0;
      int fd;
      cygheap_fdenum cfd;
      while ((fd = cfd.next ()) >= 0)
	n += sizeof (int);
      cfd.rewind ();
      s = (char *) malloc (n);
      int *p = (int *) s;
      while ((fd = cfd.next ()) >= 0 && (char *) p - s < (int) n)
	*p++ = fd;
    }
  return s;
}

char *
_pinfo::root (size_t& n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_ROOT);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      if (cygheap->root.exists ())
	s = strdup (cygheap->root.posix_path ());
      else
	s = strdup ("/");
      n = strlen (s) + 1;
    }
  return s;
}

char *
_pinfo::cwd (size_t& n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_CWD);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      s = (char *) malloc (CYG_MAX_PATH);
      cygheap->cwd.get (s, 1, 1, CYG_MAX_PATH);
      n = strlen (s) + 1;
    }
  return s;
}

char *
_pinfo::cmdline (size_t& n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_CMDLINE);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      n = 1;
      for (char **a = __argv; *a; a++)
	n += strlen (*a) + 1;
      char *p;
      p = s = (char *) malloc (n);
      for (char **a = __argv; *a; a++)
	{
	  strcpy (p, *a);
	  p = strchr (p, '\0') + 1;
	}
      *p = '\0';
    }
  return s;
}

/* This is the workhorse which waits for the write end of the pipe
   created during new process creation.  If the pipe is closed or a zero
   is received on the pipe, it is assumed that the cygwin pid has exited.
   Otherwise, various "signals" can be sent to the parent to inform the
   parent to perform a certain action. */
static DWORD WINAPI
proc_waiter (void *arg)
{
  pinfo vchild = *(pinfo *) arg;
  ((pinfo *) arg)->waiter_ready = true;

  siginfo_t si;
  si.si_signo = SIGCHLD;
  si.si_code = SI_KERNEL;
  si.si_pid = vchild->pid;
  si.si_errno = 0;
#if 0	// FIXME: This is tricky to get right
  si.si_utime = pchildren[rc]->rusage_self.ru_utime;
  si.si_stime = pchildren[rc].rusage_self.ru_stime;
#else
  si.si_utime = 0;
  si.si_stime = 0;
#endif
  pid_t pid = vchild->pid;

  for (;;)
    {
      DWORD nb;
      char buf = '\0';
      extern HANDLE hExeced;

      if (!ReadFile (vchild.rd_proc_pipe, &buf, 1, &nb, NULL)
	  && GetLastError () != ERROR_BROKEN_PIPE)
	{
	  system_printf ("error on read of child wait pipe %p, %E", vchild.rd_proc_pipe);
	  break;
	}

      si.si_uid = vchild->uid;

      switch (buf)
	{
	case __ALERT_ALIVE:
	  continue;
	case 0:
	  /* Child exited.  Do some cleanup and signal myself.  */
	  CloseHandle (vchild.rd_proc_pipe);
	  vchild.rd_proc_pipe = NULL;
	  vchild.maybe_set_exit_code_from_windows ();
	  if (WIFEXITED (vchild->exitcode))
	    si.si_sigval.sival_int = CLD_EXITED;
	  else if (WCOREDUMP (vchild->exitcode))
	    si.si_sigval.sival_int = CLD_DUMPED;
	  else
	    si.si_sigval.sival_int = CLD_KILLED;
	  si.si_status = vchild->exitcode;
	  vchild->process_state = PID_EXITED;
	  /* This should always be last.  Do not use vchild-> beyond this point */
	  break;
	case SIGTTIN:
	case SIGTTOU:
	case SIGTSTP:
	case SIGSTOP:
	  if (ISSTATE (myself, PID_NOCLDSTOP))	// FIXME: No need for this flag to be in _pinfo any longer
	    continue;
	  /* Child stopped.  Signal myself.  */
	  si.si_sigval.sival_int = CLD_STOPPED;
	  break;
	case SIGCONT:
	  continue;
	default:
	  system_printf ("unknown value %d on proc pipe", buf);
	  continue;
	}

      /* Special case:  If the "child process" that died is us, then we're
	 execing.  Just call proc_subproc directly and then exit this loop.
	 We're done here.  */
      if (hExeced)
	{
	  /* execing.  no signals available now. */
	  proc_subproc (PROC_CLEARWAIT, 0);
	  break;
	}

      /* Send a SIGCHLD to myself.   We do this here, rather than in proc_subproc
	 to avoid the proc_subproc lock since the signal thread will eventually
	 be calling proc_subproc and could unnecessarily block. */
      sig_send (myself_nowait, si);

      /* If we're just stopped or got a continue signal, keep looping.
	 Otherwise, return this thread to the pool. */
      if (buf != '\0')
	sigproc_printf ("looping");
      else
	break;
    }

  sigproc_printf ("exiting wait thread for pid %d", pid);
  vchild.wait_thread = NULL;
  _my_tls._ctinfo->auto_release ();	/* automatically return the cygthread to the cygthread pool */
  return 0;
}

bool
_pinfo::dup_proc_pipe (HANDLE hProcess)
{
  DWORD flags = DUPLICATE_SAME_ACCESS;
  /* Grr.  Can't set DUPLICATE_CLOSE_SOURCE for exec case because we could be
     execing a non-cygwin process and we need to set the exit value before the
     parent sees it.  */
  if (this != myself || is_toplevel_proc)
    flags |= DUPLICATE_CLOSE_SOURCE;
  bool res = DuplicateHandle (hMainProc, wr_proc_pipe, hProcess, &wr_proc_pipe,
			      0, FALSE, flags);
  if (!res)
    sigproc_printf ("DuplicateHandle failed, pid %d, hProcess %p, %E", pid, hProcess);
  else
    {
      wr_proc_pipe_owner = dwProcessId;
      sigproc_printf ("closed wr_proc_pipe %p for pid %d(%u)", wr_proc_pipe,
		      pid, dwProcessId);
    }
  return res;
}

/* function to set up the process pipe and kick off proc_waiter */
int
pinfo::wait ()
{
  /* FIXME: execed processes should be able to wait for pids that were started
     by the process which execed them. */
  if (!CreatePipe (&rd_proc_pipe, &((*this)->wr_proc_pipe), &sec_none_nih, 16))
    {
      system_printf ("Couldn't create pipe tracker for pid %d, %E",
		     (*this)->pid);
      return 0;
    }

  if (!(*this)->dup_proc_pipe (hProcess))
    {
      system_printf ("Couldn't duplicate pipe topid %d(%p), %E", (*this)->pid, hProcess);
      return 0;
    }

  preserve ();		/* Preserve the shared memory associated with the pinfo */

  waiter_ready = false;
  /* Fire up a new thread to track the subprocess */
  cygthread *h = new cygthread (proc_waiter, this, "proc_waiter");
  if (!h)
    sigproc_printf ("tracking thread creation failed for pid %d", (*this)->pid);
  else
    {
      wait_thread = h;
      sigproc_printf ("created tracking thread for pid %d, winpid %p, rd_pipe %p",
		      (*this)->pid, (*this)->dwProcessId, rd_proc_pipe);
    }

  return 1;
}

void
_pinfo::sync_proc_pipe ()
{
  if (wr_proc_pipe && wr_proc_pipe != INVALID_HANDLE_VALUE)
    while (wr_proc_pipe_owner != GetCurrentProcessId ())
      low_priority_sleep (0);
}

/* function to send a "signal" to the parent when something interesting happens
   in the child. */
bool
_pinfo::alert_parent (char sig)
{
  DWORD nb = 0;
  /* Send something to our parent.  If the parent has gone away,
     close the pipe. */
  if (wr_proc_pipe == INVALID_HANDLE_VALUE
      || !myself->wr_proc_pipe)
    /* no parent */;
  else
    {
      sync_proc_pipe ();
      if (WriteFile (wr_proc_pipe, &sig, 1, &nb, NULL))
	/* all is well */;
      else if (GetLastError () != ERROR_BROKEN_PIPE)
	debug_printf ("sending %d notification to parent failed, %E", sig);
      else
	{
	  ppid = 1;
	  HANDLE closeit = wr_proc_pipe;
	  wr_proc_pipe = INVALID_HANDLE_VALUE;
	  CloseHandle (closeit);
	}
    }
  return (bool) nb;
}

void
pinfo::release ()
{
  if (procinfo)
    {
#ifdef DEBUGGING
      if (((DWORD) procinfo & 0x77000000) == 0x61000000)
	try_to_debug ();
#endif
      UnmapViewOfFile (procinfo);
      procinfo = NULL;
    }
  if (h)
    {
      ForceCloseHandle1 (h, pinfo_shared_handle);
      h = NULL;
    }
}

/* DOCTOOL-START

<sect1 id="func-cygwin-winpid-to-pid">
  <title>cygwin_winpid_to_pid</title>

  <funcsynopsis><funcprototype>
    <funcdef>extern "C" pid_t
      <function>cygwin_winpid_to_pid</function>
      </funcdef>
      <paramdef>int <parameter>winpid</parameter></paramdef>
  </funcprototype></funcsynopsis>

  <para>Given a windows pid, converts to the corresponding Cygwin
pid, if any.  Returns -1 if windows pid does not correspond to
a cygwin pid.</para>
  <example>
    <title>Example use of cygwin_winpid_to_pid</title>
    <programlisting>
      extern "C" cygwin_winpid_to_pid (int winpid);
      pid_t mypid;
      mypid = cygwin_winpid_to_pid (windows_pid);
    </programlisting>
  </example>
</sect1>

   DOCTOOL-END */

extern "C" pid_t
cygwin_winpid_to_pid (int winpid)
{
  pinfo p (cygwin_pid (winpid));
  if (p)
    return p->pid;

  set_errno (ESRCH);
  return (pid_t) -1;
}

#include <tlhelp32.h>

#define slop_pidlist 200
#define size_pidlist(i) (sizeof (pidlist[0]) * ((i) + 1))
#define size_pinfolist(i) (sizeof (pinfolist[0]) * ((i) + 1))

inline void
winpids::add (DWORD& nelem, bool winpid, DWORD pid)
{
  pid_t cygpid = cygwin_pid (pid);
  if (nelem >= npidlist)
    {
      npidlist += slop_pidlist;
      pidlist = (DWORD *) realloc (pidlist, size_pidlist (npidlist + 1));
      pinfolist = (pinfo *) realloc (pinfolist, size_pinfolist (npidlist + 1));
    }

  pinfolist[nelem].init (cygpid, PID_NOREDIR | (winpid ? PID_ALLPIDS : 0)
			 | pinfo_access, NULL);
  if (winpid)
    goto out;

  if (!pinfolist[nelem])
    {
      if (!pinfo_access)
	return;
      pinfolist[nelem].init (cygpid, PID_NOREDIR | (winpid ? PID_ALLPIDS : 0), NULL);
      if (!pinfolist[nelem])
	return;
      }

  /* Scan list of previously recorded pids to make sure that this pid hasn't
     shown up before.  This can happen when a process execs. */
  for (unsigned i = 0; i < nelem; i++)
    if (pinfolist[i]->pid == pinfolist[nelem]->pid)
      {
	if ((_pinfo *) pinfolist[nelem] != (_pinfo *) myself)
	  pinfolist[nelem].release ();
	return;
      }

out:
  pidlist[nelem++] = pid;
}

DWORD
winpids::enumNT (bool winpid)
{
  static DWORD szprocs;
  static SYSTEM_PROCESSES *procs;

  DWORD nelem = 0;
  if (!szprocs)
    procs = (SYSTEM_PROCESSES *) malloc (sizeof (*procs) + (szprocs = 200 * sizeof (*procs)));

  NTSTATUS res;
  for (;;)
    {
      res = NtQuerySystemInformation (SystemProcessesAndThreadsInformation,
				      procs, szprocs, NULL);
      if (res == 0)
	break;

      if (res == STATUS_INFO_LENGTH_MISMATCH)
	procs =  (SYSTEM_PROCESSES *) realloc (procs, szprocs += 200 * sizeof (*procs));
      else
	{
	  system_printf ("error %p reading system process information", res);
	  return 0;
	}
    }

  SYSTEM_PROCESSES *px = procs;
  for (;;)
    {
      if (px->ProcessId)
	add (nelem, winpid, px->ProcessId);
      if (!px->NextEntryDelta)
	break;
      px = (SYSTEM_PROCESSES *) ((char *) px + px->NextEntryDelta);
    }

  return nelem;
}

DWORD
winpids::enum9x (bool winpid)
{
  DWORD nelem = 0;

  HANDLE h = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
  if (!h)
    {
      system_printf ("Couldn't create process snapshot, %E");
      return 0;
    }

  PROCESSENTRY32 proc;
  proc.dwSize = sizeof (proc);

  if (Process32First (h, &proc))
    do
      {
	if (proc.th32ProcessID)
	  add (nelem, winpid, proc.th32ProcessID);
      }
    while (Process32Next (h, &proc));

  CloseHandle (h);
  return nelem;
}

void
winpids::set (bool winpid)
{
  __malloc_lock ();
  npids = (this->*enum_processes) (winpid);
  if (pidlist)
    pidlist[npids] = 0;
  __malloc_unlock ();
}

DWORD
winpids::enum_init (bool winpid)
{
  if (wincap.is_winnt ())
    enum_processes = &winpids::enumNT;
  else
    enum_processes = &winpids::enum9x;

  return (this->*enum_processes) (winpid);
}

void
winpids::release ()
{
  for (unsigned i = 0; i < npids; i++)
    if (pinfolist[i] && (_pinfo *) pinfolist[i] != (_pinfo *) myself)
      pinfolist[i].release ();
}

winpids::~winpids ()
{
  if (npidlist)
    {
      release ();
      free (pidlist);
      free (pinfolist);
    }
}
