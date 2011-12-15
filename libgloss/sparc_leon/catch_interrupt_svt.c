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

/* single vector trapping version of trap handler installs */
struct svt_trap_entry
{
  int start, end, func;
};
extern struct svt_trap_entry trap_table[28];
extern struct svt_trap_entry svt_trap_table_ext[17];
extern struct svt_trap_entry svt_trap_table_ext_end;

static struct svt_trap_entry *
gettrap_pos (int nr)
{
  struct svt_trap_entry *p = trap_table;
  while ((p->start) || (p->end) || (p->func))
    {
      if (p->start <= nr && p->end >= nr)
	{
	  break;
	}
      p++;
    }
  return p;
}

int
svtloleveltrapinstall (int trap, void (*handler) ())
{
  struct svt_trap_entry *p = gettrap_pos (trap);
  if (p >= &svt_trap_table_ext_end)
    {
      return 0;
    }
  p->start = trap;
  p->end = trap;
  p->func = handler;
  return 1;
}

int
svtlolevelirqinstall (int irqnr, void (*handler) ())
{
  if (irqnr == 0 || irqnr >= 15)
    {
      return 0;
    }
  return svtloleveltrapinstall (irqnr + 0x10, handler);
}
