/*
(C) Copyright IBM Corp. 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/* Second Level Interrupt handler and related services for SPU timers.  */
#include "spu_timer_internal.h"
/* Resets decrementer to the specified value. Also updates software timebase
   to account for the time between the last decrementer reset and now. There
   are two cases:
    * Called by application to start a new timer.
    * Called by spu_clock to active the next timer.
   In both cases, the amount of time is the current interval timeout minus the
   current decrementer value.  */
void
__reset_spu_decr (int val)
{

  /* The interrupt occurs when the msb goes from 0 to 1 or when the decrementer
     goes from 0 to -1.  To be precisely accurate we should set the timer to
     the intverval -1, unless the interval passed in is 0 in which case it
     should be left at 0.  */
  int enable_val = (__likely (val)) ? val - 1 : 0;

  /* Decrementer must be stopped before writing it - minimize the time
     stopped.  */
  unsigned mask = __disable_spu_decr ();

  /* Perform tb correction before resettting the decrementer. the corrected
     value is the current timeout value minus the current decrementer value.
     Occasionally the read returns 0 - a second read will clear this
     condition.  */
  spu_readch (SPU_RdDec);
  int decval = spu_readch (SPU_RdDec);
  /* Restart decrementer with next timeout val.  */
  __enable_spu_decr (enable_val, mask);

  /* Update the timebase values before enabling for interrupts.  */
  __spu_tb_val += __spu_tb_timeout - decval;
  __spu_tb_timeout = enable_val;
}

/* Update software timebase and timeout value for the 'next to expire' timer.
   Called when starting a new timer so the timer list will have timeouts
   relative to the current time.  */
static inline void
__update_spu_tb_val (void)
{
  int elapsed = __spu_tb_timeout - spu_readch (SPU_RdDec);
#ifdef SPU_TIMER_DEBUG
  if (elapsed < 0)
    ABORT ();
#endif
  __spu_tb_val += elapsed;

  /* Adjust the timeout for the timer next to expire. Note this could cause
     the timeout to go negative, if it was just about to expire when we called
     spu_timer_start.  This is OK, since this can happen any time interrupts
     are disabled. We just schedule an immediate timeout in this case.  */
  if (__spu_timers_active)
    {
      __spu_timers_active->tmout -= elapsed;
      if (__spu_timers_active->tmout < 0)
	__spu_timers_active->tmout = 0;
    }
}

/* Add an allocated timer to the active list. The active list is sorted by
   timeout value. The timer at the head of the list is the timer that will
   expire next.  The rest of the timers have a timeout value that is relative
   to the timer ahead of it on the list.  This relative value is determined
   here, when the timer is added to the active list. When its position in the
   list is found, the timer's timeout value is set to its interval minus the
   sum of all the timeout values ahead of it.  The timeout value for the timer
   following the newly added timer is then adjusted to a new relative value. If
   the newly added timer is at the head of the list, the decrementer is reset.
   This function is called by SLIH to restart multiple timers (reset == 0) or
   by spu_timer_start() to start a single timer (reset == 1).  */
void
__spu_timer_start (int id, int reset)
{
  spu_timer_t *t;
  spu_timer_t **pn;
  spu_timer_t *start = &__spu_timers[id];
  unsigned tmout_time = 0;
  unsigned my_intvl = start->intvl;
  unsigned was_enabled = spu_readch (SPU_RdMachStat) & 0x1;

  spu_idisable ();

  t = __spu_timers_active;
  pn = &__spu_timers_active;

  /* If the active list is empty, just add the timer with the timeout set to
     the interval. Otherwise find the place in the list for the timer, setting
     its timeout to its interval minus the sum of timeouts ahead of it.  */
  start->state = SPU_TIMER_ACTIVE;
  if (__likely (!t))
    {
      __spu_timers_active = start;
      start->next = NULL;
      start->tmout = my_intvl;
    }
  else
    {

      /* Update swtb and timeout val of the next timer, so all times are
         relative to now.  */
      if (reset)
	__update_spu_tb_val ();

      while (t && (my_intvl >= (tmout_time + t->tmout)))
	{
	  tmout_time += t->tmout;
	  pn = &t->next;;
	  t = t->next;
	}
      start->next = t;
      start->tmout = my_intvl - tmout_time;
      *pn = start;

      /* Adjust timeout for timer after us.  */
      if (t)
	t->tmout -= start->tmout;
    }

  if (reset && (__spu_timers_active == start))
    __reset_spu_decr (__spu_timers_active->tmout);

  if (__unlikely (was_enabled))
    spu_ienable ();
}

/* SLIH for decrementer.  Manages software timebase and timers.
   Called by SPU FLIH. Assumes decrementer is still running
   (event not yet acknowledeged).  */
unsigned int
spu_clock_slih (unsigned status)
{
  int decr_reset_val;
  spu_timer_t *active, *handled;
  unsigned was_enabled = spu_readch (SPU_RdMachStat) & 0x1;

  status &= ~MFC_DECREMENTER_EVENT;

  spu_idisable ();

  /* The decrementer has now expired.  The decrementer event was acknowledged
     in the FLIH but not disabled. The decrementer will continue to run while
     we're running the clock/timer handler. The software clock keeps running,
     and accounts for all the time spent running handlers. Add the current
     timeout to the software timebase and set the timeout to DECR_MAX. This
     allows the "clock read" code to continue to work while we're in here, and
     gives us the most possible time to finish before another underflow.  */
  __spu_tb_val += __spu_tb_timeout;
  __spu_tb_timeout = DECR_MAX;

  /* For all timers that have the current timeout value, move them from the
     active list to the handled list and call their handlers. Note that the
     handled/stopped lists may be manipulated by the handlers if they wish to
     stop/free the timers. Note that only the first expired timer will reflect
     the real timeout value; the rest of the timers that had the same timeout
     value will have a relative value of zero.  */
  if (__spu_timers_active)
    {
      __spu_timers_active->tmout = 0;
      while ((active = __spu_timers_active)
	     && (active->tmout <= TIMER_INTERVAL_WINDOW))
	{
	  __spu_timers_active = active->next;
	  active->next = __spu_timers_handled;
	  __spu_timers_handled = active;
	  active->state = SPU_TIMER_HANDLED;
	  (*active->func) (active->id);
	}
    }

  /* put the handled timers back on the list and restart decrementer.  */
  while ((handled = __spu_timers_handled) != NULL)
    {
      __spu_timers_handled = handled->next;
      __spu_timer_start (handled->id, 0);
    }

  /* Reset the decrementer before returning. If we have any active timers, we
     set it to the timeout value for the timer at the head of the list, else
     the default clock value.  */
  decr_reset_val = __spu_timers_active ? __spu_timers_active->tmout : CLOCK_START_VALUE;

  __reset_spu_decr (decr_reset_val);

  if (__likely (was_enabled))
    spu_ienable ();

  return status;
}
