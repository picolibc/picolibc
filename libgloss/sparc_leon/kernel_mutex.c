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

leonbare_mutex_t
leonbare_mutex_init (leonbare_mutex_t m)
{
  m->mx_owner_cnt = 0;
  m->mx_owner = 0;
  LBTAILQ_INIT (&(m->mx_threads));

  LEONBARE_PROTECT_KERNEL_START ();
  {
    LBTAILQ_INSERT_TAIL (LEONBARE_KR_ALLM, m, mx_allm);
  }
  LEONBARE_PROTECT_KERNEL_END ();

}

int
_self__leonbare_mutex_lock (leonbare_mutex_t m, int wait)
{
  int ret = LEONBARE_MUTEX_LOCK_OK;
  leonbare_thread_t c;

  LEONBARE_PROTECT_MUTEXSTRUCT_START (m);
  while (1)
    {
      if (LEONBARE_MUTEX_OWNER_GET (m) == 0)
	{
	  LEONBARE_MUTEX_OWNER_SET (m, LEONBARE_KR_CURRENT);
	  LEONBARE_MUTEX_OWNER_CNT_SET (m, 0);
	  LBTAILQ_INSERT_TAIL (&c->th_mutex_locked, m, mx_locked);
	  ret = LEONBARE_MUTEX_LOCK_OK;
	  break;
	}
      else if (m->mx_owner == LEONBARE_KR_CURRENT)
	{
	  m->mx_owner_cnt++;
	  ret = LEONBARE_MUTEX_LOCK_OK;
	  break;
	}
      LBTAILQ_INSERT_TAIL (&m->mx_threads, c, th_mutex);
      current_suspend ();
    }
  LEONBARE_PROTECT_MUTEXSTRUCT_END (m);
  return ret;
}

int
leonbare_mutex_unlock (leonbare_mutex_t m)
{
  int ret = LEONBARE_MUTEX_UNLOCK_ERROR;
  leonbare_thread_t c, h;

  LEONBARE_PROTECT_MUTEXSTRUCT_START (m);
  {
    c = LEONBARE_KR_CURRENT;
    if (m->mx_owner != c)
      {
	ret = LEONBARE_MUTEX_UNLOCK_OK;
      }
    else if (m->mx_owner == c && m->mx_owner_cnt)
      {
	m->mx_owner_cnt--;
	ret = LEONBARE_MUTEX_UNLOCK_OK;
      }
    else if ((h = LBTAILQ_FIRST (&m->mx_threads)))
      {
	LBTAILQ_REMOVE (&m->mx_threads, h, th_mutex);
	leonbare_thread_resume (h);
	ret = LEONBARE_MUTEX_UNLOCK_OK;
      }
  }
  LEONBARE_PROTECT_MUTEXSTRUCT_END (m);
  return ret;
}
