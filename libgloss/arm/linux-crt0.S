/** Linux startup code for the ARM processor.
 * Written by Shaun Jackman <sjackman@gmail.com>.
 * Copyright 2006 Pathway Connectivity
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

.global _start
.type _start, %function
_start:
#if __thumb__
	/* Switch to Thumb mode. */
	adr r0, _start_thumb+1
	bx r0
.size _start, .-_start
.global _start_thumb
.thumb_func
_start_thumb:
#endif

#if 0
	/* Clear the BSS. This task is normally handled by the kernel. */
	ldr r0, =__bss_start
	mov r1, #0
	ldr r2, =_end
	sub r2, r2, r0
	bl memset
#endif

	pop {r0} @ argc
	mov r1, sp @ argv
	lsl r2, r0, #2
	add r2, r1
	add r2, #4 @ envp
	ldr r3, =environ
	str r2, [r3]
	bl main
	bl exit
	b .

#if __thumb__
.size _start_thumb, .-_start_thumb
#else
.size _start, .-_start
#endif
