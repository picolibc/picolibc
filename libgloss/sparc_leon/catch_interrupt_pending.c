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


#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
#define NULL 0

TAILQ_HEAD (pending_queue, pendingaction) pending =
TAILQ_HEAD_INITIALIZER (pending);

     void add_pending (struct pendingaction *a)
{
  unsigned long old = leonbare_disable_traps ();
  TAILQ_INSERT_TAIL (&pending, a, next);
  leonbare_enable_traps (old);
}

struct pendingaction *
get_pending ()
{
  struct pendingaction *a = 0;
  unsigned long old = leonbare_disable_traps ();
  if (a = TAILQ_FIRST (&pending))
    {
      TAILQ_REMOVE (&pending, a, next);
    }
  leonbare_enable_traps (old);
  return a;
}

void
process_pending ()
{
  struct pendingaction *a;
  while (a = get_pending ())
    {
      if (a->handler)
	{
	  a->handler (a->arg);
	}
    }
}
