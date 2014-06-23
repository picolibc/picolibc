/* select.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* The following line means that the BSD socket definitions for
   fd_set, FD_ISSET etc. are used in this file.  */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <stdlib.h>
#include <sys/param.h>
#include "ntdll.h"

#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <netdb.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "select.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "sigproc.h"
#include "cygtls.h"
#include "cygwait.h"

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

#define UNIX_NFDBITS (sizeof (fd_mask) * NBBY)       /* bits per mask */
#ifndef unix_howmany
#define unix_howmany(x,y) (((x)+((y)-1))/(y))
#endif

#define unix_fd_set fd_set

#define NULL_fd_set ((fd_set *) NULL)
#define sizeof_fd_set(n) \
  ((size_t) (NULL_fd_set->fds_bits + unix_howmany ((n), UNIX_NFDBITS)))
#define UNIX_FD_SET(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] |= (1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_CLR(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] &= ~(1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_ISSET(n, p) \
  ((p)->fds_bits[(n)/UNIX_NFDBITS] & (1L << ((n) % UNIX_NFDBITS)))
#define UNIX_FD_ZERO(p, n) \
  memset ((caddr_t) (p), 0, sizeof_fd_set ((n)))

#define allocfd_set(n) ({\
  size_t __sfds = sizeof_fd_set (n) + 8; \
  void *__res = alloca (__sfds); \
  memset (__res, 0, __sfds); \
  (fd_set *) __res; \
})

#define copyfd_set(to, from, n) memcpy (to, from, sizeof_fd_set (n));

#define set_handle_or_return_if_not_open(h, s) \
  h = (s)->fh->get_handle (); \
  if (cygheap->fdtab.not_open ((s)->fd)) \
    { \
      (s)->thread_errno =  EBADF; \
      return -1; \
    }

static int select (int, fd_set *, fd_set *, fd_set *, DWORD);

/* The main select code.  */
extern "C" int
cygwin_select (int maxfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	       struct timeval *to)
{
  select_printf ("select(%d, %p, %p, %p, %p)", maxfds, readfds, writefds, exceptfds, to);

  pthread_testcancel ();
  int res;
  if (maxfds < 0)
    {
      set_errno (EINVAL);
      res = -1;
    }
  else
    {
      /* Convert to milliseconds or INFINITE if to == NULL */
      DWORD ms = to ? (to->tv_sec * 1000) + (to->tv_usec / 1000) : INFINITE;
      if (ms == 0 && to->tv_usec)
	ms = 1;			/* At least 1 ms granularity */

      if (to)
	select_printf ("to->tv_sec %ld, to->tv_usec %ld, ms %d", to->tv_sec, to->tv_usec, ms);
      else
	select_printf ("to NULL, ms %x", ms);

      res = select (maxfds, readfds ?: allocfd_set (maxfds),
		    writefds ?: allocfd_set (maxfds),
		    exceptfds ?: allocfd_set (maxfds), ms);
    }
  syscall_printf ("%R = select(%d, %p, %p, %p, %p)", res, maxfds, readfds,
		  writefds, exceptfds, to);
  return res;
}

/* This function is arbitrarily split out from cygwin_select to avoid odd
   gcc issues with the use of allocfd_set and improper constructor handling
   for the sel variable.  */
