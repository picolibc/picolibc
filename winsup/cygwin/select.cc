/* select.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

   Written by Christopher Faylor of Cygnus Solutions
   cgf@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/*
 * The following line means that the BSD socket
 * definitions for fd_set, FD_ISSET etc. are used in this
 * file.
 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/time.h>

#include <wingdi.h>
#include <winuser.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock.h>
#include "select.h"
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "sync.h"
#include "sigproc.h"
#include "perthread.h"
#include "tty.h"

/*
 * All these defines below should be in sys/types.h
 * but because of the includes above, they may not have
 * been included. We create special UNIX_xxxx versions here.
 */

#ifndef NBBY
#define NBBY 8    /* number of bits in a byte */
#endif /* NBBY */

/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */

typedef long fd_mask;
#define UNIX_NFDBITS (sizeof (fd_mask) * NBBY)       /* bits per mask */
#ifndef unix_howmany
#define unix_howmany(x,y) (((x)+((y)-1))/(y))
#endif

#define unix_fd_set fd_set

#define NULL_fd_set ((fd_set *) NULL)
#define sizeof_fd_set(n) \
  ((unsigned) (NULL_fd_set->fds_bits + unix_howmany((n), UNIX_NFDBITS)))
#define UNIX_FD_SET(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] |= (1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_CLR(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] &= ~(1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_ISSET(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] & (1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_ZERO(p, n) \
  bzero ((caddr_t)(p), sizeof_fd_set ((n)))

#define allocfd_set(n) ((fd_set *) memset (alloca (sizeof_fd_set (n)), 0, sizeof_fd_set (n)))
#define copyfd_set(to, from, n) memcpy (to, from, sizeof_fd_set (n));

#define set_handle_or_return_if_not_open(h, s) \
  h = (s)->fh->get_handle (); \
  if (cygheap->fdtab.not_open ((s)->fd)) \
    { \
      (s)->saw_error = true; \
      set_sig_errno (EBADF); \
      return -1; \
    } \

/* The main select code.
 */
extern "C"
int
cygwin_select (int maxfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	       struct timeval *to)
{
  select_stuff sel;
  fd_set *dummy_readfds = allocfd_set (maxfds);
  fd_set *dummy_writefds = allocfd_set (maxfds);
  fd_set *dummy_exceptfds = allocfd_set (maxfds);
  sigframe thisframe (mainthread);

  select_printf ("%d, %p, %p, %p, %p", maxfds, readfds, writefds, exceptfds, to);

  if (!readfds)
    readfds = dummy_readfds;
  if (!writefds)
    writefds = dummy_writefds;
  if (!exceptfds)
    exceptfds = dummy_exceptfds;

  for (int i = 0; i < maxfds; i++)
    if (!sel.test_and_set (i, readfds, writefds, exceptfds))
      {
	select_printf ("aborting due to test_and_set error");
	return -1;	/* Invalid fd, maybe? */
      }

  /* Convert to milliseconds or INFINITE if to == NULL */
  DWORD ms = to ? (to->tv_sec * 1000) + (to->tv_usec / 1000) : INFINITE;
  if (ms == 0 && to->tv_usec)
    ms = 1;			/* At least 1 ms granularity */

  if (to)
    select_printf ("to->tv_sec %d, to->tv_usec %d, ms %d", to->tv_sec, to->tv_usec, ms);
  else
    select_printf ("to NULL, ms %x", ms);

  select_printf ("sel.always_ready %d", sel.always_ready);

  int timeout = 0;
  /* Allocate some fd_set structures using the number of fds as a guide. */
  fd_set *r = allocfd_set (maxfds);
  fd_set *w = allocfd_set (maxfds);
  fd_set *e = allocfd_set (maxfds);

  /* Degenerate case.  No fds to wait for.  Just wait. */
  if (sel.start.next == NULL)
    {
      if (WaitForSingleObject (signal_arrived, ms) == WAIT_OBJECT_0)
	{
	  select_printf ("signal received");
	  set_sig_errno (EINTR);
	  return -1;
	}
      timeout = 1;
    }
  else if (sel.always_ready || ms == 0)
    /* Don't bother waiting. */;
  else if ((timeout = sel.wait (r, w, e, ms) < 0))
    return -1;	/* some kind of error */

  sel.cleanup ();
  copyfd_set (readfds, r, maxfds);
  copyfd_set (writefds, w, maxfds);
  copyfd_set (exceptfds, e, maxfds);
  return timeout ? 0 : sel.poll (readfds, writefds, exceptfds);
}

