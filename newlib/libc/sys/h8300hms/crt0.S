; h8/300 and h8/300h start up file.

#include "setarch.h"

#ifdef __H8300__

	.section .text
	.global	_start
_start:
	mov.w	#_stack,sp
	mov.w	#_edata,r0
	mov.w	#_end,r2
	sub.w   r1,r1
	sub.w	r0,r2
	jsr	@_memset
#ifdef __ELF__
	mov.l   #__fini,r0
	jsr     @_atexit
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr     @__init
#else
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr	@___main
#endif
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.word 	1

#endif

#ifdef __H8300H__

	.section .text
	.global	_start
_start:
	mov.l	#_stack,sp
	mov.l	#_edata,er0
	mov.l	#_end,er2
	sub.w   r1,r1
	sub.l	er0,er2
	jsr	@_memset
#ifdef __ELF__
	mov.l   #__fini,er0
	jsr     @_atexit
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr     @__init
#else
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr	@___main
#endif
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.long 	1

#endif

#if defined (__H8300S__) || defined (__H8300SX__)

	.section .text
	.global	_start
_start:
	mov.l	#_stack,sp
	mov.l	#_edata,er0
	mov.l	#_end,er2
	sub.w   r1,r1
	sub.l	er0,er2
	jsr	@_memset
#ifdef __ELF__
	mov.l   #__fini,er0
	jsr     @_atexit
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr     @__init
#else
#ifdef __SIMULATOR__
        jsr     @0xcc
#endif
	jsr	@___main
#endif
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.long 	1

#endif
