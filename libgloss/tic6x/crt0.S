/* crt0.S for the TI C6X series of processors

   Copyright (c) 2010 CodeSourcery, Inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of CodeSourcery nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY CODESOURCERY, INC. ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CODESOURCERY BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
    DAMAGE.  */

	.text
	.align 	2

	.global	_start
_start:
	/* Start by setting up a stack */
	mvkl .s2	_STACK_START - 4, B15
	mvkh .s2	_STACK_START - 4, B15
	and .s2		-8, B15, B15

	mvkl .s2	__c6xabi_DSBT_BASE, B14
	mvkh .s2	__c6xabi_DSBT_BASE, B14

#ifdef __DSBT__
	stw .d2t2	B14, *B14
#endif
	/* Zero the memory in the .bss section.  */

	/* Set up GOT pointer.  */

	mvkl .s2	1f, B3
	mvkh .s2	1f, B3
	call .s2	_init
	nop		5
1:

	mvkl .s2	1f, B3
	mvkh .s2	1f, B3
	call .s2	main
	nop		5
1:

	b .s2		exit
	nop		5
