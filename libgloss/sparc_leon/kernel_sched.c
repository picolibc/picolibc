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

unsigned int
reschedule ()
{
  leonbare_sched_update ();
  return leonbare_sched ();
}

unsigned int
leonbare_sched ()
{
  unsigned int ret = 0;
  volatile leonbare_thread_t old = LEONBARE_KR_CURRENT, new =
    LEONBARE_KR_NEXT;
  LBDEBUG_FNCALL;
  LBDEBUG_HEADER_PRINTF (LBDEBUG_SCHED_NR, "switch %s[%x]->%s[%x]\n",
			 LEONBARE_TH_NAME_DBG (old), old,
			 LEONBARE_TH_NAME_DBG (new), new);

  LBPASSERT ((old != new),
	     "leonbare_sched should only be called with reschedule work to do",
	     0);

  LEONBARE_KR_CURRENT = new;

  /* to be able to programm symetrically on kernel level each thread
     saves it's spinlock on mutexes and kernel and irq flags in its
     own save region. On a kernel switch they are released until the
     thread is reawakened. Then the locks will be reaquired (and finally
     released when the codeblock exits). The locking can be recursive. */
  if (old->th_prot.krp_k_depth)
    {
      LEONBARE_SMP_SPINLOCK_RELEASE (LEONBARE_KR_LOCK);
    }
  if (old->th_prot.krp_m_depth)
    {
      LEONBARE_SMP_SPINLOCK_RELEASE (old->th_prot.krp_m);
    }

  ret = _leonbare_kernel_switchto (old, new);
  optbarrier ();

  if (new->th_prot.krp_m_depth)
    {
      LEONBARE_SMP_SPINLOCK_AQUIRE (new->th_prot.krp_m);
    }
  if (old->th_prot.krp_k_depth)
    {
      LEONBARE_SMP_SPINLOCK_AQUIRE (LEONBARE_KR_LOCK);
    }

  return ret;
}
