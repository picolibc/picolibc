/* sync.cc: Synchronization functions for cygwin.

   This file implements the methods for controlling the "muto" class
   which is intended to operate similarly to a mutex but attempts to
   avoid making expensive calls to the kernel.

   Copyright 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include "sync.h"
#include "security.h"

#undef WaitForSingleObject

DWORD NO_COPY muto::exiting_thread;

/* Constructor */
muto *
muto::init (const char *s)
{
  waiters = -1;
  /* Create event which is used in the fallback case when blocking is necessary */
  if (!(bruteforce = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL)))
    {
      DWORD oerr = GetLastError ();
      SetLastError (oerr);
      return NULL;
    }
  name = s;
  return this;
}

#if 0 /* FIXME: Do we need this? mutos aren't destroyed until process exit */
/* Destructor (racy?) */
muto::~muto ()
{
  while (visits)
    release ();

  HANDLE h = bruteforce;
  bruteforce = NULL;
  /* Just need to close the event handle */
  if (h)
    CloseHandle (h);
}
#endif

/* Acquire the lock.  Argument is the number of milliseconds to wait for
   the lock.  Multiple visits from the same thread are allowed and should
   be handled correctly.

   Note: The goal here is to minimize, as much as possible, calls to the
   OS.  Hence the use of InterlockedIncrement, etc., rather than (much) more
   expensive OS mutexes.  Also note that the only two valid "ms" times are
   0 and INFINITE. */
int
muto::acquire (DWORD ms)
{
  DWORD this_tid = GetCurrentThreadId ();
#if 0
  if (exiting_thread)
    return this_tid == exiting_thread;
#endif

  if (tid != this_tid)
    {
      /* Increment the waiters part of the class.  Need to do this first to
	 avoid potential races. */
      LONG was_waiting = ms ? InterlockedIncrement (&waiters) : 0;

      while (was_waiting || InterlockedExchange (&sync, 1) != 0)
	  {
	    switch (WaitForSingleObject (bruteforce, ms))
		{
		case WAIT_OBJECT_0:
		  was_waiting = 0;
		  break;
		default:
		  return 0;	/* failed. */
		}
	  }

      /* Have to do it this way to avoid a race */
      if (!ms)
	InterlockedIncrement (&waiters);

      tid = this_tid;	/* register this thread. */
    }

  return ++visits;	/* Increment visit count. */
}

/* Return the muto lock.  Needs to be called once per every acquire. */
int
muto::release ()
{
  DWORD this_tid = GetCurrentThreadId ();

  if (tid != this_tid || !visits)
    {
      SetLastError (ERROR_NOT_OWNER);	/* Didn't have the lock. */
      return 0;	/* failed. */
    }

  /* FIXME: Need to check that other thread has not exited, too. */
  if (!--visits)
    {
      tid = 0;		/* We were the last unlocker. */
      (void) InterlockedExchange (&sync, 0); /* Reset trigger. */
      /* This thread had incremented waiters but had never decremented it.
	 Decrement it now.  If it is >= 0 then there are possibly other
	 threads waiting for the lock, so trigger bruteforce.  */
      if (InterlockedDecrement (&waiters) >= 0)
	(void) SetEvent (bruteforce); /* Wake up one of the waiting threads */
    }

  return 1;	/* success. */
}

bool
muto::acquired ()
{
  return tid == GetCurrentThreadId ();
}

/* Call only when we're exiting.  This is not thread safe. */
void
muto::reset ()
{
  visits = sync = tid = 0;
  InterlockedExchange (&waiters, -1);
  if (bruteforce)
    {
      CloseHandle (bruteforce);
      bruteforce = CreateEvent (&sec_none_nih, FALSE, FALSE, name);
    }
}