static int
select (int maxfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	DWORD ms)
{
  int res = select_stuff::select_loop;

  LONGLONG start_time = gtod.msecs ();	/* Record the current time for later use. */

  select_stuff sel;
  sel.return_on_signal = 0;

  /* Allocate some fd_set structures using the number of fds as a guide. */
  fd_set *r = allocfd_set (maxfds);
  fd_set *w = allocfd_set (maxfds);
  fd_set *e = allocfd_set (maxfds);

  while (res == select_stuff::select_loop)
    {
      /* Build the select record per fd linked list and set state as
	 needed. */
      for (int i = 0; i < maxfds; i++)
	if (!sel.test_and_set (i, readfds, writefds, exceptfds))
	  {
	    select_printf ("aborting due to test_and_set error");
	    return -1;	/* Invalid fd, maybe? */
	  }
      select_printf ("sel.always_ready %d", sel.always_ready);

      /* Degenerate case.  No fds to wait for.  Just wait for time to run out
	 or signal to arrive. */
      if (sel.start.next == NULL)
	switch (cygwait (ms))
	  {
	  case WAIT_SIGNALED:
	    select_printf ("signal received");
	    /* select() is always interrupted by a signal so set EINTR,
	       unconditionally, ignoring any SA_RESTART detection by
	       call_signal_handler().  */
	    _my_tls.call_signal_handler ();
	    set_sig_errno (EINTR);
	    res = select_stuff::select_signalled;
	    break;
	  case WAIT_CANCELED:
	    sel.destroy ();
	    pthread::static_cancel_self ();
	    /*NOTREACHED*/
	  default:
	    res = select_stuff::select_set_zero;	/* Set res to zero below. */
	    break;
	  }
      else if (sel.always_ready || ms == 0)
	res = 0;					/* Catch any active fds via
							   sel.poll() below */
      else
	res = sel.wait (r, w, e, ms);			/* wait for an fd to become
							   become active or time out */
      select_printf ("res %d", res);
      if (res >= 0)
	{
	  copyfd_set (readfds, r, maxfds);
	  copyfd_set (writefds, w, maxfds);
	  copyfd_set (exceptfds, e, maxfds);
	  if (res == select_stuff::select_set_zero)
	    res = 0;
	  else
	    /* Set the bit mask from sel records */
	    res = sel.poll (readfds, writefds, exceptfds) ?: select_stuff::select_loop;
	}
      /* Always clean up everything here.  If we're looping then build it
	 all up again.  */
      sel.cleanup ();
      sel.destroy ();
      /* Recalculate the time remaining to wait if we are going to be looping. */
      if (res == select_stuff::select_loop && ms != INFINITE)
	{
	  select_printf ("recalculating ms");
	  LONGLONG now = gtod.msecs ();
	  if (now > (start_time + ms))
	    {
	      select_printf ("timed out after verification");
	      res = 0;
	    }
	  else
	    {
	      ms -= (now - start_time);
	      start_time = now;
	      select_printf ("ms now %u", ms);
	    }
	}
    }

  if (res < -1)
    res = -1;
  return res;
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
    set_signal_mask (_my_tls.sigmask, *set);
  int ret = cygwin_select (maxfds, readfds, writefds, exceptfds,
			   ts ? &tv : NULL);
  if (set)
    set_signal_mask (_my_tls.sigmask, oldset);
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
inline void
select_stuff::destroy ()
{
  select_record *s;
  select_record *snext = start.next;

  select_printf ("deleting select records");
  while ((s = snext))
    {
      snext = s->next;
      delete s;
    }
  start.next = NULL;
}

select_stuff::~select_stuff ()
{
  cleanup ();
  destroy ();
}

#ifdef DEBUGGING
void
select_record::dump_select_record ()
{
  select_printf ("fd %d, h %p, fh %p, thread_errno %d, windows_handle %p",
		 fd, h, fh, thread_errno, windows_handle);
  select_printf ("read_ready %d, write_ready %d, except_ready %d",
		 read_ready, write_ready, except_ready);
  select_printf ("read_selected %d, write_selected %d, except_selected %d, except_on_write %d",
		 read_selected, write_selected, except_selected, except_on_write);
                    
  select_printf ("startup %p, peek %p, verify %p cleanup %p, next %p",
		 startup, peek, verify, cleanup, next);
}
#endif /*DEBUGGING*/

/* Add a record to the select chain */
bool
select_stuff::test_and_set (int i, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds)
{
  if (!UNIX_FD_ISSET (i, readfds) && !UNIX_FD_ISSET (i, writefds)
      && ! UNIX_FD_ISSET (i, exceptfds))
    return true;

  select_record *s = new select_record;
  if (!s)
    return false;

  s->next = start.next;
  start.next = s;

  if (UNIX_FD_ISSET (i, readfds) && !cygheap->fdtab.select_read (i, this))
    goto err;
  if (UNIX_FD_ISSET (i, writefds) && !cygheap->fdtab.select_write (i, this))
    goto err;
  if (UNIX_FD_ISSET (i, exceptfds) && !cygheap->fdtab.select_except (i, this))
    goto err; /* error */

  if (s->read_ready || s->write_ready || s->except_ready)
    always_ready = true;

  if (s->windows_handle)
    windows_used = true;

#ifdef DEBUGGING
  s->dump_select_record ();
#endif
  return true;

err:
  start.next = s->next;
  delete s;
  return false;
}

/* The heart of select.  Waits for an fd to do something interesting. */
select_stuff::wait_states
select_stuff::wait (fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		    DWORD ms)
{
  HANDLE w4[MAXIMUM_WAIT_OBJECTS];
  select_record *s = &start;
  DWORD m = 0;

  set_signal_arrived here (w4[m++]);
  if ((w4[m] = pthread::get_cancel_event ()) != NULL)
    m++;

  DWORD startfds = m;
  /* Loop through the select chain, starting up anything appropriate and
     counting the number of active fds. */
  while ((s = s->next))
    {
      if (m >= MAXIMUM_WAIT_OBJECTS)
	{
	  set_sig_errno (EINVAL);
	  return select_error;
	}
      if (!s->startup (s, this))
	{
	  s->set_select_errno ();
	  return select_error;
	}
      if (s->h != NULL)
	{
	  for (DWORD i = startfds; i < m; i++)
	    if (w4[i] == s->h)
	      goto next_while;
	  w4[m++] = s->h;
	}
next_while:;
    }

  debug_printf ("m %d, ms %u", m, ms);

  DWORD wait_ret;
  if (!windows_used)
    wait_ret = WaitForMultipleObjects (m, w4, FALSE, ms);
  else
    /* Using MWMO_INPUTAVAILABLE is the officially supported solution for
       the problem that the call to PeekMessage disarms the queue state
       so that a subsequent MWFMO hangs, even if there are still messages
       in the queue. */
    wait_ret = MsgWaitForMultipleObjectsEx (m, w4, ms,
					    QS_ALLINPUT | QS_ALLPOSTMESSAGE,
					    MWMO_INPUTAVAILABLE);
  select_printf ("wait_ret %d, m = %d.  verifying", wait_ret, m);

  wait_states res;
  switch (wait_ret)
    {
    case WAIT_OBJECT_0:
      select_printf ("signal received");
      /* Need to get rid of everything when a signal occurs since we can't
	 be assured that a signal handler won't jump out of select entirely. */
      cleanup ();
      destroy ();
      /* select() is always interrupted by a signal so set EINTR,
	 unconditionally, ignoring any SA_RESTART detection by
	 call_signal_handler().  */
      _my_tls.call_signal_handler ();
      set_sig_errno (EINTR);
      res = select_signalled;	/* Cause loop exit in cygwin_select */
      break;
    case WAIT_FAILED:
      system_printf ("WaitForMultipleObjects failed, %E");
      s = &start;
      s->set_select_errno ();
      res = select_error;
      break;
    case WAIT_TIMEOUT:
      select_printf ("timed out");
      res = select_set_zero;
      break;
    case WAIT_OBJECT_0 + 1:
      if (startfds > 1)
	{
	  cleanup ();
	  destroy ();
	  pthread::static_cancel_self ();
	  /*NOTREACHED*/
	}
      /* Fall through.  This wasn't a cancel event.  It was just a normal object
	 to wait for.  */
    default:
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
	    res = select_error;		/* Somebody detected an error */
	    goto out;
	  }
	else if ((((wait_ret >= m && s->windows_handle) || s->h == w4[wait_ret]))
		 && s->verify (s, readfds, writefds, exceptfds))
	  gotone = true;

      if (!gotone)
	res = select_loop;
      else
	res = select_ok;
      select_printf ("gotone %d", gotone);
      break;
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
pipe_data_available (int fd, fhandler_base *fh, HANDLE h, bool writing)
{
  IO_STATUS_BLOCK iosb = {{0}, 0};
  FILE_PIPE_LOCAL_INFORMATION fpli = {0};

  bool res;
  if (fh->has_ongoing_io ())
    res = false;
  else if (NtQueryInformationFile (h, &iosb, &fpli, sizeof (fpli),
				   FilePipeLocalInformation))
    {
      /* If NtQueryInformationFile fails, optimistically assume the
	 pipe is writable.  This could happen if we somehow
	 inherit a pipe that doesn't permit FILE_READ_ATTRIBUTES
	 access on the write end.  */
      select_printf ("fd %d, %s, NtQueryInformationFile failed",
		     fd, fh->get_name ());
      res = writing ? true : -1;
    }
  else if (!writing)
    {
      paranoid_printf ("fd %d, %s, read avail %u", fd, fh->get_name (),
		       fpli.ReadDataAvailable);
      res = !!fpli.ReadDataAvailable;
    }
  else if ((res = (fpli.WriteQuotaAvailable = (fpli.OutboundQuota -
					       fpli.ReadDataAvailable))))
    /* If there is anything available in the pipe buffer then signal
       that.  This means that a pipe could still block since you could
       be trying to write more to the pipe than is available in the
       buffer but that is the hazard of select().  */
    paranoid_printf ("fd %d, %s, write: size %u, avail %u", fd,
		     fh->get_name (), fpli.OutboundQuota,
		     fpli.WriteQuotaAvailable);
  else if ((res = (fpli.OutboundQuota < PIPE_BUF &&
		   fpli.WriteQuotaAvailable == fpli.OutboundQuota)))
    /* If we somehow inherit a tiny pipe (size < PIPE_BUF), then consider
       the pipe writable only if it is completely empty, to minimize the
       probability that a subsequent write will block.  */
    select_printf ("fd, %s, write tiny pipe: size %u, avail %u",
		   fd, fh->get_name (), fpli.OutboundQuota,
		   fpli.WriteQuotaAvailable);
  return res ?: -!!(fpli.NamedPipeState & FILE_PIPE_CLOSING_STATE);
}

static int
peek_pipe (select_record *s, bool from_select)
{
  HANDLE h;
  set_handle_or_return_if_not_open (h, s);

  int gotone = 0;
  fhandler_base *fh = (fhandler_base *) s->fh;

  DWORD dev = fh->get_device ();
  if (s->read_selected && dev != FH_PIPEW)
    {
      if (s->read_ready)
	{
	  select_printf ("%s, already ready for read", fh->get_name ());
	  gotone = 1;
	  goto out;
	}

      switch (fh->get_major ())
	{
	case DEV_PTYM_MAJOR:
	  {
	    fhandler_pty_master *fhm = (fhandler_pty_master *) fh;
	    fhm->flush_to_slave ();
	    if (fhm->need_nl)
	      {
		gotone = s->read_ready = true;
		goto out;
	      }
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
      int n = pipe_data_available (s->fd, fh, h, false);

      if (n < 0)
	{
	  select_printf ("read: %s, n %d", fh->get_name (), n);
	  if (s->except_selected)
	    gotone += s->except_ready = true;
	  if (s->read_selected)
	    gotone += s->read_ready = true;
	}
      else if (n > 0)
	{
	  select_printf ("read: %s, ready for read: avail %d", fh->get_name (), n);
	  gotone += s->read_ready = true;
	}
      if (!gotone && s->fh->hit_eof ())
	{
	  select_printf ("read: %s, saw EOF", fh->get_name ());
	  if (s->except_selected)
	    gotone += s->except_ready = true;
	  if (s->read_selected)
	    gotone += s->read_ready = true;
	}
    }

out:
  if (s->write_selected && dev != FH_PIPER)
    {
      gotone += s->write_ready =  pipe_data_available (s->fd, fh, h, true);
      select_printf ("write: %s, gotone %d", fh->get_name (), gotone);
    }
  return gotone;
}

static int start_thread_pipe (select_record *me, select_stuff *stuff);

static DWORD WINAPI
thread_pipe (void *arg)
{
  select_pipe_info *pi = (select_pipe_info *) arg;
  DWORD sleep_time = 0;
  bool looping = true;

  while (looping)
    {
      for (select_record *s = pi->start; (s = s->next); )
	if (s->startup == start_thread_pipe)
	  {
	    if (peek_pipe (s, true))
	      looping = false;
	    if (pi->stop_thread)
	      {
		select_printf ("stopping");
		looping = false;
		break;
	      }
	  }
      if (!looping)
	break;
      Sleep (sleep_time >> 3);
      if (sleep_time < 80)
	++sleep_time;
      if (pi->stop_thread)
	break;
    }
  return 0;
}

static int
start_thread_pipe (select_record *me, select_stuff *stuff)
{
  select_pipe_info *pi = stuff->device_specific_pipe;
  if (pi->start)
    me->h = *((select_pipe_info *) stuff->device_specific_pipe)->thread;
  else
    {
      pi->start = &stuff->start;
      pi->stop_thread = false;
      pi->thread = new cygthread (thread_pipe, pi, "pipesel");
      me->h = *pi->thread;
      if (!me->h)
	return 0;
    }
  return 1;
}

static void
pipe_cleanup (select_record *, select_stuff *stuff)
{
  select_pipe_info *pi = (select_pipe_info *) stuff->device_specific_pipe;
  if (!pi)
    return;
  if (pi->thread)
    {
      pi->stop_thread = true;
      pi->thread->detach ();
    }
  delete pi;
  stuff->device_specific_pipe = NULL;
}

select_record *
fhandler_pipe::select_read (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;

  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_pipe::select_write (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->write_selected = true;
  s->write_ready = false;
  return s;
}

select_record *
fhandler_pipe::select_except (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

select_record *
fhandler_fifo::select_read (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_fifo::select_write (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->write_selected = true;
  s->write_ready = false;
  return s;
}

select_record *
fhandler_fifo::select_except (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
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
	fh->send_winch_maybe ();
	if (irec.EventType == KEY_EVENT)
	  {
	    if (irec.Event.KeyEvent.bKeyDown
		&& (irec.Event.KeyEvent.uChar.AsciiChar
		    || get_nonascii_key (irec, tmpbuf)))
	      return me->read_ready = true;
	  }
	else
	  {
	    if (irec.EventType == MOUSE_EVENT
		&& fh->mouse_aware (irec.Event.MouseEvent))
		return me->read_ready = true;
	    if (irec.EventType == FOCUS_EVENT && fh->focus_aware ())
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
fhandler_console::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_console::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_console::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_pty_common::select_read (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;

  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}

select_record *
fhandler_pty_common::select_write (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->write_selected = true;
  s->write_ready = false;
  return s;
}

select_record *
fhandler_pty_common::select_except (select_stuff *ss)
{
  if (!ss->device_specific_pipe
      && (ss->device_specific_pipe = new select_pipe_info) == NULL)
    return NULL;
  select_record *s = ss->start.next;
  s->startup = start_thread_pipe;
  s->peek = peek_pipe;
  s->verify = verify_ok;
  s->cleanup = pipe_cleanup;
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

static int
verify_tty_slave (select_record *me, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds)
{
  if (IsEventSignalled (me->h))
    me->read_ready = true;
  return set_bits (me, readfds, writefds, exceptfds);
}

select_record *
fhandler_pty_slave::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
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
fhandler_dev_null::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->read_selected = true;
  s->read_ready = true;
  return s;
}

select_record *
fhandler_dev_null::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_dev_null::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = no_verify;
    }
  s->h = get_handle ();
  s->except_selected = true;
  s->except_ready = false;
  return s;
}

static int start_thread_serial (select_record *me, select_stuff *stuff);

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

  if ((s->read_selected && s->read_ready) || (s->write_selected && s->write_ready))
    {
      select_printf ("already ready");
      ready = 1;
      goto out;
    }

  /* This is apparently necessary for the com0com driver.
     See: http://cygwin.com/ml/cygwin/2009-01/msg00667.html */
  SetCommMask (h, 0);

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

  switch (WaitForSingleObject (fh->io_status.hEvent, 10L))
    {
    case WAIT_OBJECT_0:
      if (!ClearCommError (h, &fh->ev, &st))
	{
	  debug_printf ("ClearCommError");
	  goto err;
	}
      else if (!st.cbInQue)
	Sleep (10L);
      else
	{
	  return s->read_ready = true;
	  select_printf ("got something");
	}
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
  select_serial_info *si = (select_serial_info *) arg;
  bool looping = true;

  while (looping)
    for (select_record *s = si->start; (s = s->next); )
      if (s->startup != start_thread_serial)
	continue;
      else
	{
	  if (peek_serial (s, true))
	    looping = false;
	  if (si->stop_thread)
	    {
	      select_printf ("stopping");
	      looping = false;
	      break;
	    }
	}

  select_printf ("exiting");
  return 0;
}

static int
start_thread_serial (select_record *me, select_stuff *stuff)
{
  if (stuff->device_specific_serial)
    me->h = *((select_serial_info *) stuff->device_specific_serial)->thread;
  else
    {
      select_serial_info *si = new select_serial_info;
      si->start = &stuff->start;
      si->stop_thread = false;
      si->thread = new cygthread (thread_serial, si, "sersel");
      me->h = *si->thread;
      stuff->device_specific_serial = si;
    }
  return 1;
}

static void
serial_cleanup (select_record *, select_stuff *stuff)
{
  select_serial_info *si = (select_serial_info *) stuff->device_specific_serial;
  if (!si)
    return;
  if (si->thread)
    {
      si->stop_thread = true;
      si->thread->detach ();
    }
  delete si;
  stuff->device_specific_serial = NULL;
}

select_record *
fhandler_serial::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_serial::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_serial::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = NULL;
  s->peek = peek_serial;
  s->except_selected = false;	// Can't do this
  s->except_ready = false;
  return s;
}

select_record *
fhandler_base::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->read_selected = true;
  s->read_ready = true;
  return s;
}

select_record *
fhandler_base::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->h = get_handle ();
  s->write_selected = true;
  s->write_ready = true;
  return s;
}

select_record *
fhandler_base::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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

static DWORD WINAPI
thread_socket (void *arg)
{
  select_socket_info *si = (select_socket_info *) arg;
  DWORD timeout = (si->num_w4 <= MAXIMUM_WAIT_OBJECTS)
		  ? INFINITE
		  : (64 / (roundup2 (si->num_w4, MAXIMUM_WAIT_OBJECTS)
			   / MAXIMUM_WAIT_OBJECTS));
  bool event = false;

  select_printf ("stuff_start %p, timeout %u", si->start, timeout);
  while (!event)
    {
      for (select_record *s = si->start; (s = s->next); )
	if (s->startup == start_thread_socket)
	  if (peek_socket (s, false))
	    event = true;
      if (!event)
	for (int i = 0; i < si->num_w4; i += MAXIMUM_WAIT_OBJECTS)
	  switch (WaitForMultipleObjects (MIN (si->num_w4 - i,
					       MAXIMUM_WAIT_OBJECTS),
					  si->w4 + i, FALSE, timeout))
	    {
	    case WAIT_FAILED:
	      goto out;
	    case WAIT_TIMEOUT:
	      continue;
	    case WAIT_OBJECT_0:
	      if (!i)	/* Socket event set. */
		goto out;
	      /*FALLTHRU*/
	    default:
	      break;
	    }
    }
out:
  select_printf ("leaving thread_socket");
  return 0;
}

static inline bool init_tls_select_info () __attribute__ ((always_inline));
static inline bool
init_tls_select_info ()
{
  if (!_my_tls.locals.select.sockevt)
    {
      _my_tls.locals.select.sockevt = CreateEvent (&sec_none_nih, TRUE, FALSE,
						   NULL);
      if (!_my_tls.locals.select.sockevt)
	return false;
    }
  if (!_my_tls.locals.select.ser_num)
    {
      _my_tls.locals.select.ser_num
	      = (LONG *) malloc (MAXIMUM_WAIT_OBJECTS * sizeof (LONG));
      if (!_my_tls.locals.select.ser_num)
	return false;
      _my_tls.locals.select.w4
	      = (HANDLE *) malloc (MAXIMUM_WAIT_OBJECTS * sizeof (HANDLE));
      if (!_my_tls.locals.select.w4)
	{
	  free (_my_tls.locals.select.ser_num);
	  _my_tls.locals.select.ser_num = NULL;
	  return false;
	}
      _my_tls.locals.select.max_w4 = MAXIMUM_WAIT_OBJECTS;
    }
  return true;
}

static int
start_thread_socket (select_record *me, select_stuff *stuff)
{
  select_socket_info *si;

  if ((si = (select_socket_info *) stuff->device_specific_socket))
    {
      me->h = *si->thread;
      return 1;
    }

  si = new select_socket_info;

  if (!init_tls_select_info ())
    {
      delete si;
      return 0;
    }

  si->ser_num = _my_tls.locals.select.ser_num;
  si->w4 = _my_tls.locals.select.w4;

  si->w4[0] = _my_tls.locals.select.sockevt;
  si->num_w4 = 1;

  select_record *s = &stuff->start;
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
	if (si->num_w4 >= _my_tls.locals.select.max_w4)
	  {
	    LONG *nser = (LONG *) realloc (si->ser_num,
					   (_my_tls.locals.select.max_w4
					    + MAXIMUM_WAIT_OBJECTS)
					   * sizeof (LONG));
	    if (!nser)
	      {
		delete si;
		return 0;
	      }
	    _my_tls.locals.select.ser_num = si->ser_num = nser;
	    HANDLE *nw4 = (HANDLE *) realloc (si->w4,
					   (_my_tls.locals.select.max_w4
					    + MAXIMUM_WAIT_OBJECTS)
					   * sizeof (HANDLE));
	    if (!nw4)
	      {
		delete si;
		return 0;
	      }
	    _my_tls.locals.select.w4 = si->w4 = nw4;
	    _my_tls.locals.select.max_w4 += MAXIMUM_WAIT_OBJECTS;
	  }
	si->ser_num[si->num_w4] = ser_num;
	si->w4[si->num_w4++] = ((fhandler_socket *) s->fh)->wsock_event ();
      continue_outer_loop:
	;
      }
  stuff->device_specific_socket = si;
  si->start = &stuff->start;
  select_printf ("stuff_start %p", &stuff->start);
  si->thread = new cygthread (thread_socket, si, "socksel");
  me->h = *si->thread;
  return 1;
}

void
socket_cleanup (select_record *, select_stuff *stuff)
{
  select_socket_info *si = (select_socket_info *) stuff->device_specific_socket;
  select_printf ("si %p si->thread %p", si, si ? si->thread : NULL);
  if (!si)
    return;
  if (si->thread)
    {
      SetEvent (si->w4[0]);
      /* Wait for thread to go away */
      si->thread->detach ();
      ResetEvent (si->w4[0]);
    }
  delete si;
  stuff->device_specific_socket = NULL;
  select_printf ("returning");
}

select_record *
fhandler_socket::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_socket::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
fhandler_socket::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
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
  /* We need the hWnd value, not the io_handle. */
  h = ((fhandler_windows *) me->fh)->get_hwnd ();

  if (me->read_selected && me->read_ready)
    return 1;

  if (PeekMessageW (&m, (HWND) h, 0, 0, PM_NOREMOVE))
    {
      me->read_ready = true;
      select_printf ("window %d(%p) ready", me->fd, h);
      return 1;
    }

  select_printf ("window %d(%p) not ready", me->fd, h);
  return me->write_ready;
}

static int
verify_windows (select_record *me, fd_set *rfds, fd_set *wfds,
		fd_set *efds)
{
  return peek_windows (me, true);
}

select_record *
fhandler_windows::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
    }
  s->verify = verify_windows;
  s->peek = peek_windows;
  s->read_selected = true;
  s->read_ready = false;
  s->windows_handle = true;
  return s;
}

