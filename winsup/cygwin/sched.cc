/* sched.cc: scheduler interface for Cygwin

   Copyright 2001  Red Hat, Inc.

   Written by Robert Collins <rbtcollins@hotmail.com>

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "winsup.h"
#include <limits.h>
#include <errno.h>
#include "cygerrno.h"
#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include <sched.h>
#include "sigproc.h"
#include "pinfo.h"
/* for getpid */
#include <unistd.h>


/* Win32 priority to UNIX priority Mapping.
   For now, I'm just following the spec: any range of priorities is ok.
   There are probably many many issues with this...
  
   We don't want process's going realtime. Well, they probably could, but the issues 
   with avoiding the priority values 17-22 and 27-30 (not supported before win2k)
   make that inefficient. 
   However to complicate things most unixes use lower is better priorities.
  
   So we map -14 to 15, and 15 to 1 via (16- ((n+16) >> 1)) 
   we then map 1 to 15 to various process class and thread priority combinations
  
   Then we need to look at the threads vi process priority. As win95 98 and NT 4 
   Don't support opening threads cross-process (unless a thread HANDLE is passed around)
   for now, we'll just use the priority class. 
  
   The code and logic are present to calculate the priority for thread
   , if a thread handle can be obtained. Alternatively, if the symbols wouldn't be 
   resolved until they are used
   we could support this on windows 2000 and ME now, and just fall back to the 
   class only on pre win2000 machines.
  
   Lastly, because we can't assume that the pid we're given are Windows pids, we can't
   alter non-cygwin started programs. 
*/

