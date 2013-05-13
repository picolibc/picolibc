/* Copyright (c) 2012-2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

/* This file provides macros for various MSP430 instructions
   which have similar, but not identical, versions when assembling
   for the LARGE memory model.  */

#ifdef __MSP430X_LARGE__

#define call_	CALLA
#define ret_	RETA
#define mov_	MOVA
#define movx_	MOVX
#define br_	BRA
#define cmp_	CMPA
#define add_	ADDA
#define PTRsz	4

#else

#define call_	CALL
#define ret_	RET
#define mov_	MOV
#define movx_	MOV
#define br_	BR
#define cmp_	CMP
#define add_	ADD
#define PTRsz	2


#endif
