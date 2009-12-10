/*
 * Copyright (C) 2006 KPIT Cummins
 * Copyright (C) 2009 Conny Marco Menebröcker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
	.section .text
	.global _start
	.align 2
_start:
	#if __xc16xL__
	mov dpp0,#0x2
	#else
	mov dpp0,#0x300
	#endif
	mov dpp1,#1
	mov dpp2,#2
	mov dpp3,#3
	mov sp,#0xfc00
	mov r0,#0xc000
	#if __xc16x__
	mov csp,#0x0
	mov dpp0,#0x0
	#else
	mov csp,#0xc0
      	#endif
        
#if __xc16xL__
calls #SEG:_main,#SOF:_main
#else
calla cc_UC,_main
#endif
 	mov r8,#0x0	
	trap #0	
ret