/* Call cleanup functions for all inspected fds.  Gets rid of any
   executing threads. */
void
select_stuff::cleanup ()
{
  select_record *s = &start;

  select_printf ("calling cleanup routines");
  while ((s = s->next))
    if (s->cleanup)
      {
	s->cleanup (s, this);
	s->cleanup = NULL;
      }
}

/* Destroy all storage associated with select stuff. */
select_stuff::~select_stuff ()
{
  cleanup ();
  select_record *s = &start;
  select_record *snext = start.next;

  select_printf ("deleting select records");
  while ((s = snext))
    {
      snext = s->next;
      delete s;
    }
}

/* Add a record to the select chain */
int
select_stuff::test_and_set (int i, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds)
{
  select_record *s = NULL;
  if (UNIX_FD_ISSET (i, readfds) && (s = cygheap->fdtab.select_read (i, s)) == NULL)
    return 0; /* error */
  if (UNIX_FD_ISSET (i, writefds) && (s = cygheap->fdtab.select_write (i, s)) == NULL)
    return 0; /* error */
  if (UNIX_FD_ISSET (i, exceptfds) && (s = cygheap->fdtab.select_except (i, s)) == NULL)
    return 0; /* error */
  if (s == NULL)
    return 1; /* nothing to do */

  if (s->read_ready || s->write_ready || s->except_ready)
    always_ready = true;

  if (s->windows_handle || s->windows_handle || s->windows_handle)
    windows_used = true;

  s->next = start.next;
  start.next = s;
  return 1;
}

/* The heart of select.  Waits for an fd to do something interesting. */
int
select_stuff::wait (fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		    DWORD ms)
{
  int wait_ret;
  HANDLE w4[MAXIMUM_WAIT_OBJECTS];
  select_record *s = &start;
  int m = 0;
  int res = 0;

  w4[m++] = signal_arrived;  /* Always wait for the arrival of a signal. */
  /* Loop through the select chain, starting up anything appropriate and
     counting the number of active fds. */
  while ((s = s->next))
    {
      if (m > MAXIMUM_WAIT_OBJECTS)
	{
	  set_sig_errno (EINVAL);
	  return -1;
	}
      if (!s->startup (s, this))
	{
	  __seterrno ();
	  return -1;
	}
      if (s->h == NULL)
	continue;
      for (int i = 1; i < m; i++)
	if (w4[i] == s->h)
	  goto next_while;
      w4[m++] = s->h;
  next_while:
      continue;
    }

  DWORD start_time = GetTickCount ();	/* Record the current time for later use. */

  debug_printf ("m %d, ms %u", m, ms);
  for (;;)
    {
      if (!windows_used)
	wait_ret = WaitForMultipleObjects (m, w4, FALSE, ms);
      else
	wait_ret = MsgWaitForMultipleObjects (m, w4, FALSE, ms, QS_ALLINPUT);

      switch (wait_ret)
      {
	case WAIT_OBJECT_0:
	  select_printf ("signal received");
	  set_sig_errno (EINTR);
	  return -1;
	case WAIT_FAILED:
	  select_printf ("WaitForMultipleObjects failed");
	  __seterrno ();
	  return -1;
	case WAIT_TIMEOUT:
	  select_printf ("timed out");
	  res = 1;
	  goto out;
      }

      select_printf ("woke up.  wait_ret %d.  verifying", wait_ret);
      s = &start;
      int gotone = FALSE;
      /* Some types of object (e.g., consoles) wake up on "inappropriate" events
	 like mouse movements.  The verify function will detect these situations.
	 If it returns false, then this wakeup was a false alarm and we should go
	 back to waiting. */
      while ((s = s->next))
	if (s->saw_error)
	  return -1;		/* Somebody detected an error */
	else if ((((wait_ret >= m && s->windows_handle) || s->h == w4[wait_ret])) &&
	    s->verify (s, readfds, writefds, exceptfds))
	  gotone = true;

      select_printf ("gotone %d", gotone);
      if (gotone)
	goto out;

      if (ms == INFINITE)
	{
	  select_printf ("looping");
	  continue;
	}
      select_printf ("recalculating ms");

      DWORD now = GetTickCount ();
      if (now > (start_time + ms))
	{
	  select_printf ("timed out after verification");
	  goto out;
	}
      ms -= (now - start_time);
      start_time = now;
      select_printf ("ms now %u", ms);
    }

out:
  select_printf ("returning %d", res);
  return res;
}

