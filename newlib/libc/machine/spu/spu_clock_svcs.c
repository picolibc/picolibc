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

/* SPU clock start and read library services.  */
#include <spu_timer.h>
#include "spu_timer_internal.h"

/* The software managed timebase value.  */
volatile uint64_t __spu_tb_val __attribute__ ((aligned (16)));

/* Timeout value of the current interval.  */
volatile int __spu_tb_timeout __attribute__ ((aligned (16)));

/* Clock start count (clock is running if >0).  */
volatile unsigned __spu_clock_startcnt __attribute__ ((aligned (16)));

/* Saved interrupt state from clock_start.  */
volatile unsigned __spu_clock_state_was_enabled;

/* Initializes the software managed timebase, enables the decrementer event,
   starts the decrementer and enables interrupts. Must be called before
   clock or timer services can be used. Should only be called by base app/lib
   code (not from an interrupt/timer handler).
   Returns with interrupts ENABLED.  */
void
spu_clock_start (void)
{
  /* Increment clock start and return if it was already running.  */
  if (++__spu_clock_startcnt > 1)
    return;

  __spu_clock_state_was_enabled = spu_readch (SPU_RdMachStat) & 0x1;

  spu_idisable ();
  __spu_tb_timeout = CLOCK_START_VALUE;
  __spu_tb_val = 0;

  /* Disable, write, enable the decrementer.  */
  __enable_spu_decr (__spu_tb_timeout, __disable_spu_decr ());

  spu_ienable ();

  return;
}

/* Returns a monotonically increasing, 64-bit counter, in timebase units,
   relative to the last call to spu_clock_start().  */
uint64_t
spu_clock_read (void)
{
  int64_t time;
  unsigned was_enabled;

  /* Return 0 if clock is off.  */
  if (__spu_clock_startcnt == 0)
    return 0LL;

  was_enabled = spu_readch (SPU_RdMachStat) & 0x1;
  spu_idisable ();

  time = __spu_tb_val + (__spu_tb_timeout - spu_readch (SPU_RdDec));

  if (__likely (was_enabled))
    spu_ienable ();
  return time;
}
