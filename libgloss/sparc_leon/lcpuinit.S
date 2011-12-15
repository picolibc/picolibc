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


#define LEON3_IO_AREA 0xfff00000
#define LEON3_CONF_AREA 0xff000
#define VENDOR_GAISLER 0x01
#define GAISLER_APBMST 0x006
#define GAISLER_APBUART 0x00C
#define GAISLER_GPTIMER 0x011
   
	.text
	/* ------- */
  	.weak	cpuinit
	.set	cpuinit,_cpuinit
	/* ------- */
	!.global cpuinit
_cpuinit:

        mov %o7,%g6

	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 0xf, %g5
	subcc	%g5, 3, %0
	be	1f

	/* other versions */
	
        ba 2f  	

        ! ####### leon3 #########

1:      set 0x01, %o0              !VENDOR_GAISLER
        set 0x006, %o1             !GAISLER_APBMST  
        call ahbslv_scan
        nop

        cmp %g0,%o0 
        be 2f
         nop

        ld [%o0+16],%g1
        set 0xfff00000,%o0         !LEON3_IO_AREA          
        and %g1,%o0,%g1            !g1: apb base

        set 0xff000,%o0            !LEON3_CONF_AREA    
        or %g1,%o0,%g2             !g2: apb conf base
  
        ! ####### uart #########
        mov %g2,%o0
        set 0x01 , %o1             ! VENDOR_GAISLER   
        set 0x00C,%o2              ! GAISLER_APBUART  
        call apbslv_scan
         nop
        cmp %g0,%o0
        be 2f
         nop
        
        call iobar_getbase
         mov %g1,%o1
        
	set 	console, %g5
        st %o1,[%g5]             !uart base address

        ! ####### timer #########

        mov %g2,%o0
        set 0x01 , %o1           !VENDOR_GAISLER
        set 0x011,%o2            !GAISLER_GPTIMER
        call apbslv_scan
         nop
        cmp %g0,%o0
        be 2f
         nop
        
        call iobar_getbase
         mov %g1,%o1
        
        add %o1,0x10,%o1
	set rtc, %g5
        st %o1,[%g5]

        ! ################
        
        mov %g2,%o0
        set 0x01 , %o1           !VENDOR_GAISLER
        set 0x00D,%o2            !GAISLER_IRQMP
        call apbslv_scan
        nop
        cmp %g0,%o0
        be 2f
        nop
        
        call iobar_getbase
        mov %g1,%o1
        
	set irqmp, %g5
        st %o1,[%g5]
        ld [%o1+0x10], %o2
	srl %o2, 16, %o2
	and %o2, 15, %o2
        st %o2,[%g5+4]

        ! ################

2:	mov %g6,%o7
        retl
	nop

        ! force link of jiffies_64
        .global jiffies_64
3:      set jiffies_64,%g1


	.data
	.global irqmp

irqmp:	.word 0		! IRQMP base address
	.word 0		! extended irq number

	.text