static int
set_bits (select_record *me, fd_set *readfds, fd_set *writefds,
	  fd_set *exceptfds)
{
  int ready = 0;
  select_printf ("me %p, testing fd %d (%s)", me, me->fd, me->fh->get_name ());
  if (me->read_selected && me->read_ready)
    {
      UNIX_FD_SET (me->fd, readfds);
      ready++;
    }
  if (me->write_selected && me->write_ready)
    {
      UNIX_FD_SET (me->fd, writefds);
      ready++;
    }
  if (me->except_selected && me->except_ready)
    {
      UNIX_FD_SET (me->fd, exceptfds);
      ready++;
    }
  select_printf ("ready %d", ready);
  return ready;
}

/* Poll every fd in the select chain.  Set appropriate fd in mask. */
int
select_stuff::poll (fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
  int n = 0;
  select_record *s = &start;
  while ((s = s->next))
    n += (!s->peek || s->peek (s, true)) ?
	 set_bits (s, readfds, writefds, exceptfds) : 0;
  select_printf ("returning %d", n);
  return n;
}

static int
verify_true (select_record *, fd_set *, fd_set *, fd_set *)
{
  return 1;
}

static int
verify_ok (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)
{
  return set_bits (me, readfds, writefds, exceptfds);
}

static int
no_startup (select_record *, select_stuff *)
{
  return 1;
}

static int
no_verify (select_record *, fd_set *, fd_set *, fd_set *)
{
  return 0;
}

static int
peek_pipe (select_record *s, bool from_select)
{
  int n = 0;
  int gotone = 0;
  fhandler_base *fh = s->fh;

  HANDLE h;
  set_handle_or_return_if_not_open (h, s);

  /* pipes require a guard mutex to guard against the situation where multiple
     readers are attempting to read from the same pipe.  In this scenario, it
     is possible for PeekNamedPipe to report available data to two readers but
     only one will actually get the data.  This will result in the other reader
     entering fhandler_base::raw_read and blocking indefinitely in an interruptible
     state.  This causes things like "make -j2" to hang.  So, for the non-select case
     we use the pipe mutex, if it is available. */
  HANDLE guard_mutex = from_select ? NULL : fh->get_guard ();

  /* Don't perform complicated tests if we don't need to. */
  if (!s->read_selected && !s->except_selected)
    goto out;

  if (s->read_selected)
    {
      if (s->read_ready)
	{
	  select_printf ("already ready");
	  gotone = 1;
	  goto out;
	}

      switch (fh->get_device ())
	{
	case FH_PTYM:
	case FH_TTYM:
	  if (((fhandler_pty_master *)fh)->need_nl)
	    {
	      gotone = s->read_ready = true;
	      goto out;
	    }
	  break;
	default:
	  if (fh->get_readahead_valid ())
	    {
	      select_printf ("readahead");
	      gotone = s->read_ready = true;
	      goto out;
	    }
	}

      if (fh->bg_check (SIGTTIN) <= bg_eof)
	{
	  gotone = s->read_ready = true;
	  goto out;
	}
    }

  if (fh->get_device () == FH_PIPEW)
    /* nothing */;
  else if (!PeekNamedPipe (h, NULL, 0, NULL, (DWORD *) &n, NULL))
    {
      select_printf ("%s, PeekNamedPipe failed, %E", fh->get_name ());
      n = -1;
    }
  else if (!n || !guard_mutex)
    /* no guard mutex or nothing to read fromt he pipe. */;
  else if (WaitForSingleObject (guard_mutex, 0) != WAIT_OBJECT_0)
    {
      select_printf ("%s, couldn't get mutex %p, %E", fh->get_name (),
		     guard_mutex);
      n = 0;
    }
  else
    {
      /* Now that we have the mutex, make sure that no one else has snuck
	 in and grabbed the data that we originally saw. */
      if (!PeekNamedPipe (h, NULL, 0, NULL, (DWORD *) &n, NULL))
	{
	  select_printf ("%s, PeekNamedPipe failed, %E", fh->get_name ());
	  n = -1;
	}
      if (n <= 0)
	ReleaseMutex (guard_mutex);	/* Oops.  We lost the race.  */
    }

  if (n < 0)
    {
      fh->set_eof ();		/* Flag that other end of pipe is gone */
      select_printf ("%s, n %d", fh->get_name (), n);
      if (s->except_selected)
	gotone += s->except_ready = true;
      if (s->read_selected)
	gotone += s->read_ready = true;
    }
  if (n > 0 && s->read_selected)
    {
      select_printf ("%s, ready for read", fh->get_name ());
      gotone += s->read_ready = true;
    }
  if (!gotone && s->fh->hit_eof ())
    {
      select_printf ("%s, saw EOF", fh->get_name ());
      if (s->except_selected)
	gotone = s->except_ready = true;
      if (s->read_selected)
	gotone += s->read_ready = true;
      select_printf ("saw eof on '%s'", fh->get_name ());
    }

out:
  return gotone || s->write_ready;
}

