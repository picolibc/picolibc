/* fhandler_fifo.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 2002, 2003, 2004 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"

fhandler_fifo::fhandler_fifo ()
  : fhandler_pipe (), output_handle (NULL), owner (NULL), upand (0),
    read_use (0), write_use (0)
{
}

void
fhandler_fifo::set_use (int incr)
{
  long oread_use = read_use;

  if (get_flags () & (O_WRONLY | O_APPEND))
    write_use += incr;
  else if (get_flags () & O_RDWR)
    {
      write_use += incr;
      read_use += incr;
    }
  else
    read_use += incr;

  if (incr >= 0)
    return;
  char buf[24];
  __small_sprintf (buf, "%x.%x", upand, myself->pid);
  ATOM ant = GlobalFindAtom (buf);
  if (!ant)
    return;
  if (read_use <= 0 && oread_use != read_use)
    {
      HANDLE h = get_handle ();
      if (h)
	{
	  set_io_handle (NULL);
	  CloseHandle (h);
	}
      DeleteAtom (ant);
    }
}

int
fhandler_fifo::close ()
{
  fhandler_pipe::close ();
  CloseHandle (get_output_handle ());
  set_use (-1);
  return 0;
}

#define DUMMY_O_RDONLY 4
int
fhandler_fifo::open_not_mine (int flags)
{
  char buf[24];
  winpids pids;
  static int flagtypes[] = {DUMMY_O_RDONLY | O_RDWR, O_WRONLY | O_APPEND | O_RDWR};
  HANDLE *usehandles[2] = {&(get_handle ()), &(get_output_handle ())};
  int res;

  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];
      __small_sprintf (buf, "%x.%x", upand, p->pid);
      if (!GlobalFindAtom (buf))
	continue;

      HANDLE hp = OpenProcess (PROCESS_DUP_HANDLE, false, p->dwProcessId);
      if (!hp)
	{
	  __seterrno ();
	  goto err;
	}

      HANDLE handles[2];
      commune_result r;
      r = p->commune_send (PICOM_FIFO, upand);
      flags = (flags & (O_RDWR | O_WRONLY | O_APPEND)) ?: DUMMY_O_RDONLY;
      for (int i = 0; i < 2; i++)
	{
	  if (!(flags & flagtypes[i]))
	    continue;
	   if (!DuplicateHandle (hp, r.handles[i], hMainProc, usehandles[i], 0,
				 false, DUPLICATE_SAME_ACCESS))
	    {
	      debug_printf ("couldn't duplicate handle %d/%p, %E", i, handles[i]);
	      __seterrno ();
	      goto err;
	    }

	  if (i == 0)
	    {
	      read_state = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
	      set_need_fork_fixup ();
	    }
	}
      CloseHandle (hp);

      res = 1;
      set_flags (flags);
      goto out;
    }

  set_errno (EAGAIN);

err:
  res = 0;
  debug_printf ("failed");

out:
  debug_printf ("res %d", res);
  return res;
}

int
fhandler_fifo::open (int flags, mode_t)
{
  int res = 1;
  char buf[24];

  upand = GlobalAddAtom (pc);
  __small_sprintf (buf, "%x.owner", upand);
  debug_printf ("mutex %s", buf);

  HANDLE h = CreateMutex (&sec_none, false, buf);
  if (!h)
    goto errnout;

  if (GetLastError () == ERROR_ALREADY_EXISTS)
    {
      CloseHandle (h);
      return open_not_mine (flags);
    }

  fhandler_pipe *fhs[2];
  if (create (fhs, 0, flags, true))
    {
      CloseHandle (h);
      goto errout;
    }

  set_flags (fhs[0]->get_flags ());
  set_io_handle (fhs[0]->get_handle ());
  set_output_handle (fhs[1]->get_handle ());
  guard = fhs[0]->guard;
  read_state = fhs[0]->read_state;
  writepipe_exists = fhs[1]->writepipe_exists;
  orig_pid = fhs[0]->orig_pid;
  id = fhs[0]->id;
  delete (fhs[0]);
  delete (fhs[1]);
  set_use (1);
  __small_sprintf (buf, "%x.%x", upand, myself->pid);
  (void) GlobalAddAtom (buf);
  goto out;

errnout:
  __seterrno ();
errout:
  res = 0;

out:
  debug_printf ("returning %d, errno %d", res, get_errno ());
  return res;
}

int
fhandler_fifo::dup (fhandler_base * child)
{
  int res = fhandler_pipe::dup (child);
  if (!res)
    {
      fhandler_fifo *ff = (fhandler_fifo *) child;

      if (!DuplicateHandle (hMainProc, get_output_handle (), hMainProc,
			    &ff->get_output_handle (), false, true,
			    DUPLICATE_SAME_ACCESS))
	{
	  child->close ();
	  res = -1;
	}
    }
  return 0;
}
