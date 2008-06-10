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

#ifndef _SPU_TIMER_H_
#define _SPU_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Clock services.  */
extern void spu_clock_start (void);
extern int spu_clock_stop (void);
extern uint64_t spu_clock_read (void);

/* Timer services.  */
extern int spu_timer_alloc (int interval, void (*func) (int));
extern int spu_timer_free (int id);
extern int spu_timer_start (int id);
extern int spu_timer_stop (int id);

/* Interrupt services.  */
extern void spu_slih_register (unsigned event_mask,
			       unsigned (*slih) (unsigned));
extern unsigned spu_clock_slih (unsigned event_mask);

/* Number of supported timers.  */
#define SPU_TIMER_NTIMERS               4

/* Recommended minimun spu timer interval time from (cat /proc/cpuinfo)
    * QS20       100/14318000  = 6.98 usec
    * QS21/QS22  100/26666666  = 3.75 usec
    * PS3        100/79800000  = 1.25 usec  */
#define SPU_TIMER_MIN_INTERVAL          100

/* Clock error codes.  */
#define SPU_CLOCK_ERR_NOT_RUNNING       -2
#define SPU_CLOCK_ERR_STILL_RUNNING     -3
#define SPU_CLOCK_ERR_TIMERS_ACTIVE     -4

/* Timer error codes.  */
#define SPU_TIMER_ERR_INVALID_PARM      -10
#define SPU_TIMER_ERR_NONE_FREE         -11
#define SPU_TIMER_ERR_INVALID_ID        -12
#define SPU_TIMER_ERR_ACTIVE            -13
#define SPU_TIMER_ERR_NOT_ACTIVE        -14
#define SPU_TIMER_ERR_NOCLOCK           -15
#define SPU_TIMER_ERR_FREE              -16
#define SPU_TIMER_ERR_NOT_STOPPED       -17

#ifdef __cplusplus
}
#endif

#endif
