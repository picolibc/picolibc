/* tty.cc

   Copyright 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <utmp.h>
#include <wingdi.h>
#include <winuser.h>
#include "dtable.h"
#include "pinfo.h"

extern fhandler_tty_master *tty_master;

extern "C"
int
grantpt (void)
{
  return 0;
}

extern "C"
int
unlockpt (void)
{
  return 0;
}

extern "C"
int
ttyslot (void)
{
  if (NOTSTATE (myself, PID_USETTY))
    return -1;
  return myself->ctty;
}

void __stdcall
tty_init (void)
{
  if (NOTSTATE (myself, PID_USETTY))
    return;
  if (myself->ctty == -1)
    if (NOTSTATE (myself, PID_CYGPARENT))
      myself->ctty = attach_tty (myself->ctty);
    else
      return;
  if (myself->ctty == -1)
    termios_printf ("Can't attach to tty");
}

/* Create session's master tty */

void __stdcall
create_tty_master (int ttynum)
{
  tty_master = (fhandler_tty_master *) fdtab.build_fhandler (-1, FH_TTYM,
							     "/dev/ttym", ttynum);
  if (tty_master->init (ttynum))
    api_fatal ("Can't create master tty");
  else
    {
      /* Log utmp entry */
      struct utmp our_utmp;

      bzero ((char *) &our_utmp, sizeof (utmp));
      (void) time (&our_utmp.ut_time);
      strncpy (our_utmp.ut_name, getlogin (), sizeof (our_utmp.ut_name));
      cygwin_gethostname (our_utmp.ut_host, sizeof (our_utmp.ut_host));
      __small_sprintf (our_utmp.ut_line, "tty%d", ttynum);
      our_utmp.ut_type = USER_PROCESS;
      myself->ctty = ttynum;
      login (&our_utmp);
    }
}

void __stdcall
tty_terminate (void)
{
  if (NOTSTATE (myself, PID_USETTY))
    return;
  cygwin_shared->tty.terminate ();
}

int __stdcall
attach_tty (int num)
{
  if (num != -1)
    {
      return cygwin_shared->tty.connect_tty (num);
    }
  if (NOTSTATE (myself, PID_USETTY))
    return -1;
  return cygwin_shared->tty.allocate_tty (1);
}

void
tty_list::terminate (void)
{
  int ttynum = myself->ctty;

  /* Keep master running till there are connected clients */
  if (ttynum != -1 && ttys[ttynum].master_pid == GetCurrentProcessId ())
    {
      tty *t = ttys + ttynum;
      CloseHandle (t->from_master);
      CloseHandle (t->to_master);
      /* Wait for children which rely on tty handling in this process to
	 go away */
      for (int i = 0; ; i++)
	{
	  if (!t->slave_alive ())
	    break;
	  if (i >= 100)
	    {
	      small_printf ("waiting for children using tty%d to terminate\n",
			    ttynum);
	      i = 0;
	    }

	  Sleep (200);
	}

      termios_printf ("tty %d master about to finish", ttynum);
      CloseHandle (t->to_slave);
      CloseHandle (t->from_slave);
      WaitForSingleObject (tty_master->hThread, INFINITE);
      t->init ();

      char buf[20];
      __small_sprintf (buf, "tty%d", ttynum);
      logout (buf);
    }
}

int
tty_list::connect_tty (int ttynum)
{
  if (ttynum < 0 || ttynum >= NTTYS)
    {
      termios_printf ("ttynum (%d) out of range", ttynum);
      return -1;
    }
  if (!ttys[ttynum].exists ())
    {
      termios_printf ("tty %d was not allocated", ttynum);
      return -1;
    }

  return ttynum;
}

void
tty_list::init (void)
{
  for (int i = 0; i < NTTYS; i++)
    {
      ttys[i].init ();
      ttys[i].setntty (i);
    }
}

/* Search for tty class for our console. Allocate new tty if our process is
   the only cygwin process in the current console.
   Return tty number or -1 if error.
   If flag == 0, just find a free tty.
 */
int
tty_list::allocate_tty (int with_console)
{
  HWND console;

  /* FIXME: This whole function needs a protective mutex. */

  if (!with_console)
    console = NULL;
  else
    {
      char *oldtitle = new char [TITLESIZE];

      if (!oldtitle)
	{
	  termios_printf ("Can't *allocate console title buffer");
	  return -1;
	}
      if (!GetConsoleTitle (oldtitle, TITLESIZE))
	{
	  termios_printf ("Can't read console title");
	  return -1;
	}

      if (WaitForSingleObject (title_mutex, INFINITE) == WAIT_FAILED)
	termios_printf ("WFSO for title_mutext %p failed, %E", title_mutex);

      char buf[40];

      __small_sprintf (buf, "cygwin.find.console.%d", myself->pid);
      SetConsoleTitle (buf);
      Sleep (40);
      console = FindWindow (NULL, buf);
      SetConsoleTitle (oldtitle);
      Sleep (40);
      ReleaseMutex (title_mutex);
      if (console == NULL)
	{
	  termios_printf ("Can't find console window");
	  return -1;
	}
    }
  /* Is a tty allocated for console? */

  int freetty = -1;
  for (int i = 0; i < NTTYS; i++)
    {
      if (!ttys[i].exists ())
	{
	  if (freetty < 0)	/* Scanning? */
	    freetty = i;	/* Yes. */
	  if (!with_console)	/* Do we want to attach this to a console? */
	    break;		/* No.  We've got one. */
	}

      if (with_console && ttys[i].gethwnd () == console)
	{
	  termios_printf ("console %x already associated with tty%d",
		console, i);
	  /* Is the master alive? */
	  HANDLE hMaster;
	  hMaster = OpenProcess (PROCESS_DUP_HANDLE, FALSE, ttys[i].master_pid);
	  if (hMaster)
	    {
	      CloseHandle (hMaster);
	      return i;
	    }
	  /* Master is dead */
	  freetty = i;
	  break;
	}
    }

  /* There is no tty allocated to console, allocate the first free found */
  if (freetty == -1)
    {
      system_printf ("No free ttys available");
      return -1;
    }
  tty *t = ttys + freetty;
  t->init ();
  t->setsid (-1);
  t->setpgid (myself->pgid);
  t->sethwnd (console);

  if (with_console)
    {
      termios_printf ("console %x associated with tty%d", console, freetty);
      create_tty_master (freetty);
    }
  else
    termios_printf ("tty%d allocated", freetty);
  return freetty;
}

