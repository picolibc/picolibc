/* tty.cc

   Copyright 1997, 1998, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <utmp.h>
#include <wingdi.h>
#include <winuser.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "cygserver.h"
#include "shared_info.h"
#include "cygthread.h"

extern fhandler_tty_master *tty_master;

extern "C" int
grantpt (int fd)
{
  return 0;
}

extern "C" int
unlockpt (int fd)
{
  return 0;
}

extern "C" int
revoke (char *ttyname)
{
  set_errno (ENOSYS);
  return -1;
}

extern "C" int
ttyslot (void)
{
  if (NOTSTATE (myself, PID_USETTY))
    return -1;
  return myself->ctty;
}

void __stdcall
tty_init ()
{
  if (!myself->cygstarted && NOTSTATE (myself, PID_CYGPARENT))
    cygheap->fdtab.get_debugger_info ();

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
  device ttym = *ttym_dev;
  ttym.setunit (ttynum); /* CGF FIXME device */
  tty_master = (fhandler_tty_master *) build_fh_dev (ttym);
  if (tty_master->init ())
    api_fatal ("Can't create master tty");
  else
    {
      /* Log utmp entry */
      struct utmp our_utmp;
      DWORD len = sizeof our_utmp.ut_host;

      bzero ((char *) &our_utmp, sizeof (utmp));
      time (&our_utmp.ut_time);
      strncpy (our_utmp.ut_name, getlogin (), sizeof (our_utmp.ut_name));
      GetComputerName (our_utmp.ut_host, &len);
      __small_sprintf (our_utmp.ut_line, "tty%d", ttynum);
      if ((len = strlen (our_utmp.ut_line)) >= UT_IDLEN)
	len -= UT_IDLEN;
      else
	len = 0;
      strncpy (our_utmp.ut_id, our_utmp.ut_line + len, UT_IDLEN);
      our_utmp.ut_type = USER_PROCESS;
      our_utmp.ut_pid = myself->pid;
      myself->ctty = ttynum;
      login (&our_utmp);
    }
}

void __stdcall
tty_terminate ()
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
  return cygwin_shared->tty.allocate_tty (true);
}

