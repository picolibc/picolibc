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
#include <stdarg.h>

/*indent:
        indent -kr -i4 -ts4 -sob -l80 -ss -ncs -nut newlib-1.13.0/libgloss/sparc_leon/kernel*.c
	indent -kr -i4 -ts4 -sob -l80 -ss -ncs -nut *.h
	indent -kr -i4 -ts4 -sob -l80 -ss -ncs -nut *.c
*/

void leonbare_sched_printqueue ();

int
leonbare_sched_verify ()
{
#ifdef LBDEBUG_DO_ASSERT
  int i, j;
  leonbare_thread_t c, d;
  for (i = 0; i < LEONBARE_RUNQ_NR; i++)
    {
      LBTAILQ_FOREACH (c, LEONBARE_KR_RUNQ (i), th_runq)
      {
	if (i < LEONBARE_RUNQ_READY_NR)
	  {
	    LBPASSERT (c->th_runq_idx == i,
		       "thread %s has wrong runq[%d] index (%d) ",
		       LEONBARE_TH_NAME_DBG (c), i, c->th_runq_idx);
	    LBPASSERT (c->th_runq_which == LEONBARE_KR_RUNQ_WHICH,
		       "thread %s in runqueue[%d] has wrong th_runq_which(%d) (!=LEONBARE_KR_RUNQ_WHICH(%d))",
		       LEONBARE_TH_NAME_DBG (c), i, c->th_runq_which,
		       LEONBARE_KR_RUNQ_WHICH);
	  }
	else if (i == LEONBARE_RUNQ_SUSPENDED_IDX)
	  {
	    LBPASSERT (c->th_flags & LEONBARE_TH_SUSPENDED,
		       "thread %s in suspension queue has LEONBARE_TH_SUSPENDED not set ",
		       LEONBARE_TH_NAME_DBG (c));
	  }
	else if (i == LEONBARE_RUNQ_KILLED_IDX)
	  {
	    LBPASSERT (c->
		       th_flags & (LEONBARE_TH_TERMINATED |
				   LEONBARE_TH_FINISHED),
		       "thread %s in killed queue has (LEONBARE_TH_TERMINATED | LEONBARE_TH_FINISHED) not set ",
		       LEONBARE_TH_NAME_DBG (c));
	  }
	else if (i >= LEONBARE_RUNQ_PREPARE_IDX &&
		 i < (LEONBARE_RUNQ_PREPARE_IDX + LEONBARE_RUNQ_READY_NR))
	  {
	    LBPASSERT (c->th_runq_idx == (i - LEONBARE_RUNQ_PREPARE_IDX),
		       "thread %s has wrong prepare-runq[%d] index (%d) ",
		       LEONBARE_TH_NAME_DBG (c),
		       i - LEONBARE_RUNQ_PREPARE_IDX, c->th_runq_idx);
	    LBPASSERT (c->th_runq_which != LEONBARE_KR_RUNQ_WHICH,
		       "thread %s in prepare-runqueue[%d] has wrong th_runq_which(%d) (==LEONBARE_KR_RUNQ_WHICH(%d))",
		       LEONBARE_TH_NAME_DBG (c), i, c->th_runq_which,
		       LEONBARE_KR_RUNQ_WHICH);
	  }

	if (i != LEONBARE_RUNQ_KILLED_IDX)
	  {
	    LBPASSERT (!
		       (c->
			th_flags & (LEONBARE_TH_TERMINATED |
				    LEONBARE_TH_FINISHED)),
		       "thread %s not in killed queue has (LEONBARE_TH_TERMINATED | LEONBARE_TH_FINISHED) set ",
		       LEONBARE_TH_NAME_DBG (c));
	  }
	if (i != LEONBARE_RUNQ_SUSPENDED_IDX)
	  {
	    LBPASSERT (!(c->th_flags & (LEONBARE_TH_SUSPENDED)),
		       "thread %s not in suspend queue has LEONBARE_TH_SUSPENDED set ",
		       LEONBARE_TH_NAME_DBG (c));
	  }

	if (LBTAILQ_NEXT (c, th_runq))
	  {
	    LBPASSERT (c->th_account <=
		       LBTAILQ_NEXT (c, th_runq)->th_account,
		       "thread %s account is not sorted (%d<=%d)",
		       LEONBARE_TH_NAME_DBG (c), c->th_account,
		       LBTAILQ_NEXT (c, th_runq)->th_account);
	  }
      }
    }
  LBTAILQ_FOREACH (c, LEONBARE_KR_ALLQ, th_allq)
  {
    if ((j = c->th_runq_idx) != -1)
      {
	LBPASSERT (j >= 0
		   && j < LEONBARE_RUNQ_NR,
		   "thread %s has wrong runq index (%d) ",
		   LEONBARE_TH_NAME_DBG (c), c->th_runq_idx);
	LBTAILQ_FOREACH (d, LEONBARE_KR_RUNQ (j), th_runq)
	{
	  if (d == c)
	    {
	      break;
	    }
	}
	/*LBPASSERT(d,"thread %s is not in runq[%d] ",LEONBARE_TH_NAME_DBG(c),j); */
      }
  }
#endif
}

int
leonbare_debug_printf (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vprintf (fmt, ap);
  va_end (ap);
  return 0;
}

void
leonbare_sched_printqueue ()
{
  int i, j;
  leonbare_thread_t c;
  for (i = 0; i < LEONBARE_RUNQ_NR; i++)
    {
      LBDEBUG_HEADER_PRINTF (LBDEBUG_QUEUE_NR, "runq[%d]:[", i);
      LBTAILQ_FOREACH (c, LEONBARE_KR_RUNQ (i), th_runq)
      {
	LBDEBUG (LBDEBUG_QUEUE_NR, "%s[0x%x](%d),", LEONBARE_TH_NAME_DBG (c),
		 c, c->th_account);
      }
      LBDEBUG (LBDEBUG_QUEUE_NR, "]\n", 0);
    }
}