select_record *
fhandler_windows::select_write (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->peek = peek_windows;
  s->write_selected = true;
  s->write_ready = true;
  s->windows_handle = true;
  return s;
}

select_record *
fhandler_windows::select_except (select_stuff *ss)
{
  select_record *s = ss->start.next;
  if (!s->startup)
    {
      s->startup = no_startup;
      s->verify = verify_ok;
    }
  s->peek = peek_windows;
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

static DWORD WINAPI
thread_mailslot (void *arg)
{
  select_mailslot_info *mi = (select_mailslot_info *) arg;
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
	    if (mi->stop_thread)
	      {
		select_printf ("stopping");
		goto out;
	      }
	  }
      /* Paranoid check */
      if (mi->stop_thread)
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
      me->h = *((select_mailslot_info *) stuff->device_specific_mailslot)->thread;
      return 1;
    }
  select_mailslot_info *mi = new select_mailslot_info;
  mi->start = &stuff->start;
  mi->stop_thread = false;
  mi->thread = new cygthread (thread_mailslot, mi, "mailsel");
  me->h = *mi->thread;
  if (!me->h)
    return 0;
  stuff->device_specific_mailslot = mi;
  return 1;
}

static void
mailslot_cleanup (select_record *, select_stuff *stuff)
{
  select_mailslot_info *mi = (select_mailslot_info *) stuff->device_specific_mailslot;
  if (!mi)
    return;
  if (mi->thread)
    {
      mi->stop_thread = true;
      mi->thread->detach ();
    }
  delete mi;
  stuff->device_specific_mailslot = NULL;
}

select_record *
fhandler_mailslot::select_read (select_stuff *ss)
{
  select_record *s = ss->start.next;
  s->startup = start_thread_mailslot;
  s->peek = peek_mailslot;
  s->verify = verify_mailslot;
  s->cleanup = mailslot_cleanup;
  s->read_selected = true;
  s->read_ready = false;
  return s;
}
