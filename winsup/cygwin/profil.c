/* profil.c -- win32 profil.c equivalent

   Copyright 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/*
 * This file is taken from Cygwin distribution. Please keep it in sync.
 * The differences should be within __MINGW32__ guard.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include "profil.h"

#define SLEEPTIME (1000 / PROF_HZ)

/* global profinfo for profil() call */
static struct profinfo prof;

/* Get the pc for thread THR */

static size_t
get_thrpc (HANDLE thr)
{
  CONTEXT ctx;
  size_t pc;
  int res;

  res = SuspendThread (thr);
  if (res == -1)
    return (size_t) - 1;
  ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
  pc = (size_t) - 1;
  if (GetThreadContext (thr, &ctx)) {
#ifndef _WIN64
    pc = ctx.Eip;
#else
    pc = ctx.Rip;
#endif
  }
  ResumeThread (thr);
  return pc;
}

/* Display cell of profile buffer */
#if 0
static void
print_prof (struct profinfo *p)
{
  printf ("profthr %x\ttarget thr %x\n", p->profthr, p->targthr);
  printf ("pc: %x - %x\n", p->lowpc, p->highpc);
  printf ("scale: %x\n", p->scale);
  return;
}
#endif

/* Everytime we wake up use the main thread pc to hash into the cell in the
   profile buffer ARG. */

static void CALLBACK profthr_func (LPVOID);

static void CALLBACK
profthr_func (LPVOID arg)
{
  struct profinfo *p = (struct profinfo *) arg;
  size_t pc, idx;

  for (;;)
    {
      pc = (size_t) get_thrpc (p->targthr);
      if (pc >= p->lowpc && pc < p->highpc)
	{
	  idx = PROFIDX (pc, p->lowpc, p->scale);
	  p->counter[idx]++;
	}
#if 0
      print_prof (p);
#endif
      /* Check quit condition, WAIT_OBJECT_0 or WAIT_TIMEOUT */
      if (WaitForSingleObject (p->quitevt, SLEEPTIME) == WAIT_OBJECT_0)
	return;
    }
}

/* Stop profiling to the profiling buffer pointed to by P. */

static int
profile_off (struct profinfo *p)
{
  if (p->profthr)
    {
      SignalObjectAndWait (p->quitevt, p->profthr, INFINITE, FALSE);
      CloseHandle (p->quitevt);
      CloseHandle (p->profthr);
    }
  if (p->targthr)
    CloseHandle (p->targthr);
  return 0;
}

/* Create a timer thread and pass it a pointer P to the profiling buffer. */

static int
profile_on (struct profinfo *p)
{
  DWORD thrid;

  /* get handle for this thread */
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
			GetCurrentProcess (), &p->targthr, 0, FALSE,
			DUPLICATE_SAME_ACCESS))
    {
      errno = ESRCH;
      return -1;
    }

  p->quitevt = CreateEvent (NULL, TRUE, FALSE, NULL);

  if (!p->quitevt)
    {
      CloseHandle (p->quitevt);
      p->targthr = 0;
      errno = EAGAIN;
      return -1;
    }

  p->profthr = CreateThread (0, 0, (DWORD (WINAPI *)(LPVOID)) profthr_func,
                             (void *) p, 0, &thrid);

  if (!p->profthr)
    {
      CloseHandle (p->targthr);
      CloseHandle (p->quitevt);
      p->targthr = 0;
      errno = EAGAIN;
      return -1;
    }

  /* Set profiler thread priority to highest to be sure that it gets the
     processor as soon it request it (i.e. when the Sleep terminate) to get
     the next data out of the profile. */

  SetThreadPriority (p->profthr, THREAD_PRIORITY_TIME_CRITICAL);

  return 0;
}

/*
 * start or stop profiling
 *
 * profiling goes into the SAMPLES buffer of size SIZE (which is treated
 * as an array of u_shorts of size size/2)
 *
 * each bin represents a range of pc addresses from OFFSET.  The number
 * of pc addresses in a bin depends on SCALE.  (A scale of 65536 maps
 * each bin to two addresses, A scale of 32768 maps each bin to 4 addresses,
 * a scale of 1 maps each bin to 128k addreses).  Scale may be 1 - 65536,
 * or zero to turn off profiling
 */
int
profile_ctl (struct profinfo * p, char *samples, size_t size,
	     size_t offset, u_int scale)
{
  size_t maxbin;

  if (scale > 65536)
    {
      errno = EINVAL;
      return -1;
    }

  profile_off (p);
  if (scale)
    {
      memset (samples, 0, size);
      memset (p, 0, sizeof *p);
      maxbin = size >> 1;
      prof.counter = (u_short *) samples;
      prof.lowpc = offset;
      prof.highpc = PROFADDR (maxbin, offset, scale);
      prof.scale = scale;

      return profile_on (p);
    }
  return 0;
}

/* Equivalent to unix profil()
   Every SLEEPTIME interval, the user's program counter (PC) is examined:
   offset is subtracted and the result is multiplied by scale.
   The word pointed to by this address is incremented.  Buf is unused. */

int
profil (char *samples, size_t size, size_t offset, u_int scale)
{
  return profile_ctl (&prof, samples, size, offset, scale);
}

