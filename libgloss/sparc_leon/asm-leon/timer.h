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


#ifndef _ASMSPARC_TIMER_H
#define _ASMSPARC_TIMER_H

#include <asm-leon/queue.h>
/*#include <sys/fsu_pthread_queue.h>*/
#include <sys/time.h>
#include <asm-leon/clock.h>

#ifndef __ASSEMBLER__
typedef int (*timerevent_handler) (void *);
struct timerevent
{
  TAILQ_ENTRY (timerevent) n;
  struct timespec expire;
  timerevent_handler handler;
  void *arg;

};
#endif

#define GT_TIMESPEC(t1, t2) \
      (t1.tv_sec > t2.tv_sec || \
       (t1.tv_sec == t2.tv_sec && \
	t1.tv_nsec > t2.tv_nsec))

#define GT_TIMEVAL(t1, t2) \
      (t1.tv_sec > t2.tv_sec || \
       (t1.tv_sec == t2.tv_sec && \
	t1.tv_usec > t2.tv_usec))

/*
 * MINUS_TIME only works if src1 > src2
 */
#define MINUS_TIMEVAL(dst, src1, src2) \
    if ((src2).tv_usec > (src1).tv_usec) { \
      (dst).tv_sec = (src1).tv_sec - (src2).tv_sec - 1; \
      (dst).tv_usec = ((src1).tv_usec - (src2).tv_usec) + USEC_PER_SEC; \
    } \
    else { \
      (dst).tv_sec = (src1).tv_sec - (src2).tv_sec; \
      (dst).tv_usec = (src1).tv_usec - (src2).tv_usec; \
    }

/* Protypes */
#ifndef __ASSEMBLER__
void leonbare_init_ticks ();
int addtimer (struct timerevent *e);
#endif

#endif
