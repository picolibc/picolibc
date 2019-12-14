/*
 * sim-unlink.S -- write interface for moxie simulator
 * 
 * Copyright (c) 2008, 2019 Anthony Green
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

	.globl	_unlink
	.type	_unlink,@function
	.weak	unlink
	.text
_unlink:
unlink:
	swi	SYS_unlink
	ret
.Lend:
	.size	_unlink,.Lend-_unlink
