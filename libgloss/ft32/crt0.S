/* crt0.S -- startup file for FT32
 *
 * Copyright (c) 2015  FTDI
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

        .globl        __start
        .weak        _start
        .text
        .type        __start,@function

__start:
_start:

    jmp   truestart
    jmp   truestart

    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti
    reti

truestart:
        ldk.l   $sp, 0xfffc        /* set the top of stack */
        ldk.l   $fp,0           /* zero fp to allow unwinders to stop */

        /* Set argc and argv to zero */
        ldk.l        $r0, 0
        ldk.l        $r1, 0

        sub.l   $sp,$sp,24    # room for the args to main
        call        main
        call        exit

.Lend:
        .size        __start,(.Lend-__start)
