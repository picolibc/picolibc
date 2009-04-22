/*
 * sim-read.S -- read interface for moxie simulator
 * 
 * Copyright (c) 2008 Anthony Green
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

#include "syscall.h"

/*
 * Input:
 * $r0	      -- File descriptor.
 * $r1	      -- Buffer to be read into.
 * -0x12($fp) -- Length of the buffer.
 *
 * Output:
 * $r0	  -- Length read or -1.
 * errno  -- Set if an error
 */

	.globl	_read
	.type	_read,@function
	.weak	read
	.text
_read:
read:
	swi	SYS_read
	ret
.Lend:
	.size	_read,.Lend-_read