static int start_thread_pipe (select_record *me, select_stuff *stuff);

struct pipeinf
  {
    HANDLE thread;
    BOOL stop_thread_pipe;
    select_record *start;
  };

static DWORD WINAPI
thread_pipe (void *arg)
{
  pipeinf *pi = (pipeinf *)arg;
  BOOL gotone = FALSE;

  for (;;)
    {
      select_record *s = pi->start;
      while ((s = s->next))
	if (s->startup == start_thread_pipe)
	  {
	    if (peek_pipe (s, true))
	      gotone = true;
	    if (pi->stop_thread_pipe)
	      {
		select_printf ("stopping");
		goto out;
	      }
	  }
      /* Paranoid check */
      if (pi->stop_thread_pipe)
	{
	  select_printf ("stopping from outer loop");
	  break;
	}
      if (gotone)
	break;
      Sleep (10);
    }
out:
  return 0;
}

static int
start_thread_pipe (select_record *me, select_stuff *stuff)
{
  if (stuff->device_specific[FHDEVN(FH_PIPE)])
    {
      me->h = ((pipeinf *) stuff->device_specific[FHDEVN(FH_PIPE)])->thread;
      return 1;
    }
  pipeinf *pi = new pipeinf;
  pi->start = &stuff->start;
  pi->stop_thread_pipe = FALSE;
  pi->thread = me->h = makethread (thread_pipe, (LPVOID)pi, 0, "select_pipe");
  if (!me->h)
    return 0;
  stuff->device_specific[FHDEVN(FH_PIPE)] = (void *)pi;
  return 1;
}

static void
pipe_cleanup (select_record *, select_stuff *stuff)
{
  pipeinf *pi = (pipeinf *)stuff->device_specific[FHDEVN(FH_PIPE)];
  if (pi && pi->thread)
    {
      pi->stop_thread_pipe = true;
      WaitForSingleObject (pi->thread, INFINITE);
      CloseHandle (pi->thread);
      delete pi;
      stuff->device_specific[FHDEVN(FH_PIPE)] = NULL;
    }
}

select_record *
fhandler_pipe::select_read (select_record *s)
{
  if (!s)
    s = new select_record;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->read_selected = true;
  s->read_ready = false;
  s->cleanup = pipe_cleanup;
  return s;
}

select_record *
fhandler_pipe::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->peek = peek_pipe;
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_pipe::select_except (select_record *s)
{
  if (!s)
      s = new select_record;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

static int
peek_console (select_record *me, bool)
{
  extern const char * get_nonascii_key (INPUT_RECORD& input_rec, char *);
  fhandler_console *fh = (fhandler_console *)me->fh;

  if (!me->read_selected)
    return me->write_ready;

  if (fh->get_readahead_valid ())
    {
      select_printf ("readahead");
      return me->read_ready = true;
    }

  if (me->read_ready)
    {
      select_printf ("already ready");
      return 1;
    }

  INPUT_RECORD irec;
  DWORD events_read;
  HANDLE h;
  char tmpbuf[17];
  set_handle_or_return_if_not_open (h, me);

  for (;;)
    if (fh->bg_check (SIGTTIN) <= bg_eof)
      return me->read_ready = true;
    else if (!PeekConsoleInput (h, &irec, 1, &events_read) || !events_read)
      break;
    else
      {
	if (irec.EventType == WINDOW_BUFFER_SIZE_EVENT)
	  kill_pgrp (fh->tc->getpgid (), SIGWINCH);
	else if (irec.EventType == MOUSE_EVENT &&
		 (irec.Event.MouseEvent.dwEventFlags == 0 ||
		  irec.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK))
	  {
	    if (fh->mouse_aware ())
	      return me->read_ready = true;
	  }
	else if (irec.EventType == KEY_EVENT && irec.Event.KeyEvent.bKeyDown == true &&
		 (irec.Event.KeyEvent.uChar.AsciiChar || get_nonascii_key (irec, tmpbuf)))
	  return me->read_ready = true;

	/* Read and discard the event */
	ReadConsoleInput (h, &irec, 1, &events_read);
      }

  return me->write_ready;
}

static int
verify_console (select_record *me, fd_set *rfds, fd_set *wfds,
	      fd_set *efds)
{
  return peek_console (me, true);
}


select_record *
fhandler_console::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_console;
      set_cursor_maybe ();
    }

  s->peek = peek_console;
  s->h = get_handle ();
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_console::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
      set_cursor_maybe ();
    }

  s->peek = peek_console;
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_console::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
      set_cursor_maybe ();
    }

  s->peek = peek_console;
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

