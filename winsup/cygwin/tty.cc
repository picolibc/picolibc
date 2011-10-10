/* tty.cc

   Copyright 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009,
   2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <utmp.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "shared_info.h"

HANDLE NO_COPY tty_list::mutex = NULL;

extern "C" int
posix_openpt (int oflags)
{
  return open ("/dev/ptmx", oflags);
}

extern "C" int
grantpt (int fd)
{
  cygheap_fdget cfd (fd);
  return cfd < 0 ? -1 : 0;
}

extern "C" int
unlockpt (int fd)
{
  cygheap_fdget cfd (fd);
  return cfd < 0 ? -1 : 0;
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
  if (myself->ctty <= 0 || iscons_dev (myself->ctty))
    return -1;
  return device::minor (myself->ctty);
}

void __stdcall
tty_list::init_session ()
{
  char mutex_name[MAX_PATH];
  char *name = shared_name (mutex_name, "tty_list::mutex", 0);

  /* tty_list::mutex is used while searching for a tty slot */
  if (!(mutex = CreateMutex (&sec_all_nih, FALSE, name)))
    api_fatal ("can't create tty_list::mutex '%s', %E", name);
  ProtectHandle (mutex);
}

void __stdcall
tty::init_session ()
{
  if (!myself->cygstarted && NOTSTATE (myself, PID_CYGPARENT))
    cygheap->fdtab.get_debugger_info ();
}

int __stdcall
tty_list::attach (int n)
{
  int res;
  if (iscons_dev (n))
    res = -1;
  else if (n != -1)
    res = connect (device::minor (n));
  else
    res = -1;
  return res;
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
      set_errno (ENXIO);
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
      ttys[i].setntty (DEV_TTYS_MAJOR, i);
    }
}

/* Search for a free tty and allocate it.
   Return tty number or -1 if error.
 */
int
tty_list::allocate ()
{
  lock_ttys here;
  int freetty = -1;

  tty *t = NULL;
  for (int i = 0; i < NTTYS; i++)
    if (!ttys[i].exists ())
      {
	t = ttys + i;
	t->init ();
	t->setsid (-1);
	freetty = i;
	break;
      }

  if (freetty < 0)
    system_printf ("No tty allocated");
  else
    {
      termios_printf ("tty%d allocated", freetty);
      here.dont_release (); /* exit with mutex still held -- caller has more work to do */
    }
  return freetty;
}

bool
tty::exists ()
{
  /* Attempt to open the from-master side of the tty.  If it is accessible
     then it exists although it may have been privileges to actually use it. */
  char pipename[sizeof("ttyNNNN-from-master")];
  __small_sprintf (pipename, "tty%d-from-master", get_unit ());
  HANDLE r, w;
  int res = fhandler_pipe::create_selectable (&sec_none_nih, r, w, 0, pipename);
  if (res)
    return true;

  CloseHandle (r);
  CloseHandle (w);

  HANDLE h = open_output_mutex (READ_CONTROL);
  if (h)
    {
      CloseHandle (h);
      return true;
    }
  return slave_alive ();
}

bool
tty::slave_alive ()
{
  HANDLE ev;
  if ((ev = open_inuse (READ_CONTROL)))
    CloseHandle (ev);
  return ev != NULL;
}

HANDLE
tty::open_mutex (const char *mutex, ACCESS_MASK access)
{
  char buf[MAX_PATH];
  shared_name (buf, mutex, get_unit ());
  return OpenMutex (access, TRUE, buf);
}

HANDLE
tty::open_inuse (ACCESS_MASK access)
{
  char buf[MAX_PATH];
  shared_name (buf, TTY_SLAVE_ALIVE, get_unit ());
  return OpenEvent (access, FALSE, buf);
}

HANDLE
tty::create_inuse (PSECURITY_ATTRIBUTES sa)
{
  HANDLE h;
  char buf[MAX_PATH];

  shared_name (buf, TTY_SLAVE_ALIVE, get_unit ());
  h = CreateEvent (sa, TRUE, FALSE, buf);
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
  was_opened = false;
  master_pid = 0;
  is_console = false;
}

HANDLE
tty::get_event (const char *fmt, PSECURITY_ATTRIBUTES sa, BOOL manual_reset)
{
  HANDLE hev;
  char buf[MAX_PATH];

  shared_name (buf, fmt, get_unit ());
  if (!sa)
    sa = &sec_all;
  if (!(hev = CreateEvent (sa, manual_reset, FALSE, buf)))
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

const char *
tty_min::ttyname ()
{
  device d;
  d.parse (ntty);
  return d.name;
}
