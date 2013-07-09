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
      memset - fill memory with a constant byte
    Syntax:
      void *memset(void *s, int c, size_t n);
    Description:
      The memset function copies the value of c (converted to an unsigned char)
      into each of the first n characters of the object pointed to by s.
    Return value:
      The memset function returns the value of s.
*/
	.text
	.align	2
	.globl	memset
	.type	memset, @function
memset:
	/* Corner case. If n is zero, just go return.  */
	beqz    $r2, .Lend_memset

	/* Keep $r0 as return value.
	   Set $r4 as how many words to copy.
	   Set $r2 as how many bytes are less than a word.  */
	move    $r5, $r0
	srli    $r4, $r2, 2
	andi    $r2, $r2, 3
	beqz    $r4, .Lbyte_set

	/* Set $r1 a word-size pattern composed of the value of c 
	  (converted to an unsigned char). Convert ??????ab to abababab.  */
	andi    $r1, $r1, 0xff	/* Set $r1 = 000000ab.  */
	slli    $r3, $r1, 8	/* Set $r3 = 0000ab00.  */
	or      $r1, $r1, $r3	/* Set $r1 = 0000abab.  */
	slli    $r3, $r1, 16	/* Set $r3 = abab0000.  */
	or      $r1, $r1, $r3	/* Set $r1 = abababab.  */

.Lword_set:
	/* Do the word set $r4 times. Then, do the byte set $r2 times.  */
	addi    $r4, $r4, -1
	smw.bim $r1, [$r5], $r1		/* Set a word-size.  */
	bnez    $r4, .Lword_set		/* Loop again ?  */
	beqz    $r2, .Lend_memset	/* Fall THRU or go return ?  */

.Lbyte_set:
	/* Do the byte set $r2 times.  */
	addi    $r2, $r2, -1
	sbi.p   $r1, [$r5], 1		/* Set a byte-size.  */
	bnez    $r2, .Lbyte_set		/* Loop again ?  */

.Lend_memset:
	ret
	.size   memset, .-memset
