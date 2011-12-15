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
struct irqaction *_irqtbl[32] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0 };
struct irqaction _oirqtbl[32] =
  { INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION,
  INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION, INIT_IRQACTION
};

int
catch_interrupt (int func, int irq)
{
  struct irqaction *a = _irqtbl[irq];
  struct irqaction *n = &_oirqtbl[irq];
  if (irq >= 32)
    return 0;

  while (a)
    {
      if (a == n)
	{
	  int tmp = (int) a->handler;
	  a->handler = (irqhandler) func;
	  return tmp;
	}
      a = a->next;
    }
  n->handler = (irqhandler) func;
  chained_catch_interrupt (irq, n);
  return 0;
}

void
chained_catch_interrupt (int irq, struct irqaction *a)
{
  a->next = _irqtbl[irq];
  _irqtbl[irq] = a;
}

int no_inirq_check = 0;
int inirq[32] = { 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};
extern struct irqmp_type irqmp;

void (*handler_irq_pre) (void) = 0;
void (*handler_irq_post) (void) = 0;
handler_irq (int irq, struct leonbare_pt_regs *pt_regs)
{
  struct irqaction *a;

  if (irq == irqmp.eirq)
    irq = irqmp.addr[48] & 0x1f;
  if (!irq)
    irq = irqmp.eirq;

  a = _irqtbl[irq];

  while (a)
    {
      if (a->handler)
	{
#ifndef CONFIG_LEONBARE_NONESTEDIRQ
	  if (no_inirq_check || !(inirq[irq]))
	    {
#endif
	      inirq[irq]++;
	      if (handler_irq_pre)
		handler_irq_pre ();
	      a->handler (irq, a->dev_id, pt_regs);
	      if (handler_irq_post)
		handler_irq_post ();
	      inirq[irq]--;
#ifndef CONFIG_LEONBARE_NONESTEDIRQ
	    }
#endif
	}
      a = a->next;
    }
}

schedulehandler schedule_callback = 0;
