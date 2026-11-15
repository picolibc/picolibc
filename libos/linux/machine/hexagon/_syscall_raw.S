/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Hexagon Linux syscall gateway.
 *
 * Implements the generic C-callable:
 *   long _syscall_raw(long number, long a1, long a2, long a3,
 *                     long a4, long a5, long a6);
 *
 * Hexagon Linux syscall ABI:
 *   r6  = syscall number
 *   r0  = argument 1
 *   r1  = argument 2
 *   r2  = argument 3
 *   r3  = argument 4
 *   r4  = argument 5
 *   r5  = argument 6
 *   trap0(#1) = kernel entry
 *   r0  = return value (negative errno on error)
 *
 * Hexagon homes variadic arguments in the caller's outgoing stack area.
 * The fixed syscall number arrives in r0; the syscall arguments are at
 * r29+#0 .. r29+#20.
 *
 * The kernel's result goes back to the caller unchanged; _syscall
 * converts errors.
 */

    .section .text._syscall_raw, "ax", @progbits
    .globl _syscall_raw
    .type  _syscall_raw, @function
_syscall_raw:
    r6 = r0
    r0 = memw(r29+#0)
    r1 = memw(r29+#4)
    r2 = memw(r29+#8)
    r3 = memw(r29+#12)
    r4 = memw(r29+#16)
    r5 = memw(r29+#20)
    trap0(#1)               /* invoke Linux kernel */

    jumpr r31
    .size _syscall_raw, . - _syscall_raw