void
tty_list::terminate ()
{
  int ttynum = myself->ctty;

  /* Keep master running till there are connected clients */
  if (ttynum != -1 && ttys[ttynum].master_pid == myself->pid)
    {
      tty *t = ttys + ttynum;
      CloseHandle (tty_master->from_master);
      CloseHandle (tty_master->to_master);
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

	  low_priority_sleep (200);
	}

      if (WaitForSingleObject (tty_mutex, INFINITE) == WAIT_FAILED)
	termios_printf ("WFSO for tty_mutex %p failed, %E", tty_mutex);

      termios_printf ("tty %d master about to finish", ttynum);
      CloseHandle (tty_master->get_io_handle ());
      CloseHandle (tty_master->get_output_handle ());

      t->init ();

      char buf[20];
      __small_sprintf (buf, "tty%d", ttynum);
      logout (buf);

      ReleaseMutex (tty_mutex);
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
tty_list::init ()
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
tty_list::allocate_tty (bool with_console)
{
  HWND console;
  int freetty = -1;
  HANDLE hmaster = NULL;

  /* FIXME: This whole function needs a protective mutex. */

  if (WaitForSingleObject (tty_mutex, INFINITE) == WAIT_FAILED)
    termios_printf ("WFSO for tty_mutex %p failed, %E", tty_mutex);

  if (!with_console)
    console = NULL;
  else if (!(console = GetConsoleWindow ()))
    {
      char oldtitle[TITLESIZE];

      if (!GetConsoleTitle (oldtitle, TITLESIZE))
	{
	  termios_printf ("Can't read console title");
	  goto out;
	}

      char buf[40];

      __small_sprintf (buf, "cygwin.find.console.%d", myself->pid);
      SetConsoleTitle (buf);
      for (int times = 0; times < 25; times++)
	{
	  Sleep (10);
	  if ((console = FindWindow (NULL, buf)))
	    break;
	}
      SetConsoleTitle (oldtitle);
      Sleep (40);
      if (console == NULL)
	{
	  termios_printf ("Can't find console window");
	  goto out;
	}
    }

  /* Is a tty allocated for console? */
  for (int i = 0; i < NTTYS; i++)
    {
      if (!ttys[i].exists ())
	{
	  if (freetty < 0)	/* Scanning? */
	    freetty = i;	/* Yes. */
	  if (!with_console)	/* Do we want to attach this to a console? */
	    break;		/* No.  We've got one. */
	}

      /* FIXME: Is this right?  We can potentially query a "nonexistent"
	 tty slot after falling through from the above? */
      if (with_console && ttys[i].gethwnd () == console)
	{
	  termios_printf ("console %x already associated with tty%d",
		console, i);
	  /* Is the master alive? */
	  hmaster = OpenProcess (PROCESS_DUP_HANDLE, FALSE, ttys[i].master_pid);
	  if (hmaster)
	    {
	      CloseHandle (hmaster);
	      freetty = i;
	      goto out;
	    }
	  /* Master is dead */
	  freetty = i;
	  break;
	}
    }

  /* There is no tty allocated to console, allocate the first free found */
  if (freetty == -1)
    goto out;

  tty *t;
  t = ttys + freetty;
  t->init ();
  t->setsid (-1);
  t->setpgid (myself->pgid);
  t->sethwnd (console);

out:
  if (freetty < 0)
    {
      ReleaseMutex (tty_mutex);
      system_printf ("No tty allocated");
    }
  else if (!with_console)
    {
      termios_printf ("tty%d allocated", freetty);
      /* exit with tty_mutex still held -- caller has more work to do */
    }
  else
    {
      termios_printf ("console %p associated with tty%d", console, freetty);
      if (!hmaster)
	create_tty_master (freetty);
      ReleaseMutex (tty_mutex);
    }
  return freetty;
}

bool
tty::slave_alive ()
{
  return alive (TTY_SLAVE_ALIVE);
}

bool
tty::alive (const char *fmt)
{
  HANDLE ev;
  char buf[CYG_MAX_PATH];

  shared_name (buf, fmt, ntty);
  if ((ev = OpenEvent (EVENT_ALL_ACCESS, FALSE, buf)))
    CloseHandle (ev);
  return ev != NULL;
}

HANDLE
tty::open_output_mutex ()
{
  return open_mutex (OUTPUT_MUTEX);
}

HANDLE
tty::open_input_mutex ()
{
  return open_mutex (INPUT_MUTEX);
}

HANDLE
tty::open_mutex (const char *mutex)
{
  char buf[CYG_MAX_PATH];
  shared_name (buf, mutex, ntty);
  return OpenMutex (MUTEX_ALL_ACCESS, TRUE, buf);
}

HANDLE
tty::create_inuse (const char *fmt)
{
  HANDLE h;
  char buf[CYG_MAX_PATH];

  shared_name (buf, fmt, ntty);
  h = CreateEvent (&sec_all, TRUE, FALSE, buf);
  termios_printf ("%s %p", buf, h);
  if (!h)
    termios_printf ("couldn't open inuse event, %E", buf);
  return h;
}

void
tty::init ()
{
  output_stopped = 0;
  setsid (0);
  pgid = 0;
  hwnd = NULL;
  was_opened = 0;
  master_pid = 0;
}

HANDLE
tty::get_event (const char *fmt, BOOL manual_reset)
{
  HANDLE hev;
  char buf[CYG_MAX_PATH];

  shared_name (buf, fmt, ntty);
  if (!(hev = CreateEvent (&sec_all, manual_reset, FALSE, buf)))
    {
      termios_printf ("couldn't create %s", buf);
      set_errno (ENOENT);	/* FIXME this can't be the right errno */
      return NULL;
    }

  termios_printf ("created event %s", buf);
  return hev;
}
