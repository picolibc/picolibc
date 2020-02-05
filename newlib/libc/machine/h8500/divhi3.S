/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
	.title "H8/500 DIVIDE"
!! 2 byte integer Divide code for the H8/500
!!
!! Steve Chamberlain
!! sac@cygnus.com
!!
!!

!! args in r1 and r4, result in r0/r1

#if __CODE__==32
#define RET prts
#else
#define RET rts
#endif
	.global	__divmodhi4

		
__divmodhi4:
	clr.w	r0	
	tst.w	r1	! neg arg1
	bpl	PU	


	neg.w   r1	
NU:	tst.w	r4	
	bmi	NN

NP:	divxu.w	r4,r0
	neg.w	r0
	neg.w	r1
	RET

NN:	neg.w	r4
	divxu.w	r4,r0
	neg.w	r0	! get rem sign right
	RET


PU:
	tst.w	r4
	bpl	PP
	

PN:	neg.w	r4
	divxu.w	r4,r0
	neg.w	r1
	RET

PP:	divxu.w	r4,r0	! rem in r0, q in r1
	RET

	
	