select_record *
fhandler_tty_common::select_read (select_record *s)
{
  return ((fhandler_pipe*)this)->fhandler_pipe::select_read (s);
}

select_record *
fhandler_tty_common::select_write (select_record *s)
{
  return ((fhandler_pipe *)this)->fhandler_pipe::select_write (s);
}

select_record *
fhandler_tty_common::select_except (select_record *s)
{
  return ((fhandler_pipe *)this)->fhandler_pipe::select_except (s);
}

static int
verify_tty_slave (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)
{
  if (WaitForSingleObject (me->h, 0) == WAIT_OBJECT_0)
    me->read_ready = true;
  return set_bits (me, readfds, writefds, exceptfds);
}

select_record *
fhandler_tty_slave::select_read (select_record *s)
{
  if (!s)
    s = new select_record;
  s->h = input_available_event;
  s->startup = no_startup;
  s->peek = peek_pipe;
  s->verify = verify_tty_slave;
  s->read_selected = true;
  s->read_ready = false;
  s->cleanup = NULL;
  return s;
}

int
fhandler_tty_slave::ready_for_read (int fd, DWORD howlong)
{
  HANDLE w4[2];
  if (cygheap->fdtab.not_open (fd))
    {
      set_sig_errno (EBADF);
      return 0;
    }
  if (get_readahead_valid ())
    {
      select_printf ("readahead");
      return 1;
    }
  w4[0] = signal_arrived;
  w4[1] = input_available_event;
  switch (WaitForMultipleObjects (2, w4, FALSE, howlong))
    {
    case WAIT_OBJECT_0:
      set_sig_errno (EINTR);
      return 0;
    case WAIT_OBJECT_0 + 1:
      return 1;
    case WAIT_FAILED:
      select_printf ("wait failed %E");
      set_sig_errno (EINVAL); /* FIXME: correct errno? */
      return 0;
    default:
      if (!howlong)
	set_sig_errno (EAGAIN);
      return 0;
    }
}

select_record *
fhandler_dev_null::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->read_selected = true;
  s->read_ready = true;
  return s;
}

select_record *
fhandler_dev_null::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_dev_null::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->except_selected = true;
  s->except_ready = true;
  return s;
}

static int start_thread_serial (select_record *me, select_stuff *stuff);

struct serialinf
  {
    HANDLE thread;
    BOOL stop_thread_serial;
    select_record *start;
  };

static int
peek_serial (select_record *s, bool)
{
  COMSTAT st;

  fhandler_serial *fh = (fhandler_serial *)s->fh;

  if (fh->get_readahead_valid () || fh->overlapped_armed < 0)
    return s->read_ready = true;

  select_printf ("fh->overlapped_armed %d", fh->overlapped_armed);

  HANDLE h;
  set_handle_or_return_if_not_open (h, s);
  int ready = 0;

  if (s->read_selected && s->read_ready || (s->write_selected && s->write_ready))
    {
      select_printf ("already ready");
      ready = 1;
      goto out;
    }

  (void) SetCommMask (h, EV_RXCHAR);

  if (!fh->overlapped_armed)
    {
      COMSTAT st;

      ResetEvent (fh->io_status.hEvent);

      if (!ClearCommError (h, &fh->ev, &st))
	{
	  debug_printf ("ClearCommError");
	  goto err;
	}
      else if (st.cbInQue)
	return s->read_ready = true;
      else if (WaitCommEvent (h, &fh->ev, &fh->io_status))
	return s->read_ready = true;
      else if (GetLastError () == ERROR_IO_PENDING)
	fh->overlapped_armed = 1;
      else
	{
	  debug_printf ("WaitCommEvent");
	  goto err;
	}
    }

  HANDLE w4[2];
  DWORD to;

  w4[0] = fh->io_status.hEvent;
  w4[1] = signal_arrived;
  to = 10;

  switch (WaitForMultipleObjects (2, w4, FALSE, to))
    {
    case WAIT_OBJECT_0:
      if (!ClearCommError (h, &fh->ev, &st))
	{
	  debug_printf ("ClearCommError");
	  goto err;
	}
      else if (!st.cbInQue)
	Sleep (to);
      else
	{
	  return s->read_ready = true;
	  select_printf ("got something");
	}
      PurgeComm (h, PURGE_TXABORT | PURGE_RXABORT);
      break;
    case WAIT_OBJECT_0 + 1:
      PurgeComm (h, PURGE_TXABORT | PURGE_RXABORT);
      select_printf ("interrupt");
      set_sig_errno (EINTR);
      ready = -1;
      break;
    case WAIT_TIMEOUT:
      PurgeComm (h, PURGE_TXABORT | PURGE_RXABORT);
      break;
    default:
      PurgeComm (h, PURGE_TXABORT | PURGE_RXABORT);
      debug_printf ("WaitForMultipleObjects");
      goto err;
    }

out:
  return ready;

err:
  if (GetLastError () == ERROR_OPERATION_ABORTED)
    {
      select_printf ("operation aborted");
      return ready;
    }

  __seterrno ();
  s->saw_error = true;
  select_printf ("error %E");
  return -1;
}

