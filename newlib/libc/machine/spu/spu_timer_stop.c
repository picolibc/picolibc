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

/* SPU timer stop library service.  */
#include <spu_timer.h>
#include "spu_timer_internal.h"

/* Stop a timer.  Moves it from either the active or handled list to the
   stopped list. Returns 0 on sucess, timer was successfully stopped.
   Returns <0 - Failure:
    * SPU_TIMER_ERR_NOT_ACTIVE - timer was not active
    * SPU_TIMER_ERR_INVALID_ID - invalid timer id
    * SPU_TIMER_ERR_NOCLOCK    -  spu clock is not running  */
int
spu_timer_stop (int id)
{
  spu_timer_t *t, **pn;
  unsigned was_enabled;

  if (id < 0 || id >= SPU_TIMER_NTIMERS)
    return SPU_TIMER_ERR_INVALID_ID;

  if (__spu_clock_startcnt == 0)
    return SPU_TIMER_ERR_NOCLOCK;


  /* Free or stopped states.  */
  if (__spu_timers[id].state == SPU_TIMER_ACTIVE ||
      __spu_timers[id].state == SPU_TIMER_HANDLED)
    {
      was_enabled = spu_readch (SPU_RdMachStat) & 0x1;
      spu_idisable ();

      /* Timer is on either active list or handled list.  */
      t = (__spu_timers[id].state == SPU_TIMER_ACTIVE)
	? __spu_timers_active : __spu_timers_handled;

      pn = (__spu_timers[id].state == SPU_TIMER_ACTIVE)
	? &__spu_timers_active : &__spu_timers_handled;

      while (t && t->id != id)
	{
	  pn = &t->next;
	  t = t->next;
	}
#ifdef SPU_TIMER_DEBUG
      if (!t)
	ABORT (); /* Internal error.  */
#endif
      /* Fix timeout of next timer and decrementer if we were at the head of
         the active list.  */
      if (t->next)
	{
	  t->next->tmout += t->tmout;
	  if (__spu_timers_active == t)
	    __reset_spu_decr (t->next->tmout);
	}
      else
	__reset_spu_decr (CLOCK_START_VALUE);

      *pn = t->next; /* Update this elements to pointer.  */
      t->next = __spu_timers_stopped;
      __spu_timers_stopped = t;

      __spu_timers[id].state = SPU_TIMER_STOPPED;

      if (__likely (was_enabled))
	spu_ienable ();

      return 0;
    }

  return SPU_TIMER_ERR_NOT_ACTIVE;
}
