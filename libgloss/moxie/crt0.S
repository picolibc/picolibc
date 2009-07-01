/* crt0.S -- startup file for moxie
 * 
 * Copyright (c) 2008, 2009  Anthony Green
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

	.globl	__start
	.weak	_start
	.text
	.type	__start,@function
__start:
_start:
	ldi.l	$sp, _stack	/* load up stack pointer */
	ldi.l	$fp, 0x0	/* zero fp to allow unwinders to stop */

	/* zero the bss area */
	dec	$sp, 12
	ldi.l	$r0, __bss_start__
	ldi.l	$r1, __bss_end__
	sub.l	$r1, $r0
	sto.l	8($sp), $r1
	ldi.l	$r1, 0
	jsra	memset
	inc	$sp, 12

	/* Call _init to invoke static constructors, etc.  */
	jsra	_init
	
	/* Call _fini at exit time for static destructors.  */
	ldi.l	$r0, _fini
	jsra	atexit
	
	/* Set argc and argv.  These are populated by the simulator.  */
	lda.l	$r0, 0x4
	ldi.l	$r1, 0x8
	
	jsra	main
	jsra	exit

.Lend:
	.size	__start,(.Lend-__start)