static DWORD WINAPI
thread_serial (void *arg)
{
  serialinf *si = (serialinf *)arg;
  BOOL gotone= FALSE;

  for (;;)
    {
      select_record *s = si->start;
      while ((s = s->next))
	if (s->startup == start_thread_serial)
	  {
	    if (peek_serial (s, true))
	      gotone = true;
	  }
      if (si->stop_thread_serial)
	{
	  select_printf ("stopping");
	  break;
	}
      if (gotone)
	break;
    }

  select_printf ("exiting");
  return 0;
}

static int
start_thread_serial (select_record *me, select_stuff *stuff)
{
  if (stuff->device_specific[FHDEVN(FH_SERIAL)])
    {
      me->h = ((pipeinf *) stuff->device_specific[FHDEVN(FH_SERIAL)])->thread;
      return 1;
    }
  serialinf *si = new serialinf;
  si->start = &stuff->start;
  si->stop_thread_serial = FALSE;
  si->thread = me->h = makethread (thread_serial, (LPVOID)si, 0, "select_serial");
  if (!me->h)
    return 0;
  stuff->device_specific[FHDEVN(FH_SERIAL)] = (void *)si;
  return 1;
}

static void
serial_cleanup (select_record *, select_stuff *stuff)
{
  serialinf *si = (serialinf *)stuff->device_specific[FHDEVN(FH_SERIAL)];
  if (si && si->thread)
    {
      si->stop_thread_serial = true;
      WaitForSingleObject (si->thread, INFINITE);
      CloseHandle (si->thread);
      delete si;
      stuff->device_specific[FHDEVN(FH_SERIAL)] = NULL;
    }
}

select_record *
fhandler_serial::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_serial;
      s->verify = verify_ok;
      s->cleanup = serial_cleanup;
    }
  s->peek = peek_serial;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_serial::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->peek = peek_serial;
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_serial::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = NULL;
  s->peek = peek_serial;
  s->except_selected = false;	// Can't do this
  s->except_ready = false;
  return s;
}

int
fhandler_base::ready_for_read (int fd, DWORD howlong)
{
  int avail = 0;
  select_record me (this);
  me.fd = fd;
  while (!avail)
    {
      (void) select_read (&me);
      avail = me.read_ready ?: me.peek (&me, false);

      if (fd >= 0 && cygheap->fdtab.not_open (fd))
	{
	  set_sig_errno (EBADF);
	  avail = 0;
	  break;
	}

      if (howlong != INFINITE)
	{
	  if (!avail)
	    set_sig_errno (EAGAIN);
	  break;
	}

      if (WaitForSingleObject (signal_arrived, avail ? 0 : 10) == WAIT_OBJECT_0)
	{
	  set_sig_errno (EINTR);
	  avail = 0;
	  break;
	}
    }

  if (get_guard () && !avail && me.read_ready)
    ReleaseMutex (get_guard ());

  select_printf ("read_ready %d, avail %d", me.read_ready, avail);
  return avail;
}

select_record *
fhandler_base::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->read_selected = true;
  s->read_ready = true;
  return s;
}

select_record *
fhandler_base::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_base::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = NULL;
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

struct socketinf
  {
    HANDLE thread;
    winsock_fd_set readfds, writefds, exceptfds;
    SOCKET exitsock;
    struct sockaddr_in sin;
    select_record *start;
  };

