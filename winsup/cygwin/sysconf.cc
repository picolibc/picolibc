/* sysconf.cc

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include "dtable.h"

/* sysconf: POSIX 4.8.1.1 */
/* Allows a portable app to determine quantities of resources or
   presence of an option at execution time. */
long int
sysconf (int in)
{
  switch (in)
    {
      case _SC_ARG_MAX:
	/* FIXME: what's the right value?  _POSIX_ARG_MAX is only 4K */
	return 1048576;
      case _SC_OPEN_MAX:
	/* FIXME: this returns the current limit which can increase
	   if and when dtable::find_unused_handle is called.  Perhaps
	   we should return NOFILE or OPEN_MAX instead? */
	return fdtab.size;
      case _SC_PAGESIZE:
	{
	  SYSTEM_INFO b;
	  GetSystemInfo (&b);
	  return b.dwPageSize;
	}
      case _SC_CLK_TCK:
	return CLOCKS_PER_SEC;
      case _SC_JOB_CONTROL:
	return _POSIX_JOB_CONTROL;
      case _SC_CHILD_MAX:
	return CHILD_MAX;
      case _SC_NGROUPS_MAX:
	return NGROUPS_MAX;
      case _SC_SAVED_IDS:
	return _POSIX_SAVED_IDS;
      case _SC_VERSION:
	return _POSIX_VERSION;
#if 0	/* FIXME -- unimplemented */
      case _SC_TZNAME_MAX:
	return _POSIX_TZNAME_MAX;
      case _SC_STREAM_MAX:
	return _POSIX_STREAM_MAX;
#endif
    }

  /* Invalid input or unimplemented sysconf name */
  set_errno (EINVAL);
  return -1;
}
