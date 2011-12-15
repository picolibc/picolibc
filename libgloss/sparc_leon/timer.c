/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/timer.h>
#include <asm-leon/leoncompat.h>

// '''''''''''''''''''''''''''''''''''''''''''''''''''''

TAILQ_HEAD (timer_queue, timerevent) timers = TAILQ_HEAD_INITIALIZER (timers);

     int
     addtimer (struct timerevent *e)
{
  struct timerevent *next;
  unsigned long old = leonbare_disable_traps ();
  TAILQ_FOREACH (next, &timers, n)
  {
    if (!GT_TIMESPEC (e->expire, next->expire))
      break;
  }
  if (next)
    {
      TAILQ_INSERT_BEFORE (next, e, n);
    }
  else
    {
      TAILQ_INSERT_TAIL (&timers, e, n);
    }
  leonbare_enable_traps (old);
}

extern unsigned long noalarm;
void
settimer ()
{
  struct timeval tv, te;
  struct timerevent *e = TAILQ_FIRST (&timers), *n;
  while (e)
    {
      n = TAILQ_NEXT (e, n);
      te.tv_sec = e->expire.tv_sec;
      te.tv_usec = e->expire.tv_nsec / NSEC_PER_USEC;
      do_gettimeofday (&tv);
      if (GT_TIMEVAL (te, tv))
	{
	  MINUS_TIMEVAL (te, te, tv);
	  if (!tv.tv_sec || te.tv_usec <= tick_usec)
	    {
	      if (!noalarm)
		{
		  //---------------------
		  switch (LEONCOMPAT_VERSION)
		    {
		    case 3:
		    default:
		      LEON3_GpTimer_Regs->e[1].val = 0;
		      LEON3_GpTimer_Regs->e[1].rld = te.tv_usec - 1;
		      LEON3_GpTimer_Regs->e[1].ctrl = 0;
		      LEON3_GpTimer_Regs->e[1].ctrl =
			LEON3_GPTIMER_EN |
			LEON3_GPTIMER_LD | LEON3_GPTIMER_IRQEN;
		      break;
		    }
		}
	      //---------------------
	    }
	}
      else
	{
	  unsigned long old = leonbare_disable_traps ();
	  TAILQ_REMOVE (&timers, e, n);
	  e->handler (e->arg);
	  leonbare_enable_traps (old);
	}
      e = n;
    }
}

int
Timer_getTimer1 (unsigned int **count, unsigned int **reload,
		 unsigned int **ctrl)
{
  //---------------------
  switch (LEONCOMPAT_VERSION)
    {
    case 3:
    default:
      amba_init ();
      *count = (unsigned int *) &(LEON3_GpTimer_Regs->e[0].val);
      *reload = (unsigned int *) &(LEON3_GpTimer_Regs->e[0].rld);
      *ctrl = (unsigned int *) &(LEON3_GpTimer_Regs->e[0].ctrl);
      break;
    }
  //---------------------
  return 1;
}

int
Timer_getTimer2 (unsigned int **count, unsigned int **reload,
		 unsigned int **ctrl)
{
  //---------------------
  switch (LEONCOMPAT_VERSION)
    {
    case 3:
    default:
      amba_init ();
      if (!noalarm)
	{
	  *count = (unsigned int *) &(LEON3_GpTimer_Regs->e[1].val);
	  *reload = (unsigned int *) &(LEON3_GpTimer_Regs->e[1].rld);
	  *ctrl = (unsigned int *) &(LEON3_GpTimer_Regs->e[1].ctrl);
	  break;
	}
      return 0;
    }
  //---------------------
  return 1;
}
