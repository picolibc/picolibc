/* select.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

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
#define Win32_Winsock

#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/time.h>

#include "winsup.h"
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <winsock.h>
#include "select.h"

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

#define NULL_fd_set ((fd_set *)NULL)
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

#define allocfd_set(n) ((fd_set *) alloca (sizeof_fd_set (n)))
#define copyfd_set(to, from, n) memcpy (to, from, sizeof_fd_set (n));

/* Make a fhandler_foo::ready_for_ready method.
   Assumption: The "ready_for_read" methods are called with one level of
   signal blocking. */
#define MAKEready(what) \
int \
fhandler_##what::ready_for_read (int fd, DWORD howlong, int ignra) \
{ \
  select_record me (this); \
  me.fd = fd; \
  (void) select_read (&me); \
  while (!peek_##what (&me, ignra) && howlong == INFINITE) \
    if (fd >= 0 && dtable.not_open (fd)) \
      break; \
    else if (WaitForSingleObject (signal_arrived, 10) == WAIT_OBJECT_0) \
      break; \
  return me.read_ready; \
}

#define set_handle_or_return_if_not_open(h, s) \
  h = (s)->fh->get_handle (); \
  if (dtable.not_open ((s)->fd)) \
    { \
      (s)->saw_error = TRUE; \
      set_errno (EBADF); \
      return -1; \
    } \

/* The main select code.
 */
extern "C"
int
cygwin_select (int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		     struct timeval *to)
{
  select_stuff sel;
  fd_set *dummy_readfds = allocfd_set (n);
  fd_set *dummy_writefds = allocfd_set (n);
  fd_set *dummy_exceptfds = allocfd_set (n);
  sigframe thisframe (mainthread, 0);

#if 0
  if (n > FD_SETSIZE)
    {
      set_errno (EINVAL);
      return -1;
    }
#endif

  select_printf ("%d, %p, %p, %p, %p", n, readfds, writefds, exceptfds, to);

  if (!readfds)
    {
      UNIX_FD_ZERO (dummy_readfds, n);
      readfds = dummy_readfds;
    }
  if (!writefds)
    {
      UNIX_FD_ZERO (dummy_writefds, n);
      writefds = dummy_writefds;
    }
  if (!exceptfds)
    {
      UNIX_FD_ZERO (dummy_exceptfds, n);
      exceptfds = dummy_exceptfds;
    }

  for (int i = 0; i < n; i++)
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

  /* Degenerate case.  No fds to wait for.  Just wait. */
  if (sel.start.next == NULL)
    {
      if (WaitForSingleObject (signal_arrived, ms) == WAIT_OBJECT_0)
	{
	  select_printf ("signal received");
	  set_sig_errno (EINTR);
	  return -1;
	}
      return 0;
    }

  /* If one of the selected fds is "always ready" just poll everything and return
     the result.  There is no need to wait. */
  if (sel.always_ready || ms == 0)
    {
      UNIX_FD_ZERO (readfds, n);
      UNIX_FD_ZERO (writefds, n);
      UNIX_FD_ZERO (exceptfds, n);
      return sel.poll (readfds, writefds, exceptfds);
    }

  /* Wait for an fd to come alive */
  return sel.wait (readfds, writefds, exceptfds, ms);
}

/* Cleanup */
select_stuff::~select_stuff ()
{
  select_record *s = &start;

  select_printf ("calling cleanup routines");
  while ((s = s->next))
    if (s->cleanup)
      s->cleanup (s, this);

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
  if (UNIX_FD_ISSET (i, readfds) && (s = dtable.select_read (i, s)) == NULL)
    return 0; /* error */
  if (UNIX_FD_ISSET (i, writefds) && (s = dtable.select_write (i, s)) == NULL)
    return 0; /* error */
  if (UNIX_FD_ISSET (i, exceptfds) && (s = dtable.select_except (i, s)) == NULL)
    return 0; /* error */
  if (s == NULL)
    return 1; /* nothing to do */

  if (s->read_ready || s->write_ready || s->except_ready)
    always_ready = TRUE;

  if (s->windows_handle || s->windows_handle || s->windows_handle)
    windows_used = TRUE;

  s->next = start.next;
  start.next = s;
  return 1;
}

/* Poll every fd in the select chain.  Set appropriate fd in mask. */
int
select_stuff::poll (fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
  int n = 0;
  select_record *s = &start;
  while ((s = s->next))
    n += s->poll (s, readfds, writefds, exceptfds);
  select_printf ("returning %d", n);
  return n;
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

  w4[m++] = signal_arrived;  /* Always wait for the arrival of a signal. */
  /* Loop through the select chain, starting up anything appropriate and
     counting the number of active fds. */
  while ((s = s->next))
    {
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

  int n = m - 1;
  DWORD start_time = GetTickCount ();	/* Record the current time for later use. */

  /* Allocate some fd_set structures using the number of fds as a guide. */
  fd_set *r = allocfd_set (n);
  fd_set *w = allocfd_set (n);
  fd_set *e = allocfd_set (n);
  UNIX_FD_ZERO (r, n);
  UNIX_FD_ZERO (w, n);
  UNIX_FD_ZERO (e, n);
  debug_printf ("n %d, ms %u", n, ms);
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
	  goto out;
      }

      select_printf ("woke up.  wait_ret %d.  verifying", wait_ret);
      s = &start;
      int gotone = FALSE;
      while ((s = s->next))
	if (s->saw_error)
	  return -1;		/* Somebody detected an error */
	else if ((((wait_ret >= m && s->windows_handle) || s->h == w4[wait_ret])) &&
	    s->verify (s, r, w, e))
	  gotone = TRUE;

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
  copyfd_set (readfds, r, n);
  copyfd_set (writefds, w, n);
  copyfd_set (exceptfds, e, n);

  return poll (readfds, writefds, exceptfds);
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
  if (me->except_ready && me->except_ready)
    {
      UNIX_FD_SET (me->fd, exceptfds);
      ready++;
    }
  select_printf ("ready %d", ready);
  return ready;
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
peek_pipe (select_record *s, int ignra)
{
  int n = 0;
  int gotone = 0;
  fhandler_base *fh = s->fh;

  HANDLE h;
  set_handle_or_return_if_not_open (h, s);

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
      if (fh->bg_check (SIGTTIN) <= 0)
	{
	  gotone = s->read_ready = 1;
	  goto out;
	}

      if (!ignra && fh->get_device () != FH_PTYM && fh->get_device () != FH_TTYM &&
	  fh->get_readahead_valid ())
	{
	  select_printf ("readahead");
	  gotone = s->read_ready = 1;
	  goto out;
	}
    }

  if (fh->get_device() != FH_PIPEW &&
      !PeekNamedPipe (h, NULL, 0, NULL, (DWORD *) &n, NULL))
    {
      select_printf ("%s, PeekNamedPipe failed, %E", fh->get_name ());
      n = -1;
    }

  if (n < 0)
    {
      select_printf ("%s, n %d", fh->get_name (), n);
      if (s->except_selected)
	gotone += s->except_ready = TRUE;
      if (s->read_selected)
	gotone += s->read_ready = TRUE;
    }
  if (n > 0 && s->read_selected)
    {
      select_printf ("%s, ready for read", fh->get_name ());
      gotone += s->read_ready = TRUE;
    }
  if (!gotone && s->fh->hit_eof ())
    {
      select_printf ("%s, saw EOF", fh->get_name ());
      if (s->except_selected)
	gotone = s->except_ready = TRUE;
      if (s->read_selected)
	gotone += s->read_ready = TRUE;
      select_printf ("saw eof on '%s'", fh->get_name ());
    }

out:
  return gotone || s->write_ready;
}

static int
poll_pipe (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)
{
  return peek_pipe (me, 0) ?
	 set_bits (me, readfds, writefds, exceptfds) :
	 0;
}

MAKEready(pipe)

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
	    if (peek_pipe (s, 0))
	      gotone = TRUE;
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
      pi->stop_thread_pipe = TRUE;
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
  s->poll = poll_pipe;
  s->verify = verify_ok;
  s->read_selected = TRUE;
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
      s->poll = poll_pipe;
      s->verify = no_verify;
    }
  s->write_selected = TRUE;
  s->write_ready = TRUE;
  return s;
}

