/*
Copyright (c) 2013-2014 Andes Technology Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

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
	.text
	.align	1
	.global  sqrt
	.type    sqrt, @function
sqrt:
	/* The input argument is supposed to be stored in $fd0.
	   The return value is supposed to be stored in $fd0 either.  */

	/* Clear the IEEE cumulative exceptions flags. ($fpcsr.b[6:2])  */
	FMFCSR  $r0
	bitci   $r0, $r0, #0b1111100
	FMTCSR	$r0

	fsqrtd   $fd0, $fd0

	/* Check the IEEE cumulative exceptions flags.  */
	FMFCSR	$r0
	bmski33 $r0, #2		/* Is $fpcsr.IVO('b2) set ?  */
	bnez	$r0, .L_EDOM	/* Set errno as EDOM.  */

	bmski33 $r0, #4		/* Is $fpcsr.OVF('b4) set ?  */
	bnez	$r0, .L_ERANGE	/* Set errno as ERANGE.  */

	bmski33 $r0, #5		/* Is $fpcsr.UDF('b5) set ?  */
	bnez	$r0, .L_ERANGE	/* Set errno as ERANGE.  */

	/* No error at all.  Just ret.  */
	ret

.L_EDOM:
	movi	$r0, #33	/* EDOM: Math arg out of domain of func.  */
	j	.L_Set_errno
.L_ERANGE:
	movi	$r0, #34	/* ERANGE: Math result not representable.  */
.L_Set_errno:
	l.w	$r15, _impure_ptr
	swi	$r0, [$r15]	/* Set errno.  */
	ret
	.size sqrt, .-sqrt
