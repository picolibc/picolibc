/* spinlock.h: Header file for cygwin time-sensitive synchronization primitive.

   Copyright 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include "hires.h"

class spinlock
{
  LONG *locker;
  LONG val;
public:
  spinlock (LONG& locktest, LONGLONG timeout):
    locker (&locktest)
  {
    if ((val = locktest) == 1)
      return;
    LONGLONG then = gtod.msecs ();
    for (;;)
      {
	if ((val = InterlockedExchange (locker, -1)) != -1
	    || (gtod.msecs () - then) >= timeout)
	  break;
	yield ();
      }
  }
  ~spinlock () {InterlockedExchange (locker, 1);}
  operator LONG () const {return val;}
};

#endif /*_SPINLOCK_H*/

