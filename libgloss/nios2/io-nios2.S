/*
 * Low-level semihosting primitive for csky. 
 *
 * Copyright (c) 2018 Mentor Graphics
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

/* Implement
     void __io_hosted (int func, void *args);
   by causing a break 1.  The break handler can find the two parameters in
   r4 and r5 per the normal C calling convention.  */

	.text
	.align	2
	.global	__io_hosted
	.type	_io_hosted, @function

__io_hosted:
	break 1
	ret

	.size	__io_hosted, .-__io_hosted
