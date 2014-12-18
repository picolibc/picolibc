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

/* Internal definitions for SPU timer library.  */
#ifndef _SPU_TIMER_INTERNAL_H_
#define _SPU_TIMER_INTERNAL_H_

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <limits.h>
#include <stdlib.h>

#ifdef SPU_TIMER_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

/* The timer state tells which list its on.  */
typedef enum spu_timer_state
{
  SPU_TIMER_FREE = 0,
  SPU_TIMER_ACTIVE = 1,
  SPU_TIMER_HANDLED = 2,
  SPU_TIMER_STOPPED = 3
} spu_timer_state_t;

typedef struct spu_timer
{
  int tmout __attribute__ ((aligned (16)));	/* Time until expiration (tb).  */
  int intvl __attribute__ ((aligned (16)));	/* Interval.  */
  int id __attribute__ ((aligned (16)));
  spu_timer_state_t state __attribute__ ((aligned (16)));
  void (*func) (int) __attribute__ ((aligned (16)));	/* Handler.  */
  struct spu_timer *next __attribute__ ((aligned (16)));
} spu_timer_t;


/* Max decrementer value.  */
#define DECR_MAX        0xFFFFFFFFU

 /* Arbitrary non-triggering value.  */
#define CLOCK_START_VALUE 0x7FFFFFFF

#define MIN_INTVL       1
#define MAX_INTVL       INT_MAX

/* Timers within 15 tics will expire together.  */
#define TIMER_INTERVAL_WINDOW  15

/* Disables the decrementer and returns the saved event mask for a subsequent
   call to __enable_spu_decr. The decrementer interrupt is acknowledged in the
   flih when the event is received, but is required also as part of the
   procedure to stop the decrementer.  */
static inline unsigned
__disable_spu_decr (void)
{
  unsigned mask = spu_readch (SPU_RdEventMask);
  spu_writech (SPU_WrEventMask, mask & ~MFC_DECREMENTER_EVENT);
  spu_writech (SPU_WrEventAck, MFC_DECREMENTER_EVENT);
  spu_sync_c ();
  return mask;
}

/* Writes and enables the decrementer, along with the given event mask.  */
static inline void
__enable_spu_decr (int val, unsigned mask)
{
  spu_writech (SPU_WrDec, (val));
  spu_writech (SPU_WrEventMask, mask | MFC_DECREMENTER_EVENT);
  spu_sync_c ();
}

/* These are shared between modules but are not inlined, to save space.  */
extern void __spu_timer_start (int id, int reset);
extern void __reset_spu_decr (int val);

/* The timers.  */
extern spu_timer_t __spu_timers[];

/* Active timer list.  */
extern spu_timer_t *__spu_timers_active;

/* Stopped (allocated) timer list.  */
extern spu_timer_t *__spu_timers_stopped;

/* List of timers being handled.  */
extern spu_timer_t *__spu_timers_handled;

/* Bitmask of available timers.  */
extern unsigned __spu_timers_avail;

/* The software managed timebase value.  */
extern volatile uint64_t __spu_tb_val;

/* Timeout value of the current interval.  */
extern volatile int __spu_tb_timeout;

/* Clock start count (clock is running if >0).  */
extern volatile unsigned __spu_clock_startcnt;

/* Saved interrupt state from clock_start.  */
extern volatile unsigned __spu_clock_state_was_enabled;

#define __likely(_c)        __builtin_expect((_c), 1)
#define __unlikely(_c)      __builtin_expect((_c), 0)

#define ABORT() \
{\
    fprintf(stderr, "Internal error, aborting: %s:%d\n", __FILE__, __LINE__);\
    assert(0);\
}

#endif