select_record *
fhandler_pipe::select_except (select_record *s)
{
  if (!s)
      s = new select_record;
  s->startup = start_thread_pipe;
  s->poll = poll_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->except_selected = TRUE;
  return s;
}

static int
peek_console (select_record *me, int ignra)
{
  extern const char * get_nonascii_key (INPUT_RECORD& input_rec);
  fhandler_console *fh = (fhandler_console *)me->fh;

  if (!me->read_selected)
    return me->write_ready;

  if (!ignra && fh->get_readahead_valid ())
    {
      select_printf ("readahead");
      return me->read_ready = 1;
    }

  if (me->read_ready)
    {
      select_printf ("already ready");
      return 1;
    }

  INPUT_RECORD irec;
  DWORD events_read;
  HANDLE h;
  set_handle_or_return_if_not_open (h, me);

  for (;;)
    if (fh->bg_check (SIGTTIN) <= 0)
      return me->read_ready = 1;
    else if (!PeekConsoleInput (h, &irec, 1, &events_read) || !events_read)
      break;
    else
      {
	if (irec.EventType == WINDOW_BUFFER_SIZE_EVENT)
	  kill_pgrp (fh->tc->getpgid (), SIGWINCH);
	else if (irec.EventType == KEY_EVENT && irec.Event.KeyEvent.bKeyDown == TRUE &&
		 (irec.Event.KeyEvent.uChar.AsciiChar || get_nonascii_key (irec)))
	  return me->read_ready = 1;

	/* Read and discard the event */
	ReadConsoleInput (h, &irec, 1, &events_read);
      }

  return me->write_ready;
}

