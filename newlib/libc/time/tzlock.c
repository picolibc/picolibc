/*
FUNCTION
<<__tz_lock>>, <<__tz_unlock>>---lock time zone global variables

INDEX
	__tz_lock
INDEX
	__tz_unlock

SYNOPSIS
	#include "local.h"
	void __tz_lock (void);
	void __tz_unlock (void);

DESCRIPTION
The <<tzset>> facility functions call these functions when they need to
ensure the values of global variables.  The version of these routines
supplied in the library use the lock API defined in sys/lock.h.  If multiple
threads of execution can call the time functions and give up scheduling in
the middle, then you you need to define your own versions of these functions
in order to safely lock the time zone variables during a call.  If you do
not, the results of <<localtime>>, <<mktime>>, <<ctime>>, and <<strftime>>
are undefined.

The lock <<__tz_lock>> may not be called recursively; that is,
a call <<__tz_lock>> will always lock all subsequent <<__tz_lock>> calls
until the corresponding <<__tz_unlock>> call on the same thread is made.
*/

#include <_ansi.h>
#include "local.h"
#include <sys/lock.h>

#ifndef __SINGLE_THREAD__
__LOCK_INIT(static, __tz_mutex);
#endif

void
__tz_lock (void)
{
#ifndef __SINGLE_THREAD__
  __lock_acquire(__tz_mutex);
#endif
}

void
__tz_unlock (void)
{
#ifndef __SINGLE_THREAD__
  __lock_release(__tz_mutex);
#endif
}
