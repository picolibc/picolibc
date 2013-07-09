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
      memcpy - copy memory regions
    Syntax:
      void *memcpy(void *s1, const void *s2, size_t n);
    Description:
      The memcpy function copies n characters from the object pointed to
      by s2 into the object pointed to by s1. If copying takes place
      between objects that overlap, the behavior is undeﬁned.
    Return value:
      The memcpy function returns the value of s1.
*/
	.text
	.align	2
	.globl	memcpy
	.type	memcpy, @function
memcpy:
	/* Corner cases. If *s1 equals *s2
	   or size_t is zero, just go return.  */
	beq	$r0, $r1, .Lend_memcpy
	beqz	$r2, .Lend_memcpy

	/* Keep *s1 as return value.
	   Set $r3 as how many words to copy.
	   Set $r2 as how many bytes are less than a word.  */
	move	$r5, $r0
	srli	$r3, $r2, 2
	andi	$r2, $r2, 3
	beqz	$r3, .Lbyte_copy

.Lword_copy:
	/* Do the word copy $r3 times. Then, do the byte copy $r2 times.  */
	lmw.bim	$r4, [$r1], $r4, 0
	addi	$r3, $r3, -1
	smw.bim	$r4, [$r5], $r4, 0
	bnez	$r3, .Lword_copy	/* Loop again ?  */
	beqz	$r2, .Lend_memcpy	/* Fall THRU or go return ?  */

.Lbyte_copy:
	/* Do the byte copy $r2 times.  */
	lbi.bi	$r4, [$r1], 1
	addi	$r2, $r2, -1
	sbi.bi	$r4, [$r5], 1
	bnez	$r2, .Lbyte_copy	/* Loop again ?  */

.Lend_memcpy:
	ret
	.size   memcpy, .-memcpy
