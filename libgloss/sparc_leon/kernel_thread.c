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


#include <asm-leon/queue.h>
/*#include <sys/fsu_pthread_queue.h>*/
#include <asm-leon/contextswitch.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/leonbare_debug.h>
#include <asm-leon/stack.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>

int
leonbare_thread_resume (leonbare_thread_t thread)
{
  leonbare_thread_t i = 0;
  if ((thread->th_flags & LEONBARE_TH_SUSPENDED) &&
      !(thread->th_flags & (LEONBARE_TH_TERMINATED | LEONBARE_TH_FINISHED)))
    {

      LEONBARE_PROTECT_KERNEL_START ();
      {
	unsigned int idx = leonbare_thread_getqueueidx (thread);

	if (idx != -1)
	  {
	    LBTAILQ_REMOVE (LEONBARE_KR_RUNQ (idx), thread, th_runq);
	  }

	if (thread->th_pri_idx != -1)
	  {
	    thread->th_runq_idx = thread->th_pri_idx;
	    thread->th_runq_which = LEONBARE_KR_RUNQ_WHICH;
	    LBTAILQ_INSERT_HEAD (LEONBARE_KR_RUNQ (thread->th_runq_idx),
				 thread, th_runq);
	    LEONBARE_TH_SETSTATE (thread, LEONBARE_TH_READY);
	  }
      }
      LEONBARE_PROTECT_KERNEL_END ();
    }

}

int
leonbare_thread_terminate (leonbare_thread_t thread)
{
  unsigned int ret = 0;
  leonbare_thread_t c = LEONBARE_KR_CURRENT;
  LEONBARE_PROTECT_KERNEL_START ();
  {
    unsigned int idx = leonbare_thread_getqueueidx (thread);

    if (LEONBARE_RUNQ_ISREADY (idx) || LEONBARE_RUNQ_ISPREPARE (idx) ||
	LEONBARE_RUNQ_ISSUSPEND (idx))
      {
	LBTAILQ_REMOVE (LEONBARE_KR_RUNQ (idx), thread, th_runq);
      }
    else
      {
	LBPASSERT (LEONBARE_RUNQ_ISKILLED (idx),
		   "thread %s is in no queue ",
		   LEONBARE_TH_NAME_DBG (thread));
      }
    LEONBARE_TH_SETSTATE (thread, LEONBARE_TH_TERMINATED);
    LBTAILQ_INSERT_HEAD (LEONBARE_KR_RUNQ (LEONBARE_RUNQ_KILLED_IDX), thread,
			 th_runq);

    LEONBARE_PRINTQUEUES ();

    LEONBARE_VERIFYSCHED ();
  }
  if (thread == LEONBARE_KR_CURRENT)
    {
      ret = reschedule ();
      /* never come here */
      LEONBARE_STOPALL;
    }
  LEONBARE_PROTECT_KERNEL_END ();
  return ret;
}


int
current_suspend ()
{
  unsigned int ret = 0;
  leonbare_thread_t c = LEONBARE_KR_CURRENT;
  LEONBARE_PROTECT_KERNEL_START ();
  {
    LBPASSERT ((c->th_runq_which == LEONBARE_KR_RUNQ_WHICH),
	       "Current thread cannot be on the prepare queue", 0);

    LBTAILQ_REMOVE (LEONBARE_KR_RUNQ (c->th_runq_idx), c, th_runq);
    LBTAILQ_INSERT_TAIL (LEONBARE_KR_RUNQ (LEONBARE_RUNQ_SUSPENDED_IDX),
			 c, th_runq);
    c->th_runq_idx = LEONBARE_RUNQ_SUSPENDED_IDX;
    LEONBARE_TH_SETSTATE (c, LEONBARE_TH_SUSPENDED);
    LEONBARE_VERIFYSCHED ();
  }
  ret = reschedule ();
  LEONBARE_PROTECT_KERNEL_END ();
  return ret;
}

void
_leonbare_thread_body ()
{
  LBDEBUG_FNCALL;
  leonbare_thread_t thread = LEONBARE_KR_CURRENT;

  LEONBARE_KR_IS_IN_KERNEL = 0;
  thread->th_result = thread->th_func (thread->th_arg);

  leonbare_thread_terminate (thread);

  LBDEBUG_FNEXIT;
}

#define LEONBARE_BODY_OFFSET 200

int
leonbare_thread_create (struct leonbare_thread *thread, char *stack,
			int stacksize)
{
  LEONBARE_PROTECT_DECL (flags);
  struct sparc_stackframe_regs *sp;
  unsigned int v;
  unsigned int fpspill, bodysp, bodyfp;
  struct leonbare_thread_ctx *threadctx;
  LBDEBUG_FNCALL;

  thread->th_stack_base = (char *) LEONBARE_STACKALIGN (stack);
  stacksize -= thread->th_stack_base - stack;
  thread->th_stack_size = stacksize;
  thread->th_runq_idx = 0;
  thread->th_pri_idx = 0;
  thread->th_account = 0;
  thread->th_caccount = 0;

  /* stack:
   * 0:+--------------------------------+ <- thread.th_stack_base
   *   |   ....                         |
   *   +--------------------------------+ <- thread.th_ctx->out[6] (%sp)
   *   |  _leonbare_thread_body() frame |
   *   +--------------------------------+ <- thread.th_ctx->sf_ins[6]  (%fp)
   *   |           WINDOWSPILL          |
   *   +--------------------------------+ <- thread.th_ctx->fpu  
   *   |  struct sparc_fpuwindow_regs   |
   *   +--------------------------------+ <- thread.th_stack_base + thread->th_stack_size
   *
   */
  v = (unsigned int) (thread->th_stack_base +
		      LEONBARE_STACKALIGN (thread->th_stack_size -
					   (LEONBARE_BODY_OFFSET +
					    WINDOWSIZE + FW_REGS_SZ)));

  bodysp = ((unsigned int) v);
  bodyfp = ((unsigned int) bodysp) + LEONBARE_BODY_OFFSET;
  fpspill = ((unsigned int) bodyfp) + WINDOWSIZE;

  thread->th_ctx.outs[6] = (unsigned int) bodysp;
  thread->th_ctx.outs[7] = (int) _leonbare_thread_body;
  thread->th_ctx.outs[7] -= 8;
  thread->th_ctx.sf_ins[6] = (unsigned int) bodyfp;
  thread->th_ctx.fpu = (unsigned int) fpspill;
  thread->th_ctx.magic = LEONBARE_THREAD_CTX_MAGIC;

  thread->th_ctx.psr = 0x0e0;
  thread->th_ctx.wim = 0x2;

  LBDEBUG_HEADER_PRINTF (LBDEBUG_THREAD_NR,
			 "Thread %s (0x%x): stack [0x%x-0x%x] \n",
			 LEONBARE_TH_NAME_DBG (thread), thread,
			 thread->th_stack_base,
			 thread->th_stack_base + thread->th_stack_size);

  /* newlibc reent */
  thread->th_reentp = &(thread->th_reent);
  _REENT_INIT_PTR (thread->th_reentp);

  LEONBARE_PROTECT_KERNEL_START ();
  /* queues */
  LBTAILQ_INSERT_TAIL (LEONBARE_KR_ALLQ, thread, th_allq);
  LBTAILQ_INSERT_TAIL (LEONBARE_KR_RUNQ (thread->th_runq_idx), thread,
		       th_runq);

  LEONBARE_PRINTQUEUES ();

  LEONBARE_PROTECT_KERNEL_END ();

  LBDEBUG_FNEXIT;
}
