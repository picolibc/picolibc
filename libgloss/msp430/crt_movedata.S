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

#include "memmodel.h"

;; This function is responsible for copying the
;; contents of the .data section from its load address (in ROM) to
;; its run-time address (in RAM).

START_CRT_FUNC 0300 movedata

	mov_	#__datastart, R12
	mov_	#__romdatastart, R13

	;;  memmove and memcpy do not currently work when src == dst
	cmp_	R12, R13
	jeq	1f

	mov_	#__romdatacopysize, R14

	call_	#memmove
1:
END_CRT_FUNC	movedata
