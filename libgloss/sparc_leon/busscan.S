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


!unsigned int ahbslv_scan(register unsigned int vendor,register unsigned int driver) {
!  register unsigned int conf, mbar,i, *confp;
!  register unsigned int cfg_area = (unsigned int ) (LEON3_IO_AREA | LEON3_CONF_AREA | LEON3_AHB_SLAVE_CONF_AREA);
!  for (i = 0; i < LEON3_AHB_SLAVES; i++) 
!  {
!    confp = (unsigned int*)(cfg_area + (i * LEON3_AHB_CONF_WORDS * 4));
!    conf = *confp;
!    //mbar = *(unsigned int*)(i * LEON3_AHB_CONF_WORDS+ (4 * 4));
!    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == driver)) {
!      return (unsigned int)confp;
!    }
!  }
!  return 0;
!}

                 
	.section	".text"
	.global  ahbslv_scan	
	.align 4
	
ahbslv_scan:
	mov	%o0, %g1
	mov	-2048, %o5
	mov	0, %o3
	sll	%o3, 5, %o0
.LL11:
	add	%o5, %o0, %o4
	ld	[%o5+%o0], %o2
	srl	%o2, 24, %o0
	cmp	%o0, %g1
	bne,a	.LL10
	add	%o3, 1, %o3
	srl	%o2, 12, %o0
	and	%o0, 4095, %o0
	cmp	%o0, %o1
	be	.LL1
	mov	%o4, %o2
	add	%o3, 1, %o3
.LL10:
	cmp	%o3, 7
	bleu,a	.LL11
	sll	%o3, 5, %o0
	mov	0, %o2
.LL1:
	retl
	mov	%o2, %o0


!unsigned int apbslv_scan(register unsigned int base,register unsigned int vendor, register unsigned int driver) {
!  register unsigned int conf, mbar,i, *confp;
!  for (i = 0; i < LEON3_APB_SLAVES; i++) 
!  {
!    confp = (unsigned int*)(base + (i * LEON3_APB_CONF_WORDS * 4));
!    conf = *confp;
!    //mbar = *(unsigned int*)(i * LEON3_AHB_CONF_WORDS+ (4 * 4));
!    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == driver)) {
!      return (unsigned int)confp;
!    }
!  }
!  return 0;
!}


	.section	".text"
	.align 4
	.global  apbslv_scan
	
apbslv_scan:
	mov	%o0, %g1
	mov	0, %o4
	sll	%o4, 3, %o0
.LL22:
	add	%g1, %o0, %o5
	ld	[%g1+%o0], %o3
	srl	%o3, 24, %o0
	cmp	%o0, %o1
	bne,a	.LL21
	add	%o4, 1, %o4
	srl	%o3, 12, %o0
	and	%o0, 4095, %o0
	cmp	%o0, %o2
	be	.LL12
	mov	%o5, %o3
	add	%o4, 1, %o4
.LL21:
	cmp	%o4, 15
	bleu,a	.LL22
	sll	%o4, 3, %o0
	mov	0, %o3
.LL12:
	retl
	mov	%o3, %o0



!unsigned int getbase(register unsigned int *mbar,register unsigned int iobase) {
!  register unsigned int conf = mbar[1];
!  return ((iobase & 0xfff00000) |
!          ((conf & 0xfff00000)>> 12)) & (((conf & 0x0000fff0) <<4) | 0xfff00000);
!  
!}
        
         
	.section	".text"
	.align 4
	.global  iobar_getbase

iobar_getbase:
	ld	[%o0+4], %o2
	sethi	%hi(-1048576), %o3
	and	%o1, %o3, %o1
	and	%o2, %o3, %o0
	srl	%o0, 12, %o0
	or	%o1, %o0, %o1
	sethi	%hi(64512), %o0
	or	%o0, 1008, %o0
	and	%o2, %o0, %o2
	sll	%o2, 4, %o2
	or	%o2, %o3, %o2
	and	%o1, %o2, %o1
	retl
	mov	%o1, %o0
        
