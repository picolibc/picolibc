/* profil.c -- win32 profil.c equivalent

   Copyright 1998 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>

#include <profil.h>

#define SLEEPTIME (1000 / PROF_HZ)

/* global profinfo for profil() call */
static struct profinfo prof;

/* Get the pc for thread THR */

static u_long
get_thrpc (HANDLE thr)
{
  CONTEXT ctx;
  u_long pc;
  int res;

  res = SuspendThread (thr);
  if (res == -1)
    return (u_long) - 1;
  ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
  pc = (u_long) - 1;
  if (GetThreadContext (thr, &ctx))
    pc = ctx.Eip;
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

static DWORD CALLBACK
profthr_func (LPVOID arg)
{
  struct profinfo *p = (struct profinfo *) arg;
  u_long pc, idx;

  for (;;)
    {
      pc = (u_long) get_thrpc (p->targthr);
      if (pc >= p->lowpc && pc < p->highpc)
	{
	  idx = PROFIDX (pc, p->lowpc, p->scale);
	  p->counter[idx]++;
	}
#if 0
      print_prof (p);
#endif
      Sleep (SLEEPTIME);
    }
  return 0;
}

/* Stop profiling to the profiling buffer pointed to by P. */

static int
profile_off (struct profinfo *p)
{
  if (p->profthr)
    {
      TerminateThread (p->profthr, 0);
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
  int thrid;

  /* get handle for this thread */
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
			GetCurrentProcess (), &p->targthr, 0, FALSE,
			DUPLICATE_SAME_ACCESS))
    {
      errno = ESRCH;
      return -1;
    }

  p->profthr = CreateThread (0, 0, profthr_func, (void *) p, 0, &thrid);
  if (!p->profthr)
    {
      CloseHandle (p->targthr);
      p->targthr = 0;
      errno = EAGAIN;
      return -1;
    }
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
	     u_long offset, u_int scale)
{
  u_long maxbin;

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
profil (char *samples, size_t size, u_long offset, u_int scale)
{
  return profile_ctl (&prof, samples, size, offset, scale);
}