BOOL
tty::slave_alive ()
{
  return alive (TTY_SLAVE_ALIVE);
}

BOOL
tty::master_alive ()
{
  return alive (TTY_MASTER_ALIVE);
}

BOOL
tty::alive (const char *fmt)
{
  HANDLE ev;
  char buf[sizeof (TTY_MASTER_ALIVE) + 16];

  __small_sprintf (buf, fmt, ntty);
  if ((ev = OpenEvent (EVENT_ALL_ACCESS, TRUE, buf)))
    CloseHandle (ev);
  return ev != NULL;
}

HANDLE
tty::create_inuse (const char *fmt)
{
  HANDLE h;
  char buf[sizeof (TTY_MASTER_ALIVE) + 16];

  __small_sprintf (buf, fmt, ntty);
  h = CreateEvent (&sec_all, TRUE, FALSE, buf);
  termios_printf ("%s = %p", buf, h);
  if (!h)
    termios_printf ("couldn't open inuse event, %E", buf);
  return h;
}

void
tty::init (void)
{
  OutputStopped = 0;
  setsid (0);
  pgid = 0;
  hwnd = NULL;
  to_slave = NULL;
  from_slave = NULL;
  was_opened = 0;
}

HANDLE
tty::get_event (const char *fmt, BOOL inherit)
{
  HANDLE hev;
  char buf[40];

  __small_sprintf (buf, fmt, ntty);
  if (!(hev = CreateEvent (inherit ? &sec_all : &sec_all_nih, FALSE, FALSE, buf)))
    {
      termios_printf ("couldn't create %s", buf);
      set_errno (ENOENT);	/* FIXME this can't be the right errno */
      return NULL;
    }

  termios_printf ("created event %s", buf);
  return hev;
}

int
tty::make_pipes (fhandler_pty_master *ptym)
{
  /* Create communication pipes */

  /* FIXME: should this be sec_none_nih? */
  if (CreatePipe (&from_master, &to_slave, &sec_all, 0) == FALSE)
    {
      termios_printf ("can't create input pipe");
      set_errno (ENOENT);
      return FALSE;
    }

  if (CreatePipe (&from_slave, &to_master, &sec_all, 0) == FALSE)
    {
      termios_printf ("can't create output pipe");
      set_errno (ENOENT);
      return FALSE;
    }
  termios_printf ("tty%d from_slave %p, to_slave %p", ntty, from_slave,
		  to_slave);
  ptym->set_io_handle (from_slave);
  ptym->set_output_handle (to_slave);
  return TRUE;
}

BOOL
tty::common_init (fhandler_pty_master *ptym)
{
  /* Set termios information.  Force initialization. */
  ptym->tcinit (this, TRUE);

  if (!make_pipes (ptym))
    return FALSE;
  ptym->need_nl = 0;

  /* Save our pid  */

  master_pid = GetCurrentProcessId ();

  /* Allow the others to open us (for handle duplication) */

  if ((os_being_run == winNT) &&
      (SetKernelObjectSecurity (hMainProc, DACL_SECURITY_INFORMATION,
			       get_null_sd ()) == FALSE))
    small_printf ("Can't set process security, %E");

  /* Create synchronisation events */

  if (!(ptym->restart_output_event = get_event (RESTART_OUTPUT_EVENT, TRUE)))
    return FALSE;

  if (ptym->get_device () != FH_TTYM)
    {
      ptym->output_done_event = ptym->ioctl_done_event =
      ptym->ioctl_request_event = NULL;
    }
  else
    {
      if (!(ptym->output_done_event = get_event (OUTPUT_DONE_EVENT, FALSE)))
	return FALSE;
      if (!(ptym->ioctl_done_event = get_event (IOCTL_DONE_EVENT, FALSE)))
	return FALSE;
      if (!(ptym->ioctl_request_event = get_event (IOCTL_REQUEST_EVENT, FALSE)))
	return FALSE;
    }

  char buf[40];
  __small_sprintf (buf, OUTPUT_MUTEX, ntty);
  if (!(ptym->output_mutex = CreateMutex (&sec_all, FALSE, buf)))
    {
      termios_printf ("can't create %s", buf);
      set_errno (ENOENT);
      return FALSE;
    }

  ProtectHandle1 (ptym->output_mutex, output_mutex);
  winsize.ws_col = 80;
  winsize.ws_row = 25;

  termios_printf("tty%d opened", ntty);
  return TRUE;
}
