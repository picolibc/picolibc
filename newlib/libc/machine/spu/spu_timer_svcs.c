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

/* SPU timer start and alloc library services.  */
#include <spu_timer.h>
#include "spu_timer_internal.h"

/* The timers.  */
spu_timer_t __spu_timers[SPU_TIMER_NTIMERS] __attribute__ ((aligned (16)));

/* Active timer list.  */
spu_timer_t *__spu_timers_active;

/* Stopped (allocated) timer list.  */
spu_timer_t *__spu_timers_stopped;

/* List of timers being handled.  */
spu_timer_t *__spu_timers_handled;

/* Bitmask of available timers.  */
unsigned __spu_timers_avail =
  ((1 << (SPU_TIMER_NTIMERS - 1)) + ((1 << (SPU_TIMER_NTIMERS - 1)) - 1));

/* Allocates an SPU interval timer and returns the timer ID. Must be called
   before starting a timer. interval specifies the expiration interval in
   timebase units. func specifies the function pointer to expiration handler.
   Returns the timer ID on success or <0 on Failure:
    * SPU_TIMER_ERR_NONE_FREE - no free timers to allocate
    * SPU_TIMER_ERR_INVALID_PARM - invalid parm  */
int
spu_timer_alloc (int interval, void (*func) (int))
{
  unsigned was_enabled;
  int id;
  if (interval < MIN_INTVL || interval > MAX_INTVL || func == NULL)
    return SPU_TIMER_ERR_INVALID_PARM;

  was_enabled = spu_readch (SPU_RdMachStat) & 0x1;

  /* Get id of next available timer.  */
  id = spu_extract ((spu_sub ((unsigned) 31,
				  spu_cntlz (spu_promote
					     (__spu_timers_avail, 0)))), 0);

  /* No timers avail.  */
  if (id == -1)
    return SPU_TIMER_ERR_NONE_FREE;

  /* Higher order bits represent lower timer ids.  */
  __spu_timers_avail &= ~(1 << (id));
  id = (SPU_TIMER_NTIMERS - 1) - id;

  /* Initialize timer and put it on stopped list.  */
  (__spu_timers + id)->func = func;
  (__spu_timers + id)->intvl = interval;
  (__spu_timers + id)->id = id;
  (__spu_timers + id)->state = SPU_TIMER_STOPPED;

  spu_idisable ();
  (__spu_timers + id)->next = __spu_timers_stopped;
  __spu_timers_stopped = &__spu_timers[id];

  if (__likely (was_enabled))
    spu_ienable ();
  return id;
}

/* External interface for starting a timer.  See description of
   __spu_timer_start(). Returns 0 on success and <0 on failure:
    * SPU_TIMER_ERR_INVALID_ID - invalid id
    * SPU_TIMER_ERR_NOCLOCK - clock is off
    * SPU_TIMER_ERR_NOT_STOPPED - timer not in stopped state  */
int
spu_timer_start (int id)
{
  if (id < 0 || id >= SPU_TIMER_NTIMERS)
    return SPU_TIMER_ERR_INVALID_ID;

  if (__spu_clock_startcnt == 0)
    return SPU_TIMER_ERR_NOCLOCK;

  if (__spu_timers[id].state != SPU_TIMER_STOPPED)
    return SPU_TIMER_ERR_NOT_STOPPED;

  __spu_timer_start (id, 1);

  return 0;
}
