;******************************************************************************
;* SETJMP  v7.2.0I10181                                                       *
;*                                                                            *
;* Copyright (c) 1996-2010 Texas Instruments Incorporated                     *
;* http://www.ti.com/                                                         *
;*                                                                            *
;*  Redistribution and  use in source  and binary forms, with  or without     *
;*  modification,  are permitted provided  that the  following conditions     *
;*  are met:                                                                  *
;*                                                                            *
;*     Redistributions  of source  code must  retain the  above copyright     *
;*     notice, this list of conditions and the following disclaimer.          *
;*                                                                            *
;*     Redistributions in binary form  must reproduce the above copyright     *
;*     notice, this  list of conditions  and the following  disclaimer in     *
;*     the  documentation  and/or   other  materials  provided  with  the     *
;*     distribution.                                                          *
;*                                                                            *
;*     Neither the  name of Texas Instruments Incorporated  nor the names     *
;*     of its  contributors may  be used to  endorse or  promote products     *
;*     derived  from   this  software  without   specific  prior  written     *
;*     permission.                                                            *
;*                                                                            *
;*  THIS SOFTWARE  IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS     *
;*  "AS IS"  AND ANY  EXPRESS OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT     *
;*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     *
;*  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT     *
;*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
;*  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT  NOT     *
;*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
;*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
;*  THEORY OF  LIABILITY, WHETHER IN CONTRACT, STRICT  LIABILITY, OR TORT     *
;*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
;*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
;*                                                                            *
;******************************************************************************

.text
.globl setjmp
.type setjmp,%function
setjmp:
	MV .L2X    A4, B4
||	STW .D1T2  B3, *+A4(48)

	STW .D1T1  A10, *+A4(0)
||	STW .D2T2  B10, *+B4(4)
||	RET .S2    B3

	STW .D1T1  A11, *+A4(8)
||	STW .D2T2  B11, *+B4(12)

	STW .D1T1  A12, *+A4(16)
||	STW .D2T2  B12, *+B4(20)

	STW .D1T1  A13, *+A4(24)
||	STW .D2T2  B13, *+B4(28)

	STW .D1T1  A14, *+A4(32)
||	STW .D2T2  B14, *+B4(36)

	STW .D1T1  A15, *+A4(40)
||	STW .D2T2  B15, *+B4(44)
||	ZERO .S1   A4
.size setjmp, . - setjmp

.globl longjmp
.type longjmp,%function
longjmp:
	 LDW .D1T1  *+A4(48), A3

	 MV .L2X    A4, B6
||	 MV .S1     A4, A6
||	 MV .D2     B4, B2

	 LDW .D1T1  *+A6(0), A10
||	 LDW .D2T2  *+B6(4), B10
|| [B2]  MV .L1X    B4, A4
|| [!B2] MVK .S1    1, A4

	 LDW .D1T1  *+A6(8), A11
||	 LDW .D2T2  *+B6(12), B11 

	 LDW .D1T1  *+A6(16), A12
||	 LDW .D2T2  *+B6(20), B12
 
	 LDW .D1T1  *+A6(24), A13
||	 LDW .D2T2  *+B6(28), B13
 
	 LDW .D1T1  *+A6(32), A14
||	 LDW .D2T2  *+B6(36), B14
 
	 LDW .D1T1  *+A6(40), A15
||	 LDW .D2T2  *+B6(44), B15
||	 RET .S2X   A3
	 NOP    5
.size longjmp, . - longjmp
