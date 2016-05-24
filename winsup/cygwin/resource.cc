/* resource.cc: getrusage () and friends.

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

/* Default stacksize in case RLIMIT_STACK is RLIM_INFINITY is 2 Megs with
   system-dependent number of guard pages.  The pthread stacksize does not
   include the guardpage size, so we have to subtract the default guardpage
   size.  Additionally the Windows stack handling disallows to commit the
   last page, so we subtract it, too. */
#define DEFAULT_STACKSIZE (2 * 1024 * 1024)
#define DEFAULT_STACKGUARD (wincap.def_guard_page_size() + wincap.page_size ())

muto NO_COPY rlimit_stack_guard;
static struct rlimit rlimit_stack = { 0, RLIM_INFINITY };

static void
__set_rlimit_stack (const struct rlimit *rlp)
{
  rlimit_stack_guard.init ("rlimit_stack_guard")->acquire ();
  rlimit_stack = *rlp;
  rlimit_stack_guard.release ();
}

static void
__get_rlimit_stack (struct rlimit *rlp)
{
  rlimit_stack_guard.init ("rlimit_stack_guard")->acquire ();
  if (!rlimit_stack.rlim_cur)
    {
      /* Fetch the default stacksize from the executable header... */
      PIMAGE_DOS_HEADER dosheader;
      PIMAGE_NT_HEADERS ntheader;

      dosheader = (PIMAGE_DOS_HEADER) GetModuleHandle (NULL);
      ntheader = (PIMAGE_NT_HEADERS) ((PBYTE) dosheader + dosheader->e_lfanew);
      rlimit_stack.rlim_cur = ntheader->OptionalHeader.SizeOfStackReserve;
      /* ...and subtract the guardpages. */
      rlimit_stack.rlim_cur -= DEFAULT_STACKGUARD;
    }
  *rlp = rlimit_stack;
  rlimit_stack_guard.release ();
}

size_t
get_rlimit_stack (void)
{
  struct rlimit rl;

  __get_rlimit_stack (&rl);
  /* RLIM_INFINITY doesn't make much sense.  As in glibc, use an
     "architecture-specific default". */
  if (rl.rlim_cur == RLIM_INFINITY)
    rl.rlim_cur = DEFAULT_STACKSIZE - DEFAULT_STACKGUARD;
  /* Always return at least minimum stacksize. */
  else if (rl.rlim_cur < PTHREAD_STACK_MIN)
    rl.rlim_cur = PTHREAD_STACK_MIN;
  return (size_t) rl.rlim_cur;
}

extern "C" int
getrlimit (int resource, struct rlimit *rlp)
{
  __try
    {
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
	  __get_rlimit_stack (rlp);
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
	  __leave;
	}
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

extern "C" int
setrlimit (int resource, const struct rlimit *rlp)
{
  struct rlimit oldlimits;

  /* Check if the request is to actually change the resource settings.
     If it does not result in a change, take no action and do not fail. */
  if (getrlimit (resource, &oldlimits) < 0)
    return -1;

  __try
    {
      if (oldlimits.rlim_cur == rlp->rlim_cur &&
	  oldlimits.rlim_max == rlp->rlim_max)
	/* No change in resource requirements, succeed immediately */
	return 0;

      if (rlp->rlim_cur > rlp->rlim_max)
	{
	  set_errno (EINVAL);
	  __leave;
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
	case RLIMIT_STACK:
	  __set_rlimit_stack (rlp);
	  break;
	default:
	  set_errno (EINVAL);
	  __leave;
	}
      return 0;
    }
  __except (EFAULT)
  __endtry
  return -1;
}
