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
#include <asm-leon/elfmacro.h>
#include <asm-leon/leon.h>
#include <asm-leon/irq.h>
#include <asm-leon/jiffies.h>
#include <asm-leon/param.h>
#include <asm-leon/leoncompat.h>

static __inline__ unsigned long do_gettimeoffset ();


extern int *rtc;

unsigned long wall_jiffies = INITIAL_JIFFIES;
unsigned long tick_nsec = TICK_NSEC;
unsigned long tick_usec = TICK_NSEC / 1000;
unsigned long seperateirq = 1;
unsigned long noalarm = 1;
unsigned long force_noalarm = 0;
unsigned long nodotimer = 0;
int leonbare_hz = HZ;


void settimer ();
inline void
do_timer (struct leonbare_pt_regs *regs)
{
  unsigned long ticks;
  jiffies_64++;
  ticks = jiffies - wall_jiffies;
  if (ticks)
    {
      wall_jiffies += ticks;
      do
	{
	  ticks--;
	  xtime.tv_nsec += tick_nsec;
	  if (xtime.tv_nsec >= 1000000000)
	    {
	      xtime.tv_nsec -= 1000000000;
	      xtime.tv_sec++;
	    }
	}
      while (ticks);
    }
  settimer ();
}

int
leonbare_alarm (int irq, void *arg, struct leonbare_pt_regs *regs)
{
  settimer ();
}

extern clock_t (*clock_custom) (void);
clock_t
leonbare_clock_custom ()
{
  int hz = leonbare_hz ? leonbare_hz : HZ;
  return (clock_t) ((jiffies * (CLOCK_TICK_RATE / hz)) + do_gettimeoffset ());
}


tickerhandler ticker_callback = 0;
int
leonbare_tick (int irq, void *arg, struct leonbare_pt_regs *regs)
{
  unsigned int ctrl;
  if (!seperateirq)
    {
      /* only leon3 comes here */
      if (!noalarm)
	{
	  ctrl = LEON3_GpTimer_Regs->e[1].ctrl;
	  if (ctrl & LEON3_GPTIMER_IP)
	    {
	      leonbare_alarm (irq, arg, regs);
	      LEON3_GpTimer_Regs->e[1].ctrl = ctrl & ~LEON3_GPTIMER_IP;
	    }
	}
      ctrl = LEON3_GpTimer_Regs->e[0].ctrl;
      if (!(ctrl & LEON3_GPTIMER_IP))
	{
	  return 0;
	}
      LEON3_GpTimer_Regs->e[0].ctrl = ctrl & ~LEON3_GPTIMER_IP;
    }

  if (!nodotimer)
    {
      do_timer (regs);
    }
  if (ticker_callback)
    {
      ticker_callback (regs);
    }
  return 0;
}

static struct irqaction irqact1 = { (irqhandler) leonbare_tick, 0, 0, 0 };
static struct irqaction irqact2 = { (irqhandler) leonbare_alarm, 0, 0, 0 };

void
leonbare_init_ticks ()
{
  int i, irq1 = 0, irq2 = 0;
  int hz = leonbare_hz ? leonbare_hz : HZ;
  amba_apb_device dev[1];

  //---------------------
  switch (LEONCOMPAT_VERSION)
    {
    case 3:
    default:
      amba_init ();
      if (LEON3_GpTimer_Regs && LEON3_IrqCtrl_Regs)
	{
	  if ((LEON3_GpTimer_Regs->config & LEON3_GPTIMER_CONFIG_TIMERMASK) >=
	      2 && force_noalarm == 0)
	    noalarm = 0;
	  if (!(LEON3_GpTimer_Regs->config & LEON3_GPTIMER_CONFIG_SEPERATE))
	    seperateirq = 0;
	  LEON3_GpTimer_Regs->e[0].val = 0;
	  LEON3_GpTimer_Regs->e[0].rld = (((CLOCK_TICK_RATE / hz) - 1));
	  LEON3_GpTimer_Regs->e[0].ctrl = 0;
	  LEON3_GpTimer_Regs->e[0].ctrl =
	    LEON3_GPTIMER_EN |
	    LEON3_GPTIMER_RL | LEON3_GPTIMER_LD | LEON3_GPTIMER_IRQEN;
	  irq1 = LEON3_GpTimer_Irq;
	  irq2 = LEON3_GpTimer_Irq + 1;
	}
      break;
    }
  //---------------------

  if (irq1)
    {
      clock_custom = leonbare_clock_custom;
      chained_catch_interrupt (irq1, &irqact1);
      leonbare_enable_irq (irq1);
    }
  if (irq2 && (!noalarm) && seperateirq)
    {
      chained_catch_interrupt (irq2, &irqact2);
      leonbare_enable_irq (irq2);
    }
}


//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

static __inline__ unsigned long
do_gettimeoffset ()
{
  unsigned long usec = 0;
  //---------------------
  switch (LEONCOMPAT_VERSION)
    {
    case 3:
    default:
      usec = ((LEON3_GpTimer_Regs->e[0].rld & 0x7fffff) -
	      (LEON3_GpTimer_Regs->e[0].val & 0x7fffff));
      break;
    }
  //---------------------
  return usec;
}

/* get usec (timeval)  resolution,
 * could use nsec (timespec) because pthread use it  (todo) */
void
do_gettimeofday (struct timeval *tv)
{

  unsigned long flags;
  unsigned long seq;
  unsigned long usec, sec;

  do
    {
      unsigned long lost;
      seq = jiffies;

      usec = do_gettimeoffset ();
      lost = jiffies - wall_jiffies;

      if (unlikely (lost))
	{
	  usec += lost * tick_usec;
	}

      sec = xtime.tv_sec;
      usec += (xtime.tv_nsec / 1000);
    }
  while (seq != jiffies);

  while (usec >= 1000000)
    {
      usec -= 1000000;
      sec++;
    }

  tv->tv_sec = sec;
  tv->tv_usec = usec;
}

int
gettimeofday (struct timeval *__p, void *__tz)
{
  do_gettimeofday (__p);
  return 0;
}

//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

static int
do_settimeofday (struct timespec *tv)
{
  time_t sec = tv->tv_sec;
  long nsec = tv->tv_nsec;

  if ((unsigned long) nsec >= NSEC_PER_SEC)
    return EINVAL;

  /*
   * This is revolting. We need to set "xtime" correctly. However, the
   * value in this location is the value at the most recent update of
   * wall time.  Discover what correction gettimeofday() would have
   * made, and then undo it!
   */
  nsec -= 1000 * (do_gettimeoffset () +
		  (jiffies - wall_jiffies) * (USEC_PER_SEC / HZ));

  set_normalized_timespec (&xtime, sec, nsec);
  return 0;
}

int
settimeofday (const struct timeval *tv, const struct timezone *tz)
{
  struct timespec ts;
  ts.tv_sec = tv->tv_sec;
  ts.tv_nsec = tv->tv_usec * NSEC_PER_USEC;
  return do_settimeofday (&ts);
}
