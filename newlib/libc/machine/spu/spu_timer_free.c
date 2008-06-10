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

/* SPU timer free library service.  */
#include <spu_timer.h>
#include "spu_timer_internal.h"


/* Frees an allocated timer. The timer must be in the stopped state for this
   to succeed. Maybe be called:
    * after allocated, before it's started
    * after it's been explicitly stopped
   Returns 0 on success, timer sucessfully deallocated. Returns <0 on failure
    * SPU_TIMER_INVALID_ID - id out of range
    * SPU_TIMER_ERR_FREE - id in free state
    * SPU_TIMER_ERR_ACTIVE - id in handled or active state  */
int
spu_timer_free (int id)
{
  spu_timer_t *t, **pn;
  unsigned was_enabled;

  if (id < 0 || id >= SPU_TIMER_NTIMERS)
    return SPU_TIMER_ERR_INVALID_ID;

  if (__spu_timers[id].state == SPU_TIMER_STOPPED)
    {

      was_enabled = spu_readch (SPU_RdMachStat) & 0x1;
      spu_idisable ();

      t = __spu_timers_stopped;
      pn = &__spu_timers_stopped;

      while (t && (t->id != id))
	{
	  pn = &t->next;
	  t = t->next;
	}
#ifdef SPU_TIMER_DEBUG
      if (!t)
	ABORT ();
#endif
      *pn = t->next;

      /* Add timer back to free list (mask).  */
      __spu_timers_avail |= (1 << (id));
      __spu_timers[id].state = SPU_TIMER_FREE;

      if (__likely (was_enabled))
	spu_ienable ();

      return 0;
    }

  /* Handle invalid states.  */
  return (__spu_timers[id].state == SPU_TIMER_FREE) ?
	  SPU_TIMER_ERR_FREE : SPU_TIMER_ERR_ACTIVE;
}
