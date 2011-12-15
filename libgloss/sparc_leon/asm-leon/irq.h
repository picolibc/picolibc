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


#ifndef _LEON_CATCHIRQ_HANDLER_H_
#define _LEON_CATCHIRQ_HANDLER_H_

#include <asm-leon/leon.h>
#include <asm-leon/queue.h>
/*#include <sys/fsu_pthread_queue.h>*/
#include <asm-leon/leoncompat.h>
#include <asm-leon/leonstack.h>

#ifndef __ASSEMBLER__

struct pt_regs;
typedef int (*irqhandler) (int, void *, struct leonbare_pt_regs *);

struct irqaction
{
  irqhandler handler;
  unsigned long flags;
  void *dev_id;
  struct irqaction *next;
};
#define INIT_IRQACTION { 0,0,0,0 }

struct irqmp_type
{
  int *addr;
  int eirq;
};

extern void chained_catch_interrupt (int irq, struct irqaction *a);
extern int catch_interrupt (int func, int irq);

typedef int (*schedulehandler) (struct leonbare_pt_regs *);
extern schedulehandler schedule_callback;
typedef int (*tickerhandler) (struct leonbare_pt_regs *);
extern tickerhandler ticker_callback;
extern int leonbare_hz;
extern int nestcount;
extern int no_inirq_check;
extern unsigned long force_noalarm;

extern void (*handler_irq_pre) (void);
extern void (*handler_irq_post) (void);

extern void leonbare_enable_traps (unsigned long old_flags);
extern unsigned long leonbare_disable_traps ();
extern void leonbare_flush_windows ();

static inline void
leonbare_enable_irq (int irq)
{
  unsigned int old, irqmask = 1 << irq;
  old = leonbare_disable_traps ();
  //---------------------
  switch (LEONCOMPAT_VERSION)
    {
    case 3:
    default:
      LEON3_IrqCtrl_Regs->mask[0] = LEON3_IrqCtrl_Regs->mask[0] | irqmask;
      break;
    }
  //---------------------
  leonbare_enable_traps (old);
}

typedef int (*pendinghandler) (void *);
struct pendingaction
{
  TAILQ_ENTRY (pendingaction) next;
  pendinghandler handler;
  void *arg;
};

#endif

#endif
