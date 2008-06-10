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

/* SPU clock stop library service.  */
#include <spu_timer.h>
#include "spu_timer_internal.h"

/* Stops the SPU clock:
    * decrements clock start count
    * when count is zero, disables the decrementer event and stops the
      decrementer
   Returns 0 on success and  <0 on failure:
    * SPU_CLOCK_ERR_NOT_RUNNING - clock was already off
    * SPU_CLOCK_ERR_TIMERS_ACTIVE - active timers exist
    * SPU_CLOCK_ERR_STILL_RUNNING - start count was decremented but clock was
      not stopped  */
int
spu_clock_stop (void)
{
  if (__spu_clock_startcnt == 0)
    return SPU_CLOCK_ERR_NOT_RUNNING;

  if (__spu_clock_startcnt == 1 && (__spu_timers_active || __spu_timers_handled))
    return SPU_CLOCK_ERR_TIMERS_ACTIVE;

  /* Don't stop clock if the clock is still in use.  */
  if (--__spu_clock_startcnt != 0)
    return SPU_CLOCK_ERR_STILL_RUNNING;

  /* Clock stopped, stop decrementer.  */
  __disable_spu_decr ();

  /* Clock is enabled on clock start - restore to original state (saved at start).  */
  if (__likely (!__spu_clock_state_was_enabled))
    {
      spu_idisable ();
    }

  return 0;
}
