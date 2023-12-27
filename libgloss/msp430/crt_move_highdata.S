/* Copyright (c) 2012-2015 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */

#ifdef __MSP430X_LARGE__

#include "memmodel.h"

;; This function is responsible for making sure that the
;; contents of the .upper.data section have their correct startup values.
;; If a copy of the .upper.data section is stored in ROM then this means
;; copying the contents into HIFRAM.  If a copy of .upper.data is stored in a
;; shadow section in HIFRAM then this means copying from the shadow section
;; into the real section.

START_CRT_FUNC 0400 move_highdata
	;;  __rom_highdatacopysize may be zero.  Test this first because
	;; its value may come from the weak definitions above and we do
	;; not want to access the memory at address 0 pointed to by the
	;; weak definition of __upper_data_init.
	mov.w	#__rom_highdatacopysize, R14
	cmp.w	#0, R14
	jeq	3f

	/* Test our status word.  */
	cmpx.w  #0, &__upper_data_init
	jeq	1f
	/* Status word is non-zero - copy from shadow into upper.  */
	mov_	#__high_datastart, R12
	mov_	#__rom_highdatastart, R13
	jmp	2f

1:	/* Status word is zero.  Copy from upper to shadow and change status word.  */
	movx.w  #1, &__upper_data_init
	mov_	#__rom_highdatastart, R12
	mov_	#__high_datastart, R13

2:	;; __rom_highdatacopysize may be zero.  memmove should cope.
	mov.w	#__rom_highdatacopysize, R14

	call_	#memmove
3:
END_CRT_FUNC	move_highdata

#endif /* __MSP430X_LARGE__ */
