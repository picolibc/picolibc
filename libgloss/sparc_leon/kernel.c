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


#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/stack.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>

struct leonbare_kernel leonbare_kernel;

/*
 * queue 0: [ <acc=2>],
 * queue 1: [ <acc=10>, <acc=8>,<acc=8>,<acc=1> ],
 * queue 2: [ ... ],
 * ...
 * queue n: [ ... ]
 *
 *  Seach through ready queue [0 - LEONBARE_RUNQ_READY_NR-1] for the
 *  first thread in a queue'ss head to discover
 *  leonbare_thread_tick_callback() will put threads that have their th_caccount
 *  consumed into the prepare-run queues. th_caccount is already initialized
 *  to the value for the next schedule round. So all there is to do is to
 *  move the to prepare-run queues to run queues [0 - LEONBARE_RUNQ_READY_NR-1].
 *  return the first thread in any queue, similary to leonbare_sched_next().
 *  Using LEONBARE_KR_RUNQ_WHICH and leonbare_thread.th_runq_which one can
 *  determine weather the thread is in a runqueue or a prepare-runqueue:
 *  LEONBARE_KR_RUNQ_WHICH == leonbare_thread.th_runq_which : thread in runqueue
 *  LEONBARE_KR_RUNQ_WHICH != leonbare_thread.th_runq_which : thread in prepare-runqueue
 *  after a leonbare_thread_tick_callback() or a run queue change, call
 *  leonbare_sched_update() to update LEONBARE_KR_NEXT and
 *  LEONBARE_KR_NEED_SCHEDULE
 */
int
leonbare_sched_update ()
{
  int idx;
  leonbare_thread_t n = 0;
  int i = 0;
  LEONBARE_VERIFYIRQDISABLED ();
  LEONBARE_VERIFYSCHED ();
  for (i = 0; i < LEONBARE_RUNQ_READY_NR; i++)
    {
      leonbare_thread_t c;
      if (!LBTAILQ_EMPTY (LEONBARE_KR_RUNQ (i)))
	{
	  n = LBTAILQ_FIRST (LEONBARE_KR_RUNQ (i));
	  break;
	}
    }
  if (!n)
    {
      for (idx = 0; idx < LEONBARE_RUNQ_READY_NR; idx++)
	{
	  struct leonbare_thread_queue *h0 =
	    (struct leonbare_thread_queue *) LEONBARE_KR_RUNQ (idx);
	  struct leonbare_thread_queue *h1 = (struct leonbare_thread_queue *)
	    LEONBARE_KR_RUNQ (idx + LEONBARE_RUNQ_PREPARE_IDX);
	  if (LBTAILQ_EMPTY (h1))
	    {
	      LBTAILQ_INIT (h0);
	    }
	  else
	    {
	      *h0 = *h1;
	      if (LBTAILQ_FIRST (h0))
		{
		  LBTAILQ_FIRST (h0)->th_runq.tqe_prev = &(h0)->tqh_first;
		}
	      if (!n)
		{
		  n = LBTAILQ_FIRST (h0);
		}
	    }
	}
      for (idx = 0; idx < LEONBARE_RUNQ_READY_NR; idx++)
	{
	  LBTAILQ_INIT (LEONBARE_KR_RUNQ (idx + LEONBARE_RUNQ_PREPARE_IDX));
	}
      LEONBARE_KR_RUNQ_WHICH++;
      LEONBARE_VERIFYSCHED ();
      LEONBARE_PRINTQUEUES ();
    }
  LEONBARE_KR_NEXT = n;
  return (LEONBARE_KR_NEED_SCHEDULE);
}

/*  called in the timer irq handling. Decrements the 
 *  th_caccount field. On consumption of th_caccount the 
 *  thread will be removed from the ready queue nad placed into the
 *  prepare-runqueue for later readdition by leonbare_sched_readyprepare()
 *  called from gettimeofday.c's installed ticker_callback callback
 *  leonbare_thread_tick_callback() might change the kernel state in which case
 *  state on return from leonbare_thread_tick_callback() leonbare_thread_schedule_callback()
 *  will be called from rtrap_fast.S .
 */
int
leonbare_thread_tick_callback ()
{
  LEONBARE_PROTECT_DECL (flags);
  unsigned int r;
  volatile leonbare_thread_t c = LEONBARE_KR_CURRENT;
  leonbare_thread_t i;
  LBDEBUG_FNCALL;
  if (c && LEONBARE_KR_IS_PREEMPTION)
    {

      LEONBARE_PROTECT_KERNEL_START ();
      {

	LEONBARE_VERIFYIRQDISABLED ();
	LEONBARE_VERIFYSCHED ();

	if ((--c->th_caccount) < 0)
	  {
	    LBDEBUG_HEADER_PRINTF (LBDEBUG_QUEUE_NR, "remove %s(%x)\n",
				   LEONBARE_TH_NAME_DBG (c), c);
	    LBTAILQ_REMOVE (LEONBARE_KR_RUNQ (c->th_runq_idx), c, th_runq);
	    LBTAILQ_INSERT_TAIL (LEONBARE_KR_RUNQ
				 (c->th_runq_idx + LEONBARE_RUNQ_PREPARE_IDX),
				 c, th_runq);
	    c->th_caccount = c->th_account;
	    c->th_runq_which++;
	  }
	else
	  {
	    /* todo: round robbin scheme */
	  }
      }
      LEONBARE_PROTECT_KERNEL_END ();
    }
  r = leonbare_sched_update ();
  return r;
}

/* called from rtrap_fast.S's installed schedule_callback callback */
int
leonbare_thread_schedule_callback (struct leonbare_pt_regs *ptregs)
{
  unsigned int retval;
  LBDEBUG_FNCALL;
  if (LEONBARE_KR_IS_IN_KERNEL == 0 && LEONBARE_KR_NEED_SCHEDULE)
    {

      leonbare_sched ();

      //KERNEL_ENTER;
      //KERNEL_EXIT(LEONBARE_KR_NEED_SCHEDULE, retval);
    }
  LBDEBUG_FNEXIT;
}


struct leonbare_thread _thread_main;
int
leonbare_thread_init ()
{
  int i;
  LBDEBUG_FNCALL;

  memset ((void *) &_thread_main, 0, sizeof (_thread_main));
  _thread_main.th_reentp = _impure_ptr;
  _thread_main.th_name = "<main>";
  _thread_main.th_runq_idx = 0;
  _thread_main.th_pri_idx = 0;
  _thread_main.th_account = 0;

  LBTAILQ_INIT (LEONBARE_KR_ALLQ);
  for (i = 0; i < LEONBARE_RUNQ_NR; i++)
    {
      LBTAILQ_INIT (LEONBARE_KR_RUNQ (i));
    }
  LBTAILQ_INIT (LEONBARE_KR_ALLM);

  /* queues */
  LBTAILQ_INSERT_TAIL (LEONBARE_KR_ALLQ, &_thread_main, th_allq);

  /* inseart into ready queue 0 at head */
  LBTAILQ_INSERT_HEAD (LEONBARE_KR_RUNQ (_thread_main.th_runq_idx),
		       &_thread_main, th_runq);

  LEONBARE_KR_CURRENT = &_thread_main;
  LEONBARE_KR_IS_IN_KERNEL = 0;

  leonbare_init_ticks ();
  schedule_callback = (schedulehandler) leonbare_thread_schedule_callback;
  ticker_callback = (tickerhandler) leonbare_thread_tick_callback;

  /* disable later */
  LEONBARE_KR_IS_PREEMPTION = 1;


  LBDEBUG_FNEXIT;
}
