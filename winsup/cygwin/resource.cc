/* resource.cc: getrusage () and friends.

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2005, 2008, 2009, 2010,
   2011, 2012, 2013 Red Hat, Inc.

   Written by Steve Chamberlain (sac@cygnus.com), Doug Evans (dje@cygnus.com),
   Geoffrey Noer (noer@cygnus.com) of Cygnus Support.
   Rewritten by Sergey S. Okhapkin (sos@prospect.com.ru)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/param.h>
#include "pinfo.h"
#include "psapi.h"
#include "cygtls.h"
#include "path.h"
#include "fhandler.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"

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
  KERNEL_USER_TIMES kut;

  struct timeval tv;

  memset (&kut, 0, sizeof kut);
  memset (r, 0, sizeof *r);
  NtQueryInformationProcess (h, ProcessTimes, &kut, sizeof kut, NULL);
  totimeval (&tv, &kut.KernelTime, 0, 0);
  add_timeval (&r->ru_stime, &tv);
  totimeval (&tv, &kut.UserTime, 0, 0);
  add_timeval (&r->ru_utime, &tv);

  VM_COUNTERS vmc;
  NTSTATUS status = NtQueryInformationProcess (h, ProcessVmCounters, &vmc,
					       sizeof vmc, NULL);
  if (NT_SUCCESS (status))
    {
      r->ru_maxrss += (long) (vmc.WorkingSetSize / 1024);
      r->ru_majflt += vmc.PageFaultCount;
    }
}

extern "C" int
getrusage (int intwho, struct rusage *rusage_in)
{
  int res = 0;
  struct rusage r;

  if (intwho == RUSAGE_SELF)
    {
      memset (&r, 0, sizeof (r));
      fill_rusage (&r, GetCurrentProcess ());
      *rusage_in = r;
    }
  else if (intwho == RUSAGE_CHILDREN)
    *rusage_in = myself->rusage_children;
  else
    {
      set_errno (EINVAL);
      res = -1;
    }

  syscall_printf ("%R = getrusage(%d, %p)", res, intwho, rusage_in);
  return res;
}

extern "C" int
getrlimit (int resource, struct rlimit *rlp)
{
  MEMORY_BASIC_INFORMATION m;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  rlp->rlim_cur = RLIM_INFINITY;
  rlp->rlim_max = RLIM_INFINITY;

  switch (resource)
    {
    case RLIMIT_CPU:
    case RLIMIT_FSIZE:
    case RLIMIT_DATA:
    case RLIMIT_AS:
      break;
    case RLIMIT_STACK:
      if (!VirtualQuery ((LPCVOID) &m, &m, sizeof m))
	debug_printf ("couldn't get stack info, returning def.values. %E");
      else
	{
	  rlp->rlim_cur = (rlim_t) &m - (rlim_t) m.AllocationBase;
	  rlp->rlim_max = (rlim_t) m.BaseAddress + m.RegionSize
			  - (rlim_t) m.AllocationBase;
	}
      break;
    case RLIMIT_NOFILE:
      rlp->rlim_cur = getdtablesize ();
      if (rlp->rlim_cur < OPEN_MAX)
	rlp->rlim_cur = OPEN_MAX;
      rlp->rlim_max = OPEN_MAX_MAX;
      break;
    case RLIMIT_CORE:
      rlp->rlim_cur = cygheap->rlim_core;
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}

extern "C" int
setrlimit (int resource, const struct rlimit *rlp)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  struct rlimit oldlimits;

  /* Check if the request is to actually change the resource settings.
     If it does not result in a change, take no action and do not fail. */
  if (getrlimit (resource, &oldlimits) < 0)
    return -1;

  if (oldlimits.rlim_cur == rlp->rlim_cur &&
      oldlimits.rlim_max == rlp->rlim_max)
    /* No change in resource requirements, succeed immediately */
    return 0;

  if (rlp->rlim_cur > rlp->rlim_max)
    {
      set_errno (EINVAL);
      return -1;
    }

  switch (resource)
    {
    case RLIMIT_CORE:
      cygheap->rlim_core = rlp->rlim_cur;
      break;
    case RLIMIT_NOFILE:
      if (rlp->rlim_cur != RLIM_INFINITY)
	return setdtablesize (rlp->rlim_cur);
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}
