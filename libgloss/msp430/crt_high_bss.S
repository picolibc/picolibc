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

;; This function is responsible for initializing the contents of the
;; .upper.bss section.

START_CRT_FUNC 0200 init_highbss

	mov_	#__high_bssstart, R12
	mov.w	#0, R13
	mov_	#__high_bsssize, R14
	;; If __high_bsssize is zero then skip the call to memset.
	;; This can happen if all of the bss data was placed into .either.bss.
	cmp.w	#0, R14
	jeq	1f
	call_	#memset
1:
END_CRT_FUNC	init_highbss

#endif /* __MSP430X_LARGE__ */