static int
peek_socket (select_record *me, bool)
{
  winsock_fd_set ws_readfds, ws_writefds, ws_exceptfds;
  struct timeval tv = {0, 0};
  WINSOCK_FD_ZERO (&ws_readfds);
  WINSOCK_FD_ZERO (&ws_writefds);
  WINSOCK_FD_ZERO (&ws_exceptfds);

  HANDLE h;
  set_handle_or_return_if_not_open (h, me);
  select_printf ("considering handle %p", h);

  if (me->read_selected)
    {
      select_printf ("adding read fd_set %s, fd %d", me->fh->get_name (),
		     me->fd);
      WINSOCK_FD_SET (h, &ws_readfds);
    }
  if (me->write_selected)
    {
      select_printf ("adding write fd_set %s, fd %d", me->fh->get_name (),
		     me->fd);
      WINSOCK_FD_SET (h, &ws_writefds);
    }
  if (me->except_selected)
    {
      select_printf ("adding except fd_set %s, fd %d", me->fh->get_name (),
		     me->fd);
      WINSOCK_FD_SET (h, &ws_exceptfds);
    }
  int r = WINSOCK_SELECT (0, &ws_readfds, &ws_writefds, &ws_exceptfds, &tv);
  select_printf ("WINSOCK_SELECT returned %d", r);
  if (r == -1)
    {
      select_printf ("error %d", WSAGetLastError ());
      set_winsock_errno ();
      return 0;
    }

  if (WINSOCK_FD_ISSET (h, &ws_readfds) || (me->read_selected && me->read_ready))
    me->read_ready = true;
  if (WINSOCK_FD_ISSET (h, &ws_writefds) || (me->write_selected && me->write_ready))
    me->write_ready = true;
  if (WINSOCK_FD_ISSET (h, &ws_exceptfds) || (me->except_selected && me->except_ready))
    me->except_ready = true;
  return me->read_ready || me->write_ready || me->except_ready;
}

static int start_thread_socket (select_record *, select_stuff *);

static DWORD WINAPI
thread_socket (void *arg)
{
  socketinf *si = (socketinf *)arg;

  select_printf ("stuff_start %p", &si->start);
  int r = WINSOCK_SELECT (0, &si->readfds, &si->writefds, &si->exceptfds, NULL);
  select_printf ("Win32 select returned %d", r);
  if (r == -1)
    select_printf ("error %d", WSAGetLastError ());
  select_record *s = si->start;
  while ((s = s->next))
    if (s->startup == start_thread_socket)
	{
	  HANDLE h = s->fh->get_handle ();
	  select_printf ("s %p, testing fd %d (%s)", s, s->fd, s->fh->get_name ());
	  if (WINSOCK_FD_ISSET (h, &si->readfds))
	    {
	      select_printf ("read_ready");
	      s->read_ready = true;
	    }
	  if (WINSOCK_FD_ISSET (h, &si->writefds))
	    {
	      select_printf ("write_ready");
	      s->write_ready = true;
	    }
	  if (WINSOCK_FD_ISSET (h, &si->exceptfds))
	    {
	      select_printf ("except_ready");
	      s->except_ready = true;
	    }
	}

  if (WINSOCK_FD_ISSET (si->exitsock, &si->readfds))
    select_printf ("saw exitsock read");

  return 0;
}

extern "C" unsigned long htonl (unsigned long);

