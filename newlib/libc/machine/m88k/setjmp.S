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
/* This is a simple version of setjmp and longjmp.

   Ian Lance Taylor, Cygnus Support, 15 July 1993.  */

/* We need to save the address of the return instruction, which is in
   r1, as well as general register r14 through r25.  If we are
   compiling for the 88110 with the extended register file, we also
   need to save registers x22 through x29.  The jmp_buf should be 52
   bytes long in the one case, 84 bytes in the other.  */

/* int setjmp (jmp_buf);  */
	globl	_setjmp
_setjmp:
	st	r1,r2,0
	st.d	r14,r2,4
	st.d	r16,r2,12
	st.d	r18,r2,20
	st.d	r20,r2,28
	st.d	r22,r2,36
	st.d	r24,r2,44
#ifdef __m88110__
	/* These instructions are just a guess, and gas doesn't
	   support them anyhow.  */
	st.d	x22,r2,52
	st.d	x24,r2,60
	st.d	x26,r2,68
	st.d	x28,r2,76
#endif
	jmp	r1

	global	_longjmp
_longjmp:
	ld	r1,r2,0
	ld.d	r14,r2,4
	ld.d	r16,r2,12
	ld.d	r18,r2,20
	ld.d	r20,r2,28
	ld.d	r22,r2,36
	ld.d	r24,r2,44
#ifdef __m88110__
	/* These instructions are just a guess, and gas doesn't
	   support them anyhow.  */
	ld.d	x22,r2,52
	ld.d	x24,r2,60
	ld.d	x26,r2,68
	ld.d	x28,r2,76
#endif
	jmp	r1
