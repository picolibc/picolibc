/* sync.cc: Synchronization functions for cygwin.

   This file implements the methods for controlling the "muto" class
   which is intended to operate similarly to a mutex but attempts to
   avoid making expensive calls to the kernel.

   Copyright 2000 Cygnus Solutions.

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

muto NO_COPY muto_start;

#undef WaitForSingleObject

/* Constructor */
muto::muto (int inh, const char *s) : sync (0), visits(0), waiters(-1), tid (0), next (NULL)
{
  /* Create event which is used in the fallback case when blocking is necessary */
  if (!(bruteforce = CreateEvent (inh ? &sec_all_nih : &sec_none_nih, FALSE, FALSE, name)))
    {
      DWORD oerr = GetLastError ();
      SetLastError (oerr);
      return;
    }
  name = s;
}

/* Destructor (racy?) */
muto::~muto ()
{
  while (visits)
    release ();

  HANDLE h = bruteforce;
  h = NULL;
  /* Just need to close the event handle */
  if (h)
    CloseHandle (h);
}

/* Acquire the lock.  Argument is the number of milliseconds to wait for
   the lock.  Multiple visits from the same thread are allowed and should
   be handled correctly.

   Note: The goal here is to minimize, as much as possible, calls to the
   OS.  Hence the use of InterlockedIncrement, etc., rather than (much) more
   expensive OS mutexes.  */
int
muto::acquire (DWORD ms)
{
  DWORD this_tid = GetCurrentThreadId ();

  if (tid != this_tid)
    {
      /* Increment the waiters part of the class.  Need to do this first to
	 avoid potential races. */
      LONG was_waiting = InterlockedIncrement (&waiters);

      /* This is deceptively simple.  Basically, it allows multiple attempts to
	 lock the same muto to succeed without attempting to manipulate sync.
	 If the muto is already locked then this thread will wait for ms until
	 it is signalled by muto::release.  Then it will attempt to grab the
	 sync field.  If it succeeds, then this thread owns the muto.

	 There is a pathological condition where a thread times out waiting for
	 bruteforce but the release code triggers the bruteforce event.  In this
	 case, it is possible for a thread which is going to wait for bruteforce
	 to wake up immediately.  It will then attempt to grab sync but will fail
	 and go back to waiting.  */
      while (tid != this_tid && (was_waiting || InterlockedExchange (&sync, 1) != 0))
	{
	  switch (WaitForSingleObject (bruteforce, ms))
	      {
	      case WAIT_OBJECT_0:
		was_waiting = 0;
		break;
	      default:
		InterlockedDecrement (&waiters);
		return 0;	/* failed. */
	      }
	}
    }

  tid = this_tid;	/* register this thread. */
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
      InterlockedExchange (&sync, 0); /* Reset trigger. */
      /* This thread had incremented waiters but had never decremented it.
	 Decrement it now.  If it is >= 0 then there are possibly other
	 threads waiting for the lock, so trigger bruteforce. */
      if (InterlockedDecrement (&waiters) >= 0)
	(void) SetEvent (bruteforce); /* Wake up one of the waiting threads */
    }

  return 1;	/* success. */
}
