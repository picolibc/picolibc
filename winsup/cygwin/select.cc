/* select.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* The following line means that the BSD socket definitions for
   fd_set, FD_ISSET etc. are used in this file.  */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <stdlib.h>
#include "ntdll.h"

#include <wingdi.h>
#include <winuser.h>
#include <netdb.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "sigproc.h"
#include "cygtls.h"

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
  ((unsigned) (NULL_fd_set->fds_bits + unix_howmany ((n), UNIX_NFDBITS)))
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
      (s)->thread_errno =  EBADF; \
      return -1; \
    } \

/* The main select code.
 */
extern "C" int
cygwin_select (int maxfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	       struct timeval *to)
{
  select_stuff sel;
  fd_set *dummy_readfds = allocfd_set (maxfds);
  fd_set *dummy_writefds = allocfd_set (maxfds);
  fd_set *dummy_exceptfds = allocfd_set (maxfds);

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

extern "C" int
pselect(int maxfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	const struct timespec *ts, const sigset_t *set)
{
  struct timeval tv;
  sigset_t oldset = _my_tls.sigmask;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (ts)
    {
      tv.tv_sec = ts->tv_sec;
      tv.tv_usec = ts->tv_nsec / 1000;
    }
  if (set)
    set_signal_mask (*set, _my_tls.sigmask);
  int ret = cygwin_select (maxfds, readfds, writefds, exceptfds,
			   ts ? &tv : NULL);
  if (set)
    set_signal_mask (oldset, _my_tls.sigmask);
  return ret;
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

  if (s->windows_handle)
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
      if (m >= MAXIMUM_WAIT_OBJECTS)
	{
	  set_sig_errno (EINVAL);
	  return -1;
	}
      if (!s->startup (s, this))
	{
	  s->set_select_errno ();
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

  LONGLONG start_time = gtod.msecs ();	/* Record the current time for later use. */

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
	  s->set_select_errno ();
	  return -1;
	case WAIT_TIMEOUT:
	  select_printf ("timed out");
	  res = 1;
	  goto out;
      }

      select_printf ("woke up.  wait_ret %d.  verifying", wait_ret);
      s = &start;
      bool gotone = false;
      /* Some types of objects (e.g., consoles) wake up on "inappropriate" events
	 like mouse movements.  The verify function will detect these situations.
	 If it returns false, then this wakeup was a false alarm and we should go
	 back to waiting. */
      while ((s = s->next))
	if (s->saw_error ())
	  {
	    set_errno (s->saw_error ());
	    return -1;		/* Somebody detected an error */
	  }
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

      LONGLONG now = gtod.msecs ();
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
  fhandler_socket *sock;
  select_printf ("me %p, testing fd %d (%s)", me, me->fd, me->fh->get_name ());
  if (me->read_selected && me->read_ready)
    {
      UNIX_FD_SET (me->fd, readfds);
      ready++;
    }
  if (me->write_selected && me->write_ready)
    {
      UNIX_FD_SET (me->fd, writefds);
      if (me->except_on_write && (sock = me->fh->is_socket ()))
	{
	  /* Special AF_LOCAL handling. */
	  if (!me->read_ready && sock->connect_state () == connect_pending
	      && sock->af_local_connect ())
	    {
	      if (me->read_selected)
		UNIX_FD_SET (me->fd, readfds);
	      sock->connect_state (connect_failed);
	    }
	  else
	    sock->connect_state (connected);
	}
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

  /* Don't perform complicated tests if we don't need to. */
  if (!s->read_selected && !s->except_selected)
    goto out;

  if (s->read_selected)
    {
      if (s->read_ready)
	{
	  select_printf ("%s, already ready for read", fh->get_name ());
	  gotone = 1;
	  goto out;
	}

      switch (fh->get_major ())
	{
	case DEV_TTYM_MAJOR:
	  if (((fhandler_pty_master *) fh)->need_nl)
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
    select_printf ("%s, select for read/except on write end of pipe",
		   fh->get_name ());
  else if (!PeekNamedPipe (h, NULL, 0, NULL, (DWORD *) &n, NULL))
    {
      select_printf ("%s, PeekNamedPipe failed, %E", fh->get_name ());
      n = -1;
    }

  if (n < 0)
    {
      select_printf ("%s, n %d", fh->get_name (), n);
      if (s->except_selected)
	gotone += s->except_ready = true;
      if (s->read_selected)
	gotone += s->read_ready = true;
    }
  if (n > 0 && s->read_selected)
    {
      select_printf ("%s, ready for read: avail %d", fh->get_name (), n);
      gotone += s->read_ready = true;
    }
  if (!gotone && s->fh->hit_eof ())
    {
      select_printf ("%s, saw EOF", fh->get_name ());
      if (s->except_selected)
	gotone += s->except_ready = true;
      if (s->read_selected)
	gotone += s->read_ready = true;
    }

out:
  if (s->write_selected)
    {
      if (s->write_ready)
	{
	  select_printf ("%s, already ready for write", fh->get_name ());
	  gotone++;
	}
      /* Do we need to do anything about SIGTTOU here? */
      else if (fh->get_device () == FH_PIPER)
	select_printf ("%s, select for write on read end of pipe",
		       fh->get_name ());
      else
	{
#if 0
/* FIXME: This code is not quite correct.  There's no better solution
   so far but to make simple assumptions based on WriteQuotaAvailable. */

	  IO_STATUS_BLOCK iosb = {0};
	  FILE_PIPE_LOCAL_INFORMATION fpli = {0};

	  if (NtQueryInformationFile (h,
				      &iosb,
				      &fpli,
				      sizeof (fpli),
				      FilePipeLocalInformation))
	    {
	      /* If NtQueryInformationFile fails, optimistically assume the
		 pipe is writable.  This could happen if we somehow
		 inherit a pipe that doesn't permit FILE_READ_ATTRIBUTES
		 access on the write end.  */
	      select_printf ("%s, NtQueryInformationFile failed",
			     fh->get_name ());
	      gotone += s->write_ready = true;
	    }
	  /* If there is anything available in the pipe buffer then signal
	     that.  This means that a pipe could still block since you could
	     be trying to write more to the pipe than is available in the
	     buffer but that is the hazard of select().  */
	  else if ((fpli.WriteQuotaAvailable = (fpli.OutboundQuota - fpli.ReadDataAvailable)))
	    {
	      select_printf ("%s, ready for write: size %lu, avail %lu",
			     fh->get_name (),
			     fpli.OutboundQuota,
			     fpli.WriteQuotaAvailable);
	      gotone += s->write_ready = true;
	    }
	  /* If we somehow inherit a tiny pipe (size < PIPE_BUF), then consider
	     the pipe writable only if it is completely empty, to minimize the
	     probability that a subsequent write will block.  */
	  else if (fpli.OutboundQuota < PIPE_BUF &&
		   fpli.WriteQuotaAvailable == fpli.OutboundQuota)
	    {
	      select_printf ("%s, tiny pipe: size %lu, avail %lu",
			     fh->get_name (),
			     fpli.OutboundQuota,
			     fpli.WriteQuotaAvailable);
	      gotone += s->write_ready = true;
	    }
#else
	  gotone += s->write_ready = true;
#endif
	}
    }

  return gotone;
}

static int start_thread_pipe (select_record *me, select_stuff *stuff);

struct pipeinf
  {
    cygthread *thread;
    bool stop_thread_pipe;
    select_record *start;
  };

static DWORD WINAPI
thread_pipe (void *arg)
{
  pipeinf *pi = (pipeinf *) arg;
  bool gotone = false;
  DWORD sleep_time = 0;

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
      Sleep (sleep_time >> 3);
      if (sleep_time < 80)
	++sleep_time;
    }
out:
  return 0;
}

static int
start_thread_pipe (select_record *me, select_stuff *stuff)
{
  if (stuff->device_specific_pipe)
    {
      me->h = *((pipeinf *) stuff->device_specific_pipe)->thread;
      return 1;
    }
  pipeinf *pi = new pipeinf;
  pi->start = &stuff->start;
  pi->stop_thread_pipe = false;
  pi->thread = new cygthread (thread_pipe, 0, pi, "select_pipe");
  me->h = *pi->thread;
  if (!me->h)
    return 0;
  stuff->device_specific_pipe = (void *) pi;
  return 1;
}

static void
pipe_cleanup (select_record *, select_stuff *stuff)
{
  pipeinf *pi = (pipeinf *) stuff->device_specific_pipe;
  if (pi && pi->thread)
    {
      pi->stop_thread_pipe = true;
      pi->thread->detach ();
      delete pi;
      stuff->device_specific_pipe = NULL;
    }
}

int
fhandler_pipe::ready_for_read (int fd, DWORD howlong)
{
  int res;
  if (!howlong)
    res = fhandler_base::ready_for_read (fd, howlong);
  else
    res = 1;
  return res;
}

select_record *
fhandler_pipe::select_read (select_record *s)
{
  if (!s)
    s = new select_record;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_pipe::select_write (select_record *s)
{
  if (!s)
    s = new select_record;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->write_selected = true;
  s->write_ready = false;
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
  fhandler_console *fh = (fhandler_console *) me->fh;

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
	if (irec.EventType == KEY_EVENT)
	  {
	    if (irec.Event.KeyEvent.bKeyDown
		&& (irec.Event.KeyEvent.uChar.AsciiChar
		    || get_nonascii_key (irec, tmpbuf)))
	      return me->read_ready = true;
	  }
	else
	  {
	    fh->send_winch_maybe ();
	    if (irec.EventType == MOUSE_EVENT
		&& fh->mouse_aware ()
		&& (irec.Event.MouseEvent.dwEventFlags == 0
		    || irec.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK))
		return me->read_ready = true;
	  }

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
  return ((fhandler_pipe *) this)->fhandler_pipe::select_read (s);
}

select_record *
fhandler_tty_common::select_write (select_record *s)
{
  return ((fhandler_pipe *) this)->fhandler_pipe::select_write (s);
}

select_record *
fhandler_tty_common::select_except (select_record *s)
{
  return ((fhandler_pipe *) this)->fhandler_pipe::select_except (s);
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
  s->except_ready = false;
  return s;
}

static int start_thread_serial (select_record *me, select_stuff *stuff);

struct serialinf
  {
    cygthread *thread;
    bool stop_thread_serial;
    select_record *start;
  };

static int
peek_serial (select_record *s, bool)
{
  COMSTAT st;

  fhandler_serial *fh = (fhandler_serial *) s->fh;

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

  SetCommMask (h, EV_RXCHAR);

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
      break;
    case WAIT_OBJECT_0 + 1:
      select_printf ("interrupt");
      set_sig_errno (EINTR);
      ready = -1;
      break;
    case WAIT_TIMEOUT:
      break;
    default:
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

  s->set_select_errno ();
  select_printf ("error %E");
  return -1;
}

static DWORD WINAPI
thread_serial (void *arg)
{
  serialinf *si = (serialinf *) arg;
  bool gotone = false;

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
  if (stuff->device_specific_serial)
    {
      me->h = *((serialinf *) stuff->device_specific_serial)->thread;
      return 1;
    }
  serialinf *si = new serialinf;
  si->start = &stuff->start;
  si->stop_thread_serial = false;
  si->thread = new cygthread (thread_serial, 0,  si, "select_serial");
  me->h = *si->thread;
  stuff->device_specific_serial = (void *) si;
  return 1;
}

static void
serial_cleanup (select_record *, select_stuff *stuff)
{
  serialinf *si = (serialinf *) stuff->device_specific_serial;
  if (si && si->thread)
    {
      si->stop_thread_serial = true;
      si->thread->detach ();
      delete si;
      stuff->device_specific_serial = NULL;
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
  bool avail = false;
  select_record me (this);
  me.fd = fd;
  while (!avail)
    {
      select_read (&me);
      avail = me.read_ready ?: me.peek (&me, false);

      if (fd >= 0 && cygheap->fdtab.not_open (fd))
	{
	  set_sig_errno (EBADF);
	  avail = false;
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
	  debug_printf ("interrupted");
	  set_sig_errno (EINTR);
	  avail = false;
	  break;
	}
    }

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

static int
peek_socket (select_record *me, bool)
{
  fhandler_socket *fh = (fhandler_socket *) me->fh;
  long events;
  /* Don't play with the settings again, unless having taken a deep look into
     Richard W. Stevens Network Programming book.  Thank you. */
  long evt_mask = (me->read_selected ? (FD_READ | FD_ACCEPT | FD_CLOSE) : 0)
		| (me->write_selected ? (FD_WRITE | FD_CONNECT | FD_CLOSE) : 0)
		| (me->except_selected ? FD_OOB : 0);
  int ret = fh->evaluate_events (evt_mask, events, false);
  if (me->read_selected)
    me->read_ready |= ret || !!(events & (FD_READ | FD_ACCEPT | FD_CLOSE));
  if (me->write_selected)
    me->write_ready |= ret || !!(events & (FD_WRITE | FD_CONNECT | FD_CLOSE));
  if (me->except_selected)
    me->except_ready |= !!(events & FD_OOB);

  select_printf ("read_ready: %d, write_ready: %d, except_ready: %d",
		 me->read_ready, me->write_ready, me->except_ready);
  return me->read_ready || me->write_ready || me->except_ready;
}

static int start_thread_socket (select_record *, select_stuff *);

struct socketinf
  {
    cygthread *thread;
    int max_w4;
    int num_w4;
    LONG *ser_num;
    HANDLE *w4;
    select_record *start;
  };

static DWORD WINAPI
thread_socket (void *arg)
{
  socketinf *si = (socketinf *) arg;
  DWORD timeout = 64 / (si->max_w4 / MAXIMUM_WAIT_OBJECTS);
  bool event = false;

  select_printf ("stuff_start %p", si->start);
  while (!event)
    {
      for (select_record *s = si->start; (s = s->next); )
	if (s->startup == start_thread_socket)
	  if (peek_socket (s, false))
	    event = true;
      if (!event)
	for (int i = 0; i < si->max_w4; i += MAXIMUM_WAIT_OBJECTS)
	  switch (WaitForMultipleObjects (min (si->num_w4 - i,
					       MAXIMUM_WAIT_OBJECTS),
					  si->w4 + i, FALSE, timeout))
	    {
	    case WAIT_FAILED:
	      goto out;
	    case WAIT_OBJECT_0:
	      if (!i)	/* Socket event set. */
		goto out;
	      break;
	    case WAIT_TIMEOUT:
	    default:
	      break;
	    }
    }
out:
  select_printf ("leaving thread_socket");
  return 0;
}

static int
start_thread_socket (select_record *me, select_stuff *stuff)
{
  socketinf *si;

  if ((si = (socketinf *) stuff->device_specific_socket))
    {
      me->h = *si->thread;
      return 1;
    }

  si = new socketinf;
  si->ser_num = (LONG *) malloc (MAXIMUM_WAIT_OBJECTS * sizeof (LONG));
  si->w4 = (HANDLE *) malloc (MAXIMUM_WAIT_OBJECTS * sizeof (HANDLE));
  if (!si->ser_num || !si->w4)
    return 0;
  si->max_w4 = MAXIMUM_WAIT_OBJECTS;
  select_record *s = &stuff->start;
  if (_my_tls.locals.select_sockevt != INVALID_HANDLE_VALUE)
    si->w4[0] = _my_tls.locals.select_sockevt;
  else if (!(si->w4[0] = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL)))
    return 1;
  else
    _my_tls.locals.select_sockevt = si->w4[0];
  si->num_w4 = 1;
  while ((s = s->next))
    if (s->startup == start_thread_socket)
      {
	/* No event/socket should show up multiple times.  Every socket
	   is uniquely identified by its serial number in the global
	   wsock_events record. */
	const LONG ser_num = ((fhandler_socket *) s->fh)->serial_number ();
	for (int i = 1; i < si->num_w4; ++i)
	  if (si->ser_num[i] == ser_num)
	    goto continue_outer_loop;
	if (si->num_w4 >= si->max_w4)
	  {
	    LONG *nser = (LONG *) realloc (si->ser_num,
					   (si->max_w4 + MAXIMUM_WAIT_OBJECTS)
					   * sizeof (LONG));
	    if (!nser)
	      return 0;
	    si->ser_num = nser;
	    HANDLE *nw4 = (HANDLE *) realloc (si->w4,
					   (si->max_w4 + MAXIMUM_WAIT_OBJECTS)
					   * sizeof (HANDLE));
	    if (!nw4)
	      return 0;
	    si->w4 = nw4;
	    si->max_w4 += MAXIMUM_WAIT_OBJECTS;
	  }
	si->ser_num[si->num_w4] = ser_num;
	si->w4[si->num_w4++] = ((fhandler_socket *) s->fh)->wsock_event ();
      continue_outer_loop:
	;
      }
  stuff->device_specific_socket = (void *) si;
  si->start = &stuff->start;
  select_printf ("stuff_start %p", &stuff->start);
  si->thread = new cygthread (thread_socket, 0,  si, "select_socket");
  me->h = *si->thread;
  return 1;
}

void
socket_cleanup (select_record *, select_stuff *stuff)
{
  socketinf *si = (socketinf *) stuff->device_specific_socket;
  select_printf ("si %p si->thread %p", si, si ? si->thread : NULL);
  if (si && si->thread)
    {
      SetEvent (si->w4[0]);
      /* Wait for thread to go away */
      si->thread->detach ();
      ResetEvent (si->w4[0]);
      stuff->device_specific_socket = NULL;
      if (si->ser_num)
	free (si->ser_num);
      if (si->w4)
	free (si->w4);
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
  s->write_ready = saw_shutdown_write () || connect_state () == unconnected;
  s->write_selected = true;
  if (connect_state () != unconnected)
    {
      s->except_ready = saw_shutdown_write () || saw_shutdown_read ();
      s->except_on_write = true;
    }
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
  s->except_ready = false;
  s->windows_handle = true;
  return s;
}

static int
peek_mailslot (select_record *me, bool)
{
  HANDLE h;
  set_handle_or_return_if_not_open (h, me);

  if (me->read_selected && me->read_ready)
    return 1;
  DWORD msgcnt = 0;
  if (!GetMailslotInfo (h, NULL, NULL, &msgcnt, NULL))
    {
      select_printf ("mailslot %d(%p) error %E", me->fd, h);
      return 1;
    }
  if (msgcnt > 0)
    {
      me->read_ready = true;
      select_printf ("mailslot %d(%p) ready", me->fd, h);
      return 1;
    }
  select_printf ("mailslot %d(%p) not ready", me->fd, h);
  return 0;
}

static int
verify_mailslot (select_record *me, fd_set *rfds, fd_set *wfds,
		 fd_set *efds)
{
  return peek_mailslot (me, true);
}

static int start_thread_mailslot (select_record *me, select_stuff *stuff);

struct mailslotinf
  {
    cygthread *thread;
    bool stop_thread_mailslot;
    select_record *start;
  };

static DWORD WINAPI
thread_mailslot (void *arg)
{
  mailslotinf *mi = (mailslotinf *) arg;
  bool gotone = false;
  DWORD sleep_time = 0;

  for (;;)
    {
      select_record *s = mi->start;
      while ((s = s->next))
	if (s->startup == start_thread_mailslot)
	  {
	    if (peek_mailslot (s, true))
	      gotone = true;
	    if (mi->stop_thread_mailslot)
	      {
		select_printf ("stopping");
		goto out;
	      }
	  }
      /* Paranoid check */
      if (mi->stop_thread_mailslot)
	{
	  select_printf ("stopping from outer loop");
	  break;
	}
      if (gotone)
	break;
      Sleep (sleep_time >> 3);
      if (sleep_time < 80)
	++sleep_time;
    }
out:
  return 0;
}

static int
start_thread_mailslot (select_record *me, select_stuff *stuff)
{
  if (stuff->device_specific_mailslot)
    {
      me->h = *((mailslotinf *) stuff->device_specific_mailslot)->thread;
      return 1;
    }
  mailslotinf *mi = new mailslotinf;
  mi->start = &stuff->start;
  mi->stop_thread_mailslot = false;
  mi->thread = new cygthread (thread_mailslot, 0,  mi, "select_mailslot");
  me->h = *mi->thread;
  if (!me->h)
    return 0;
  stuff->device_specific_mailslot = (void *) mi;
  return 1;
}

static void
mailslot_cleanup (select_record *, select_stuff *stuff)
{
  mailslotinf *mi = (mailslotinf *) stuff->device_specific_mailslot;
  if (mi && mi->thread)
    {
      mi->stop_thread_mailslot = true;
      mi->thread->detach ();
      delete mi;
      stuff->device_specific_mailslot = NULL;
    }
}

select_record *
fhandler_mailslot::select_read (select_record *s)
{
  if (!s)
    s = new select_record;
  s->startup = start_thread_mailslot;
  s->peek = peek_mailslot;
  s->verify = verify_mailslot;
  s->cleanup = mailslot_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}