static int
poll_console (select_record *me, fd_set *readfds, fd_set *writefds,
	      fd_set *exceptfds)
{
  return peek_console (me, 0) ?
	 set_bits (me, readfds, writefds, exceptfds) :
	 0;
}

MAKEready (console)

select_record *
fhandler_console::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = poll_console;
      s->verify = poll_console;
    }

  s->h = get_handle ();
  s->read_selected = TRUE;
  return s;
}

select_record *
fhandler_console::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = poll_console;
      s->verify = no_verify;
    }

  s->write_selected = TRUE;
  s->write_ready = TRUE;
  return s;
}

select_record *
fhandler_console::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = poll_console;
      s->verify = no_verify;
    }

  s->except_selected = TRUE;
  return s;
}

int
fhandler_tty_common::ready_for_read (int fd, DWORD howlong, int ignra)
{
#if 0
  if (myself->pgid && get_ttyp ()->getpgid () != myself->pgid &&
	myself->ctty == ttynum) // background process?
    return 1;	// Yes. Let read return an error
#endif
  return ((fhandler_pipe*)this)->fhandler_pipe::ready_for_read (fd, howlong, ignra);
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

select_record *
fhandler_dev_null::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->read_selected = TRUE;
  return s;
}

select_record *
fhandler_dev_null::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->write_selected = TRUE;
  return s;
}

select_record *
fhandler_dev_null::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->except_selected = TRUE;
  s->except_ready = TRUE;
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
peek_serial (select_record *s, int)
{
  DWORD ev;
  COMSTAT st;

  fhandler_serial *fh = (fhandler_serial *)s->fh;

  if (fh->get_readahead_valid () || fh->overlapped_armed < 0)
    return s->read_ready = 1;

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
      DWORD ev;
      COMSTAT st;

      ResetEvent (fh->io_status.hEvent);

      if (!ClearCommError (h, &ev, &st))
	{
	  debug_printf ("ClearCommError");
	  goto err;
	}
      else if (st.cbInQue)
	return s->read_ready = 1;
      else if (WaitCommEvent (h, &ev, &fh->io_status))
	return s->read_ready = 1;
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
      if (!ClearCommError (h, &ev, &st))
        {
          debug_printf ("ClearCommError");
          goto err;
        }
      else if (!st.cbInQue)
	Sleep (to);
      else
	{
	  return s->read_ready = 1;
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
  s->saw_error = TRUE;
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
	    if (peek_serial (s, 0))
	      gotone = TRUE;
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
      si->stop_thread_serial = TRUE;
      WaitForSingleObject (si->thread, INFINITE);
      CloseHandle (si->thread);
      delete si;
      stuff->device_specific[FHDEVN(FH_SERIAL)] = NULL;
    }
}

static int
poll_serial (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)

{
  return peek_serial (me, 0) ?
	 set_bits (me, readfds, writefds, exceptfds) :
	 0;
}

MAKEready (serial)

select_record *
fhandler_serial::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_serial;
      s->poll = poll_serial;
      s->verify = verify_ok;
      s->cleanup = serial_cleanup;
    }
  s->read_selected = TRUE;
  return s;
}

select_record *
fhandler_serial::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->write_selected = TRUE;
  s->write_ready = TRUE;
  return s;
}

