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


#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>

extern volatile unsigned int __window_overflow_rettseq[3];
extern volatile unsigned int __window_overflow_slow1[2];

static unsigned int installed = 0;
static unsigned int save__window_overflow_rettseq[3];

int
install_winoverflow_hook (void (*func) (void))
{
  if (installed)
    {
      return 0;
    }
  if (!installed)
    {
      /*
         a7 50 00 00  rd  %wim, %l3
         ae 10 00 01  mov  %g1, %l7
         83 34 e0 01  srl  %l3, 1, %g1

         81 c4 40 00  jmp  %l1
         81 cc 80 00  rett  %l2 */
      save__window_overflow_rettseq[0] = __window_overflow_rettseq[0];
      save__window_overflow_rettseq[1] = __window_overflow_rettseq[1];
      save__window_overflow_rettseq[2] = __window_overflow_rettseq[2];

      /*29 10 00 31   sethi  %hi(0x4000c400), %l4
         81 c5 22 48  jmp  %l4 + 0x248
         01 00 00 00  nop  */

      __window_overflow_rettseq[0] = ((((unsigned int) func) >> 10) & 0x3fffff) | 0x29000000;	/* upper 22 */
      __window_overflow_rettseq[1] = ((((unsigned int) func)) & 0x03ff) | 0x81c52000;	/* lower 10 */
      __window_overflow_rettseq[2] = 0x01000000;	/* nop */

      sparc_leon23_icache_flush ();
      installed = 1;
    }
  return 0;
}

void
uninstall_winoverflow_hook ()
{
  if (installed)
    {
      __window_overflow_rettseq[0] = save__window_overflow_rettseq[0];
      __window_overflow_rettseq[1] = save__window_overflow_rettseq[1];
      __window_overflow_rettseq[2] = save__window_overflow_rettseq[2];

      sparc_leon23_icache_flush ();
      installed = 0;
    }
}