static int
start_thread_socket (select_record *me, select_stuff *stuff)
{
  socketinf *si;

  if ((si = (socketinf *)stuff->device_specific[FHDEVN(FH_SOCKET)]))
    {
      me->h = si->thread;
      return 1;
    }

  si = new socketinf;
  WINSOCK_FD_ZERO (&si->readfds);
  WINSOCK_FD_ZERO (&si->writefds);
  WINSOCK_FD_ZERO (&si->exceptfds);
  select_record *s = &stuff->start;
  while ((s = s->next))
    if (s->startup == start_thread_socket)
      {
	HANDLE h = s->fh->get_handle ();
	select_printf ("Handle %p", h);
	if (s->read_selected)
	  {
	    WINSOCK_FD_SET (h, &si->readfds);
	    select_printf ("Added to readfds");
	  }
	if (s->write_selected)
	  {
	    WINSOCK_FD_SET (h, &si->writefds);
	    select_printf ("Added to writefds");
	  }
	if (s->except_selected)
	  {
	    WINSOCK_FD_SET (h, &si->exceptfds);
	    select_printf ("Added to exceptfds");
	  }
      }

  if ((si->exitsock = socket (PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
      set_winsock_errno ();
      select_printf ("cannot create socket, %E");
      return -1;
    }
  /* Allow rapid reuse of the port. */
  int tmp = 1;
  (void) setsockopt (si->exitsock, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));

  int sin_len = sizeof(si->sin);
  memset (&si->sin, 0, sizeof (si->sin));
  si->sin.sin_family = AF_INET;
  si->sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  if (bind (si->exitsock, (struct sockaddr *) &si->sin, sizeof (si->sin)) < 0)
    {
      select_printf ("cannot bind socket, %E");
      goto err;
    }

  if (getsockname (si->exitsock, (struct sockaddr *) &si->sin, &sin_len) < 0)
    {
      select_printf ("getsockname error");
      goto err;
    }

  if (listen (si->exitsock, 1))
    {
      select_printf ("listen failed, %E");
      goto err;
    }

  select_printf ("exitsock %p", si->exitsock);
  WINSOCK_FD_SET ((HANDLE) si->exitsock, &si->readfds);
  WINSOCK_FD_SET ((HANDLE) si->exitsock, &si->exceptfds);
  stuff->device_specific[FHDEVN(FH_SOCKET)] = (void *) si;
  si->start = &stuff->start;
  select_printf ("stuff_start %p", &stuff->start);
  si->thread = me->h = makethread (thread_socket, (LPVOID)si, 0,
				  "select_socket");
  return !!me->h;

err:
  set_winsock_errno ();
  closesocket (si->exitsock);
  return -1;
}

void
socket_cleanup (select_record *, select_stuff *stuff)
{
  socketinf *si = (socketinf *)stuff->device_specific[FHDEVN(FH_SOCKET)];
  select_printf ("si %p si->thread %p", si, si ? si->thread : NULL);
  if (si && si->thread)
    {
      select_printf ("connection to si->exitsock %p", si->exitsock);
      SOCKET s = socket (AF_INET, SOCK_STREAM, 0);

      /* Set LINGER with 0 timeout for hard close */
      struct linger tmp = {1, 0}; /* On, 0 delay */
      (void) setsockopt (s, SOL_SOCKET, SO_LINGER, (char *)&tmp, sizeof(tmp));
      (void) setsockopt (si->exitsock, SOL_SOCKET, SO_LINGER, (char *)&tmp, sizeof(tmp));

      /* Connecting to si->exitsock will cause any executing select to wake
	 up.  When this happens then the exitsock condition will cause the
	 thread to terminate. */
      if (connect (s, (struct sockaddr *) &si->sin, sizeof (si->sin)) < 0)
	{
	  set_winsock_errno ();
	  select_printf ("connect failed");
	  /* FIXME: now what? */
	}
      shutdown (s, SD_BOTH);
      closesocket (s);

      /* Wait for thread to go away */
      WaitForSingleObject (si->thread, INFINITE);
      shutdown (si->exitsock, SD_BOTH);
      closesocket (si->exitsock);
      CloseHandle (si->thread);
      stuff->device_specific[FHDEVN(FH_SOCKET)] = NULL;
      delete si;
    }
  select_printf ("returning");
}

select_record *
fhandler_socket::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->peek = peek_socket;
  s->read_ready = saw_shutdown_read ();
  s->read_selected = true;
  return s;
}

select_record *
fhandler_socket::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->peek = peek_socket;
  s->write_ready = saw_shutdown_write ();
  s->write_selected = true;
  return s;
}

select_record *
fhandler_socket::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->peek = peek_socket;
  /* FIXME: Is this right?  Should these be used as criteria for except? */
  s->except_ready = saw_shutdown_write () || saw_shutdown_read ();
  s->except_selected = true;
  return s;
}

static int
peek_windows (select_record *me, bool)
{
  MSG m;
  HANDLE h;
  set_handle_or_return_if_not_open (h, me);

  if (me->read_selected && me->read_ready)
    return 1;

  if (PeekMessage (&m, (HWND) h, 0, 0, PM_NOREMOVE))
    {
      me->read_ready = true;
      select_printf ("window %d(%p) ready", me->fd, me->fh->get_handle ());
      return 1;
    }

  select_printf ("window %d(%p) not ready", me->fd, me->fh->get_handle ());
  return me->write_ready;
}

static int
verify_windows (select_record *me, fd_set *rfds, fd_set *wfds,
		fd_set *efds)
{
  return peek_windows (me, true);
}

select_record *
fhandler_windows::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
    }
  s->verify = verify_windows;
  s->peek = peek_windows;
  s->read_selected = true;
  s->read_ready = false;
  s->h = get_handle ();
  s->windows_handle = true;
  return s;
}

select_record *
fhandler_windows::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->peek = peek_windows;
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  s->windows_handle = true;
  return s;
}

select_record *
fhandler_windows::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->peek = peek_windows;
  s->h = get_handle ();
  s->except_selected = true;
  s->except_ready = true;
  s->windows_handle = true;
  return s;
}
