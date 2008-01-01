/* tty.cc

   Copyright 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008
   Red Hat, Inc.

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
posix_openpt (int oflags)
{
  return open ("/dev/ptmx", oflags);
}

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

HANDLE NO_COPY tty_list::mutex = NULL;

void __stdcall
tty_list::init_session ()
{
  char mutex_name[CYG_MAX_PATH];
  /* tty_list::mutex is used while searching for a tty slot. It's necessary
     while finding console window handle */

  char *name = shared_name (mutex_name, "tty_list::mutex", 0);
  if (!(mutex = CreateMutex (&sec_all_nih, FALSE, name)))
    api_fatal ("can't create tty_list::mutex '%s', %E", name);
  ProtectHandle (mutex);
}

void __stdcall
tty::init_session ()
{
  if (!myself->cygstarted && NOTSTATE (myself, PID_CYGPARENT))
    cygheap->fdtab.get_debugger_info ();

  if (NOTSTATE (myself, PID_USETTY))
    return;
  if (myself->ctty == -1)
    if (NOTSTATE (myself, PID_CYGPARENT))
      myself->ctty = cygwin_shared->tty.attach (myself->ctty);
    else
      return;
  if (myself->ctty == -1)
    termios_printf ("Can't attach to tty");
}

/* Create session's master tty */

void __stdcall
tty::create_master (int ttynum)
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

int __stdcall
tty_list::attach (int num)
{
  if (num != -1)
    {
      return connect (num);
    }
  if (NOTSTATE (myself, PID_USETTY))
    return -1;
  return allocate (true);
}

void
tty_list::terminate ()
{
  if (NOTSTATE (myself, PID_USETTY))
    return;
  int ttynum = myself->ctty;

  /* Keep master running till there are connected clients */
  if (ttynum != -1 && tty_master && ttys[ttynum].master_pid == myself->pid)
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

      lock_ttys here ();

      termios_printf ("tty %d master about to finish", ttynum);
      CloseHandle (tty_master->get_io_handle ());
      CloseHandle (tty_master->get_output_handle ());

      t->init ();

      char buf[20];
      __small_sprintf (buf, "tty%d", ttynum);
      logout (buf);
    }
}

int
tty_list::connect (int ttynum)
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
tty_list::allocate (bool with_console)
{
  HWND console;
  int freetty = -1;
  HANDLE hmaster = NULL;

  /* FIXME: This whole function needs a protective mutex. */

  lock_ttys here;

  if (!with_console)
    console = NULL;
  else if (!(console = GetConsoleWindow ()))
    {
      termios_printf ("Can't find console window");
      goto out;
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
  t->sethwnd (console);

out:
  if (freetty < 0)
    system_printf ("No tty allocated");
  else if (!with_console)
    {
      termios_printf ("tty%d allocated", freetty);
      here.dont_release (); /* exit with mutex still held -- caller has more work to do */
    }
  else
    {
      termios_printf ("console %p associated with tty%d", console, freetty);
      if (!hmaster)
	tty::create_master (freetty);
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

lock_ttys::lock_ttys (DWORD howlong): release_me (true)
{
  if (WaitForSingleObject (tty_list::mutex, howlong) == WAIT_FAILED)
    {
      termios_printf ("WFSO for mutex %p failed, %E", tty_list::mutex);
      release_me = false;
    }
}

void
lock_ttys::release ()
{
  ReleaseMutex (tty_list::mutex);
}
