/*
 * sim-open.S -- open interface for moxie simulator
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
 * $r0	    -- File name to open.
 * $r1      -- open mode.
 * -0x12($fp) -- optionally, the permission bits to set the file to.
 *
 * Output:
 * $r0	  -- file descriptor or -1.
 * errno  -- Set if an error
 */

	.globl	_open
	.type	_open,@function
	.weak	open
	.text
_open:
open:
	swi	SYS_open
	ret
.Lend:
	.size	_open,.Lend-_open
