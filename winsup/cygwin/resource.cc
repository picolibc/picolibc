/* resource.cc: getrusage () and friends.

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

   Written by Steve Chamberlain (sac@cygnus.com), Doug Evans (dje@cygnus.com),
   Geoffrey Noer (noer@cygnus.com) of Cygnus Support.
   Rewritten by Sergey S. Okhapkin (sos@prospect.com.ru)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include "pinfo.h"
#include "cygerrno.h"

/* add timeval values */
static void
add_timeval (struct timeval *tv1, struct timeval *tv2)
{
  tv1->tv_sec += tv2->tv_sec;
  tv1->tv_usec += tv2->tv_usec;
  if (tv1->tv_usec >= 1000000)
    {
      tv1->tv_usec -= 1000000;
      tv1->tv_sec++;
    }
}

/* add rusage values of r2 to r1 */
void __stdcall
add_rusage (struct rusage *r1, struct rusage *r2)
{
  add_timeval (&r1->ru_utime, &r2->ru_utime);
  add_timeval (&r1->ru_stime, &r2->ru_stime);
  r1->ru_maxrss += r2->ru_maxrss;
  r1->ru_ixrss += r2->ru_ixrss;
  r1->ru_idrss += r2->ru_idrss;
  r1->ru_isrss += r2->ru_isrss;
  r1->ru_minflt += r2->ru_minflt;
  r1->ru_majflt += r2->ru_majflt;
  r1->ru_nswap += r2->ru_nswap;
  r1->ru_inblock += r2->ru_inblock;
  r1->ru_oublock += r2->ru_oublock;
  r1->ru_msgsnd += r2->ru_msgsnd;
  r1->ru_msgrcv += r2->ru_msgrcv;
  r1->ru_nsignals += r2->ru_nsignals;
  r1->ru_nvcsw += r2->ru_nvcsw;
  r1->ru_nivcsw += r2->ru_nivcsw;
}

/* FIXME: what about other fields? */
void __stdcall
fill_rusage (struct rusage *r, HANDLE h)
{
  FILETIME creation_time = {0,0};
  FILETIME exit_time = {0,0};
  FILETIME kernel_time = {0,0};
  FILETIME user_time = {0,0};

  struct timeval tv;

  GetProcessTimes (h, &creation_time, &exit_time, &kernel_time, &user_time);
  totimeval (&tv, &kernel_time, 0, 0);
  add_timeval (&r->ru_stime, &tv);
  totimeval (&tv, &user_time, 0, 0);
  add_timeval (&r->ru_utime, &tv);
}

extern "C"
int
getrusage (int intwho, struct rusage *rusage_in)
{
  int res = 0;
  struct rusage r;

  if (intwho == RUSAGE_SELF)
    {
      memset (&r, 0, sizeof (r));
      fill_rusage (&r, hMainProc);
      *rusage_in = r;
    }
  else if (intwho == RUSAGE_CHILDREN)
    *rusage_in = myself->rusage_children;
  else
    {
      set_errno (EINVAL);
      res = -1;
    }

  syscall_printf ("%d = getrusage (%d, %p)", res, intwho, rusage_in);
  return res;
}
