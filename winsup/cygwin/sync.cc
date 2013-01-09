/* sync.cc: Synchronization functions for cygwin.

   This file implements the methods for controlling the "muto" class
   which is intended to operate similarly to a mutex but attempts to
   avoid making expensive calls to the kernel.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2010, 2011, 2012
   Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include "sync.h"
#include "thread.h"
#include "cygtls.h"

#undef WaitForSingleObject

muto NO_COPY lock_process::locker;

void
muto::grab ()
{
  tls = &_my_tls;
}

/* Constructor */
muto *
muto::init (const char *s)
{
  char *already_exists = (char *) InterlockedExchangePointer ((PVOID *) &name,
							      (PVOID) s);
  if (already_exists)
    while (!bruteforce)
      yield ();
  else
    {
      waiters = -1;
      /* Create event which is used in the fallback case when blocking is necessary */
      bruteforce = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
      if (!bruteforce)
	  api_fatal ("couldn't allocate muto '%s', %E", s);
    }

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
   expensive OS mutexes.  */
int
muto::acquire (DWORD ms)
{
  void *this_tls = &_my_tls;

  if (tls != this_tls)
    {
      /* Increment the waiters part of the class.  Need to do this first to
	 avoid potential races. */
      LONG was_waiting = ms ? InterlockedIncrement (&waiters) : 0;

      while (was_waiting || InterlockedExchange (&sync, 1) != 0)
	switch (WaitForSingleObject (bruteforce, ms))
	    {
	    case WAIT_OBJECT_0:
	      was_waiting = 0;
	      break;
	    default:
	      return 0;	/* failed. */
	    }

      /* Have to do it this way to avoid a race */
      if (!ms)
	InterlockedIncrement (&waiters);

      tls = this_tls;	/* register this thread. */
    }

  return ++visits;	/* Increment visit count. */
}

bool
muto::acquired ()
{
  return tls == &_my_tls;
}

/* Return the muto lock.  Needs to be called once per every acquire. */
int
muto::release (_cygtls *this_tls)
{
  if (tls != this_tls || !visits)
    {
      SetLastError (ERROR_NOT_OWNER);	/* Didn't have the lock. */
      return 0;	/* failed. */
    }

  /* FIXME: Need to check that other thread has not exited, too. */
  if (!--visits)
    {
      tls = 0;		/* We were the last unlocker. */
      InterlockedExchange (&sync, 0); /* Reset trigger. */
      /* This thread had incremented waiters but had never decremented it.
	 Decrement it now.  If it is >= 0 then there are possibly other
	 threads waiting for the lock, so trigger bruteforce.  */
      if (InterlockedDecrement (&waiters) >= 0)
	SetEvent (bruteforce); /* Wake up one of the waiting threads */
      else if (*name == '!')
	{
	  CloseHandle (bruteforce);	/* If *name == '!' and there are no
					   other waiters, then this is the
					   last time this muto will ever be
					   used, so close the handle. */
#ifdef DEBUGGING
	  bruteforce = NULL;
#endif
	}
    }

  return 1;	/* success. */
}
