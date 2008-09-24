/*

Copyright (c) 2008 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

/* This is a sample program that shows how to use a few of the
   features of the M32C port of GCC, Binutils, and Newlib.  */

#include <varvects.h>

typedef unsigned char byte;
typedef unsigned short word;

#define prcr	(*(volatile byte *)0x000a)
#define cm0	(*(volatile byte *)0x0006)
#define cm1	(*(volatile byte *)0x0007)
#define ocd	(*(volatile byte *)0x000c)

#ifdef __r8c_cpu__
/* These are for the R8C/20 with LEDs on port P2 */

#define tracr	(*(volatile byte *)0x0100)
#define traioc	(*(volatile byte *)0x0101)
#define tramr	(*(volatile byte *)0x0102)
#define trapre	(*(volatile byte *)0x0103)
#define tra	(*(volatile byte *)0x0104)
#define traic	(*(volatile byte *)0x0056)

#define pd2	(*(volatile byte *)0x00e6)
#define p2	(*(volatile byte *)0x00e4)

#define ivec_timer_ra 22
#endif

#ifdef __m32c_cpu__
/* These are for the M32C/83 with LEDs on port P0 and P1 */

#define ta0	(*(volatile word *)0x0346)
#define ta0mr	(*(volatile byte *)0x0356)
#define tabsr	(*(volatile byte *)0x0340)
#define ta0ic	(*(volatile byte *)0x006c)

#define pd0	(*(volatile byte *)0x03e2)
#define pd1	(*(volatile byte *)0x03e3)
#define p0	(*(volatile byte *)0x03e0)
#define p1	(*(volatile byte *)0x03e1)

#define ivec_timer_a0 12
#endif

/* Newlib's exit() pulls in lots of other things.  Main() should never
   exit, but if it did, you could hard-reset the chip here.  */
void
exit(int rv)
{
  while (1)
    asm volatile ("");
}

#ifdef __r8c_cpu__
/* The "constructor" attribute causes the startup code to call this
   sometime before main() is called.  */
__attribute__((constructor))
void
fast_clock(void)
{
  asm("fclr I");
  prcr = 1;
  cm0 = 0x08;
  cm1 = 0x38;
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  ocd = 0;
  prcr = 0;
  asm("fset I");
}
#endif

/* We mark this volatile in case a non-interrupt function wants to
   read it, else gcc may optimize away extra reads.  */
static volatile int tc = 1;

/* The "interrupt" attribute changes the function entry/exit to
   properly preserve any changed registers.  */
static void __attribute__((interrupt))
timer_ra_interrupt()
{
  tc ++;
#ifdef __r8c_cpu__
  p2 = tc >> 4;
#else
  p1 = tc;
  p0 = tc >> 8;
#endif
}

main()
{
#ifdef __r8c_cpu__
  pd2 = 0xff;

  /* TIMER RA */
  tracr = 0x00;
  traioc = 0x00;
  tramr = 0x00; /* timer mode, f1 */
  trapre = 255; /* prescaler */
  tra = 255; /* cycle count */

  _set_var_vect (timer_ra_interrupt, ivec_timer_ra); 
  traic = 5;
  tracr = 1;
#endif

#ifdef __m32c_cpu__
  pd0 = 0xff;
  pd1 = 0xff;

  /* TIMER A0 */ 
  ta0mr = 0x00;			/* Timer A0 mode register */
  ta0 = 65535;			/* Timer A0 register */

  _set_var_vect (timer_ra_interrupt, ivec_timer_a0); 
  ta0ic = 5;
  tabsr = 0xff;
#endif

  /* main() must never return.  */
  while (1)
    ;
}
