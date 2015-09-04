/* A setjmp.c for FT32
   Copyright (C) 2014 FTDI (support@ftdichip.com)

   The authors hereby grant permission to use, copy, modify, distribute,
   and license this software and its documentation for any purpose, provided
   that existing copyright notices are retained in all copies and that this
   notice is included verbatim in any distributions. No written agreement,
   license, or royalty fee is required for any of the authorized uses.
   Modifications to this software may be copyrighted by their authors
   and need not follow the licensing terms described here, provided that
   the new terms are clearly indicated on the first page of each file where
   they apply.  */

# setjmp/longjmp for FT32.
# Total jumpbuf size is 108 bytes, or 27 words.
#        

        .text
        .global        setjmp
        .type        setjmp,@function
setjmp:
        pop.l   $r5           # return address in $r5
        sti.l   $r0,0,$r5
        sti.l   $r0,4,$r6
        sti.l   $r0,8,$r7
        sti.l   $r0,12,$r8
        sti.l   $r0,16,$r9
        sti.l   $r0,20,$r10
        sti.l   $r0,24,$r11
        sti.l   $r0,28,$r12
        sti.l   $r0,32,$r13
        sti.l   $r0,36,$r14
        sti.l   $r0,40,$r15
        sti.l   $r0,44,$r16
        sti.l   $r0,48,$r17
        sti.l   $r0,52,$r18
        sti.l   $r0,56,$r19
        sti.l   $r0,60,$r20
        sti.l   $r0,64,$r21
        sti.l   $r0,68,$r22
        sti.l   $r0,72,$r23
        sti.l   $r0,76,$r24
        sti.l   $r0,80,$r25
        sti.l   $r0,84,$r26
        sti.l   $r0,88,$r27
        sti.l   $r0,92,$r28
        sti.l   $r0,96,$r29
        sti.l   $r0,100,$r30
        sti.l   $r0,104,$r31
        ldk.l   $r0,0
        jmpi    $r5

.Lend1:
        .size        setjmp,.Lend1-setjmp

        .global        longjmp
        .type        longjmp,@function
longjmp:
        cmp.l   $r1,0
        jmpc    nz,.nonz
        ldk.l   $r1,1
.nonz:
        ldi.l   $r5,$r0,0
        ldi.l   $r6,$r0,4
        ldi.l   $r7,$r0,8
        ldi.l   $r8,$r0,12
        ldi.l   $r9,$r0,16
        ldi.l   $r10,$r0,20
        ldi.l   $r11,$r0,24
        ldi.l   $r12,$r0,28
        ldi.l   $r13,$r0,32
        ldi.l   $r14,$r0,36
        ldi.l   $r15,$r0,40
        ldi.l   $r16,$r0,44
        ldi.l   $r17,$r0,48
        ldi.l   $r18,$r0,52
        ldi.l   $r19,$r0,56
        ldi.l   $r20,$r0,60
        ldi.l   $r21,$r0,64
        ldi.l   $r22,$r0,68
        ldi.l   $r23,$r0,72
        ldi.l   $r24,$r0,76
        ldi.l   $r25,$r0,80
        ldi.l   $r26,$r0,84
        ldi.l   $r27,$r0,88
        ldi.l   $r28,$r0,92
        ldi.l   $r29,$r0,96
        ldi.l   $r30,$r0,100
        ldi.l   $r31,$r0,104
        move.l  $r0,$r1
        jmpi    $r5

.Lend2:
        .size        longjmp,.Lend2-longjmp
