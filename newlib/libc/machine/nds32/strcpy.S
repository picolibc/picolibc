/*
Copyright (c) 2013 Andes Technology Corporation.
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


    Function:
       strcpy - copy a string.
    Syntax:
       char *strcpy(char *dest, const char *src);
    Description:
       This function copies the string pointed to by src into the array
       point to by dest (include the teminating null character).
    Return value:
       strcpy returns the dest as given.
*/
	.text
	.align	2
	.globl	strcpy
	.type	strcpy, @function
strcpy:
	move	$r3, $r0	/* Keep r0 as reture value.  */
	/* If SRC or DEST is unaligned, then copy bytes.  */
	or	$r2, $r1, $r0
	andi	$r2, $r2, #3
	bnez	$r2, .Lbyte_mode

.Lword_mode:
	/* SRC and DEST are both "long int" aligned, try to do "long int"
	   sized copies.  */
	/* #define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
	   DETECTNULL returns nonzero if (long)X contains a NULL byte.  */
	lwi	$r2, [$r1+(0)]		/* r2 is X */
	sethi	$r4, hi20(0xFEFEFEFF)
	ori	$r4, $r4, lo12(0xFEFEFEFF)
	add	$r4, $r2, $r4		/* r4 = ((X) - 0x01010101) */
	nor	$r5, $r2, $r2		/* r5 = ~(X) */
	and	$r4, $r5, $r4		/* r4 = ~(X) & ((X) - 0x01010101) */
	sethi	$r5, hi20(0x80808080)
	ori	$r5, $r5, lo12(0x80808080)
	and	$r4, $r4, $r5		/* r4 = r4 & 0x80808080 */
	bnez	$r4, .Lbyte_mode	/* Contains a NULL byte.  */
	swi.bi	$r2, [$r3], #4
	addi	$r1, $r1, #4
	b	.Lword_mode

.Lbyte_mode:
	lbi.bi	$r4, [$r1], #1		/* r4  <- *src++ */
	sbi.bi	$r4, [$r3], #1		/* r4  -> *dest++ */
	bnez	$r4, .Lbyte_mode
	ret
	.size	strcpy, .-strcpy
