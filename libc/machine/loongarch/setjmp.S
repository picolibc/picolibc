/* Copyright (c) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <picolibc.h>

#include "asm.h"

/* Align up for FPR to avoid unaligned access on grlen == 32 & frlen == 64 */
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define FR_OFFSET ALIGN_UP((13*SZREG), SZFREG)

/* int setjmp (jmp_buf);  */
  .section .text.setjmp
  .globl  setjmp
  .type   setjmp, @function
setjmp:
	REG_S $ra,  $a0, 0*SZREG
	REG_S $sp,  $a0, 1*SZREG
	REG_S $r21, $a0, 2*SZREG /* r21 is reserved */
	REG_S $fp,  $a0, 3*SZREG
	REG_S $s0,  $a0, 4*SZREG
	REG_S $s1,  $a0, 5*SZREG
	REG_S $s2,  $a0, 6*SZREG
	REG_S $s3,  $a0, 7*SZREG
	REG_S $s4,  $a0, 8*SZREG
	REG_S $s5,  $a0, 9*SZREG
	REG_S $s6,  $a0, 10*SZREG
	REG_S $s7,  $a0, 11*SZREG
	REG_S $s8,  $a0, 12*SZREG

#ifndef __loongarch_soft_float
	FREG_S $fs0, $a0, FR_OFFSET + 0*SZFREG
	FREG_S $fs1, $a0, FR_OFFSET + 1*SZFREG
	FREG_S $fs2, $a0, FR_OFFSET + 2*SZFREG
	FREG_S $fs3, $a0, FR_OFFSET + 3*SZFREG
	FREG_S $fs4, $a0, FR_OFFSET + 4*SZFREG
	FREG_S $fs5, $a0, FR_OFFSET + 5*SZFREG
	FREG_S $fs6, $a0, FR_OFFSET + 6*SZFREG
	FREG_S $fs7, $a0, FR_OFFSET + 7*SZFREG
#endif

	li.w		$a0, 0
	jirl		$zero, $ra, 0
	.size	setjmp, .-setjmp

/* volatile void longjmp (jmp_buf, int);  */
  .section .text.longjmp
  .globl  longjmp
  .type   longjmp, @function
longjmp:
	REG_L $ra,  $a0, 0*SZREG
	REG_L $sp,  $a0, 1*SZREG
	REG_L $r21, $a0, 2*SZREG
	REG_L $fp,  $a0, 3*SZREG
	REG_L $s0,  $a0, 4*SZREG
	REG_L $s1,  $a0, 5*SZREG
	REG_L $s2,  $a0, 6*SZREG
	REG_L $s3,  $a0, 7*SZREG
	REG_L $s4,  $a0, 8*SZREG
	REG_L $s5,  $a0, 9*SZREG
	REG_L $s6,  $a0, 10*SZREG
	REG_L $s7,  $a0, 11*SZREG
	REG_L $s8,  $a0, 12*SZREG

#ifndef __loongarch_soft_float
	FREG_L $fs0, $a0, FR_OFFSET + 0*SZFREG
	FREG_L $fs1, $a0, FR_OFFSET + 1*SZFREG
	FREG_L $fs2, $a0, FR_OFFSET + 2*SZFREG
	FREG_L $fs3, $a0, FR_OFFSET + 3*SZFREG
	FREG_L $fs4, $a0, FR_OFFSET + 4*SZFREG
	FREG_L $fs5, $a0, FR_OFFSET + 5*SZFREG
	FREG_L $fs6, $a0, FR_OFFSET + 6*SZFREG
	FREG_L $fs7, $a0, FR_OFFSET + 7*SZFREG
#endif

	sltui	$a0, $a1, 1
	ADD	$a0, $a0, $a1	 # a0 = (a1 == 0) ? 1 : a1
	jirl	$zero, $ra, 0
	.size	longjmp, .-longjmp