select_record *
fhandler_serial::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = NULL;
  s->except_selected = FALSE;	// Can't do this
  return s;
}

int
fhandler_base::ready_for_read (int, DWORD, int)
{
  return 1;
}

select_record *
fhandler_base::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->read_selected = TRUE;
  s->read_ready = TRUE;
  return s;
}

select_record *
fhandler_base::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->write_selected = TRUE;
  s->write_ready = TRUE;
  return s;
}

select_record *
fhandler_base::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = NULL;
  s->write_selected = TRUE;
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
peek_socket (select_record *me, int)
{
  winsock_fd_set ws_readfds, ws_writefds, ws_exceptfds;
  struct timeval tv = {0, 0};
  WINSOCK_FD_ZERO (&ws_readfds);
  WINSOCK_FD_ZERO (&ws_writefds);
  WINSOCK_FD_ZERO (&ws_exceptfds);
  int gotone = 0;

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
      return 0;
    }

  if (WINSOCK_FD_ISSET (h, &ws_readfds) || (me->read_selected && me->read_ready))
    gotone = me->read_ready = TRUE;
  if (WINSOCK_FD_ISSET (h, &ws_writefds) || (me->write_selected && me->write_ready))
    gotone = me->write_ready = TRUE;
  if (WINSOCK_FD_ISSET (h, &ws_exceptfds) || (me->except_selected && me->except_ready))
    gotone = me->except_ready = TRUE;
  return gotone;
}

static int
poll_socket (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)
{
  return peek_socket (me, 0) ?
	 set_bits (me, readfds, writefds, exceptfds) :
	 0;
}

MAKEready (socket)

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
	      s->read_ready = TRUE;
	    }
	  if (WINSOCK_FD_ISSET (h, &si->writefds))
	    {
	      select_printf ("write_ready");
	      s->write_ready = TRUE;
	    }
	  if (WINSOCK_FD_ISSET (h, &si->exceptfds))
	    {
	      select_printf ("except_ready");
	      s->except_ready = TRUE;
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
      /* Connecting to si->exitsock will cause any executing select to wake
	 up.  When this happens then the exitsock condition will cause the
	 thread to terminate. */
      if (connect (s, (struct sockaddr *) &si->sin, sizeof (si->sin)) < 0)
	{
	  set_winsock_errno ();
	  select_printf ("connect failed");
	  /* FIXME: now what? */
	}
      closesocket (s);

      /* Wait for thread to go away */
      WaitForSingleObject (si->thread, INFINITE);
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
      s->poll = poll_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->read_selected = TRUE;
  return s;
}

select_record *
fhandler_socket::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_socket;
      s->poll = poll_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->write_selected = TRUE;
  return s;
}

select_record *
fhandler_socket::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = start_thread_socket;
      s->poll = poll_socket;
      s->verify = verify_true;
      s->cleanup = socket_cleanup;
    }
  s->except_selected = TRUE;
  return s;
}

static int
peek_windows (select_record *me, int)
{
  MSG m;
  HANDLE h;
  set_handle_or_return_if_not_open (h, me);

  if (me->read_selected && me->read_ready)
    return 1;

  if (PeekMessage (&m, (HWND) h, 0, 0, PM_NOREMOVE))
    {
      me->read_ready = TRUE;
      select_printf ("window %d(%p) ready", me->fd, me->fh->get_handle ());
      return 1;
    }

  select_printf ("window %d(%p) not ready", me->fd, me->fh->get_handle ());
  return me->write_ready;
}

static int
poll_windows (select_record *me, fd_set *readfds, fd_set *writefds,
	      fd_set *exceptfds)
{

  return peek_windows (me, 0) ?
	 set_bits (me, readfds, writefds, exceptfds) :
	 0;
}

MAKEready (windows)

select_record *
fhandler_windows::select_read (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = poll_windows;
      s->verify = poll_windows;
    }
  s->h = get_handle ();
  s->read_selected = TRUE;
  s->h = get_handle ();
  s->windows_handle = TRUE;
  return s;
}

select_record *
fhandler_windows::select_write (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->write_selected = TRUE;
  s->write_ready = TRUE;
  s->windows_handle = TRUE;
  return s;
}

select_record *
fhandler_windows::select_except (select_record *s)
{
  if (!s)
    {
      s = new select_record;
      s->startup = no_startup;
      s->poll = set_bits;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->except_selected = TRUE;
  s->except_ready = TRUE;
  s->windows_handle = TRUE;
  return s;
}
