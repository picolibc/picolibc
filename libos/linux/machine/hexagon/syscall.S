/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Hexagon Linux syscall gateway.
 *
 * Implements the generic C-callable:
 *   long syscall(long number, long a1, long a2, long a3,
 *                long a4, long a5, long a6);
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
 * Error detection: the kernel returns a value in [-4095, -1] on error.
 * We check this with an unsigned comparison and branch to _syscall_error
 * which translates the negative Linux errno into a POSIX errno value and
 * returns -1.
 */

    .section .text.syscall, "ax", @progbits
    .globl syscall
    .type  syscall, @function
syscall:
    r6 = r0
    r0 = memw(r29+#0)
    r1 = memw(r29+#4)
    r2 = memw(r29+#8)
    r3 = memw(r29+#12)
    r4 = memw(r29+#16)
    r5 = memw(r29+#20)
    trap0(#1)               /* invoke Linux kernel */

    /*
     * Error check: branch to _syscall_error if r0 in [-4095, -1].
     *
     * Unsigned interpretation: -4095 = 0xFFFFF001, -1 = 0xFFFFFFFF.
     * We want: if (unsigned)r0 > (unsigned)(-4096) -> error.
     * cmp.gtu(r1, r0) sets p0 = ((unsigned)r1 > (unsigned)r0).
     * With r1 = -4096 = 0xFFFFF000:
     *   p0 = (0xFFFFF000 > (unsigned)r0)
     *   p0 is TRUE  when r0 < 0xFFFFF000 (no error, or large positive)
     *   p0 is FALSE when r0 >= 0xFFFFF000 (i.e. r0 in [-4096, -1])
     * We want to return normally when there is NO error, so:
     *   if (p0) jumpr r31   -> return when r0 < 0xFFFFF000 (success or
     *                          large positive return value)
     * Otherwise fall through to _syscall_error.
     *
     * Note: r0 == -4096 (0xFFFFF000) is not a valid Linux error code
     * (errors are in [-4095, -1]), so either classification is harmless.
     */
    r1 = #-4096
    p0 = cmp.gtu(r1, r0)
    if (p0) jumpr r31
    jump _syscall_error
    .size syscall, . - syscall
