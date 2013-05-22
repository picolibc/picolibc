/* spinlock.h: Header file for cygwin time-sensitive synchronization primitive.

   Copyright 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include "ntdll.h"

#define SPINLOCK_WAIT (15000LL * 10000LL)

class spinlock
{
  LONG *locker;
  LONG val;
  LONG setto;
  void done (LONG what)
  {
    if (locker)
      {
	InterlockedExchange (locker, what);
	locker = NULL;
      }
  }
  long long time ()
  {
    LARGE_INTEGER t;
    if (NtQuerySystemTime (&t) == STATUS_SUCCESS)
      return get_ll (t);
    return 0LL;
  }
public:
  spinlock (LONG& locktest, LONG wanted_val = 1, LONGLONG timeout = SPINLOCK_WAIT):
    locker (&locktest), setto (wanted_val)
  {
    /* Quick test to see if we're already initialized */
    if ((val = locktest) == wanted_val)
      locker = NULL;
    /* Slightly less quick test to see if we are the first cygwin process */
    else if ((val = InterlockedExchange (locker, -1)) == 0)
      /* We're armed and dangerous */;
    else if (val == wanted_val)
      done (val);	/* This was initialized while we weren't looking */
    else
      {
	long long then = time ();
	/* Loop waiting for some other process to set locktest to something
	   other than -1, indicating that initialization has finished.  Or,
	   wait a default of 15 seconds for that to happen and, if it doesn't
	   just grab the lock ourselves. */
	while ((val = InterlockedExchange (locker, -1)) == -1
	       && (time () - then) < timeout)
	  yield ();
	/* Reset the lock back to wanted_value under the assumption that is
	   what caused the above loop to kick out.  */
	if (val == -1)
	  val = 0;	/* Timed out.  We'll initialize things ourselves. */
	else
	  done (val);	/* Put back whatever was there before, assuming that
			   it is actually wanted_val. */
      }
  }
  ~spinlock () {done (setto);}
  operator ULONG () const {return (ULONG) val;}
  /* FIXME: This should be handled in a more general fashion, probably by
     establishing a linked list of spinlocks which are freed on process exit. */
  void multiple_cygwin_problem (const char *w, unsigned m, unsigned v)
  {
    done (val);
    ::multiple_cygwin_problem (w, m, v);
  }
};

#endif /*_SPINLOCK_H*/