extern "C"
{

/* max priority for policy */
int
sched_get_priority_max (int policy)
{
  if (policy < 1 || policy > 3)
    {
      set_errno (EINVAL);
      return -1;
    }
  return -14;
}

/* min priority for policy */
int
sched_get_priority_min (int policy)
{
  if (policy < 1 || policy > 3)
    {
      set_errno (EINVAL);
      return -1;
    }
  return 15;
}

/* get sched params for process

   Note, I'm never returning EPERM, 
   Always ESRCH. This is by design (If cygwin ever looks at paranoid security
   Walking the pid values is a known hole in some os's) 
*/
int
sched_getparam (pid_t pid, struct sched_param *param)
{
  pid_t localpid;
  int winpri;
  if (!param || pid < 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  localpid = pid ? pid : getpid ();

  DWORD Class;
  int ThreadPriority;
  HANDLE process;
  pinfo p (localpid);

  /* get the class */

  if (!p)
    {
      set_errno (ESRCH);
      return -1;
    }
  process = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, p->dwProcessId);
  if (!process)
    {
      set_errno (ESRCH);
      return -1;
    }
  Class = GetPriorityClass (process);
  CloseHandle (process);
  if (!Class)
    {
      set_errno (ESRCH);
      return -1;
    }
  ThreadPriority = THREAD_PRIORITY_NORMAL;

  /* calculate the unix priority.

     FIXME: windows 2000 supports ABOVE_NORMAL and BELOW_NORMAL class's
     So this logic just defaults those class factors to NORMAL in the calculations */

  switch (Class)
    {
    case IDLE_PRIORITY_CLASS:
      switch (ThreadPriority)
	{
	case THREAD_PRIORITY_IDLE:
	  winpri = 1;
	  break;
	case THREAD_PRIORITY_LOWEST:
	  winpri = 2;
	  break;
	case THREAD_PRIORITY_BELOW_NORMAL:
	  winpri = 3;
	  break;
	case THREAD_PRIORITY_NORMAL:
	  winpri = 4;
	  break;
	case THREAD_PRIORITY_ABOVE_NORMAL:
	  winpri = 5;
	  break;
	case THREAD_PRIORITY_HIGHEST:
	default:
	  winpri = 6;
	  break;
	}
      break;
    case HIGH_PRIORITY_CLASS:
      switch (ThreadPriority)
	{
	case THREAD_PRIORITY_IDLE:
	  winpri = 1;
	  break;
	case THREAD_PRIORITY_LOWEST:
	  winpri = 11;
	  break;
	case THREAD_PRIORITY_BELOW_NORMAL:
	  winpri = 12;
	  break;
	case THREAD_PRIORITY_NORMAL:
	  winpri = 13;
	  break;
	case THREAD_PRIORITY_ABOVE_NORMAL:
	  winpri = 14;
	  break;
	case THREAD_PRIORITY_HIGHEST:
	default:
	  winpri = 15;
	  break;
	}
      break;
    case NORMAL_PRIORITY_CLASS:
    default:
      switch (ThreadPriority)
	{
	case THREAD_PRIORITY_IDLE:
	  winpri = 1;
	  break;
	case THREAD_PRIORITY_LOWEST:
	  winpri = 7;
	  break;
	case THREAD_PRIORITY_BELOW_NORMAL:
	  winpri = 8;
	  break;
	case THREAD_PRIORITY_NORMAL:
	  winpri = 9;
	  break;
	case THREAD_PRIORITY_ABOVE_NORMAL:
	  winpri = 10;
	  break;
	case THREAD_PRIORITY_HIGHEST:
	default:
	  winpri = 11;
	  break;
	}
      break;
    }

  /* reverse out winpri = (16- ((unixpri+16) >> 1)) */
  /*  
     winpri-16 = -  (unixpri +16 ) >> 1

     -(winpri-16) = unixpri +16 >> 1
     (-(winpri-16)) << 1 = unixpri+16
     ((-(winpri - 16)) << 1) - 16 = unixpri
   */

  param->sched_priority = ((-(winpri - 16)) << 1) - 16;

  return 0;
}

/* get the scheduler for pid

   All process's on WIN32 run with SCHED_FIFO.
   So we just give an answer. 
   (WIN32 uses a multi queue FIFO).
*/
int
sched_getscheduler (pid_t pid)
{
  if (pid < 0)
    return ESRCH;
  else
    return SCHED_FIFO;
}

/* get the time quantum for pid

   We can't return -11, errno ENOSYS, because that implies that
   sched_get_priority_max & min are also not supported (according to the spec)
   so some spec-driven autoconf tests will likely assume they aren't present either
  
   returning ESRCH might confuse some applications (if they assumed that when 
   rr_get_interval is called on pid 0 it always works). 
  
   If someone knows the time quanta for the various win32 platforms, then a 
   simple check for the os we're running on will finish this function
*/
int
sched_rr_get_interval (pid_t pid, struct timespec *interval)
{
  set_errno (ESRCH);
  return -1;
}

/* set the scheduling parameters */
int
sched_setparam (pid_t pid, const struct sched_param *param)
{
  pid_t localpid;
  int winpri;
  DWORD Class;
  int ThreadPriority;
  HANDLE process;

  if (!param || pid < 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  if (param->sched_priority < -14 || param->sched_priority > 15)
    {
      set_errno (EINVAL);
      return -1;
    }

  /*  winpri = (16- ((unixpri+16) >> 1)) */
  winpri = 16 - ((param->sched_priority + 16) >> 1);

  /* calculate our desired priority class and thread priority */

  if (winpri < 7)
    Class = IDLE_PRIORITY_CLASS;
  else if (winpri > 10)
    Class = HIGH_PRIORITY_CLASS;
  else
    Class = NORMAL_PRIORITY_CLASS;

  switch (Class)
    {
    case IDLE_PRIORITY_CLASS:
      switch (winpri)
	{
	case 1:
	  ThreadPriority = THREAD_PRIORITY_IDLE;
	  break;
	case 2:
	  ThreadPriority = THREAD_PRIORITY_LOWEST;
	  break;
	case 3:
	  ThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
	  break;
	case 4:
	  ThreadPriority = THREAD_PRIORITY_NORMAL;
	  break;
	case 5:
	  ThreadPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	  break;
	case 6:
	  ThreadPriority = THREAD_PRIORITY_HIGHEST;
	  break;
	}
      break;
    case NORMAL_PRIORITY_CLASS:
      switch (winpri)
	{
	case 7:
	  ThreadPriority = THREAD_PRIORITY_LOWEST;
	  break;
	case 8:
	  ThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
	  break;
	case 9:
	  ThreadPriority = THREAD_PRIORITY_NORMAL;
	  break;
	case 10:
	  ThreadPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	  break;
	case 11:
	  ThreadPriority = THREAD_PRIORITY_HIGHEST;
	  break;
	}
      break;
    case HIGH_PRIORITY_CLASS:
      switch (winpri)
	{
	case 12:
	  ThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
	  break;
	case 13:
	  ThreadPriority = THREAD_PRIORITY_NORMAL;
	  break;
	case 14:
	  ThreadPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	  break;
	case 15:
	  ThreadPriority = THREAD_PRIORITY_HIGHEST;
	  break;
	}
      break;
    }

  localpid = pid ? pid : getpid ();

  pinfo p (localpid);

  /* set the class */

  if (!p)
    {
      set_errno (1);		//ESRCH);
      return -1;
    }
  process =
    OpenProcess (PROCESS_SET_INFORMATION, FALSE, (DWORD) p->dwProcessId);
  if (!process)
    {
      set_errno (2);		//ESRCH);
      return -1;
    }
  if (!SetPriorityClass (process, Class))
    {
      CloseHandle (process);
      set_errno (EPERM);
      return -1;
    }
  CloseHandle (process);

  return 0;
}

/* set the scheduler */
int
sched_setscheduler (pid_t pid, int policy,
		    const struct sched_param *param)
{
  /* on win32, you can't change the scheduler. Doh! */
  set_errno (ENOSYS);
  return -1;
}

/* yield the cpu */
int
sched_yield (void)
{
  Sleep (0);
  return 0;
}
}
