/* linux-syscalls - Syscall interface to microblaze linux
 *
 * Copyright (c) 2011 Edgar E. Iglesias
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

#include "linux-syscalls.h"

#define FUNC(name) .type name, %function; name:

#define GLOBAL(name) .global name; FUNC(name)
#define SIZE(name) .size name, .-name

# define SYSCALL_BODY(name)		\
	addik	r12, r0, SYS_ ## name;	\
	brki    r14, 8;			\
	rtsd	r15, 8;			\
	nop;

# define SYSCALL(name)			\
        GLOBAL(_ ## name);		\
	SYSCALL_BODY(name);		\
        SIZE(_ ## name)

SYSCALL(brk)
SYSCALL(exit)
SYSCALL(read)
SYSCALL(write)
SYSCALL(open)
SYSCALL(close)
SYSCALL(lseek)
SYSCALL(fstat)
SYSCALL(unlink)
SYSCALL(getpid)
SYSCALL(kill)
SYSCALL(rt_sigaction)
