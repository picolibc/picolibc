/*
 * sim-write.S -- write interface for FT32 simulator
 *
 * Copyright (C) 2015 FTDI (support@ftdichip.com)
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
 * $r0            -- File descriptor.
 * $r1            -- String to be printed.
 * $r2      -- Length of the string.
 *
 * Output:
 * $r0          -- Length written or -1.
 * errno  -- Set if an error
 */

        .globl        _write
        .type        _write,@function
        .weak        write
        .text
_write:
write:

#         LDI.l   $r1,$r0,0
#         STA.l   MAGIC,$r1
#         jmp     .f
#
# .loop:
#         sta.l   0x10000,$r2
#         add.l   $r0,$r0,1
# .f:
#         ldi.b   $r2,$r0,0
#         cmp.b   $r2,0
#         jmpc    nz,.loop

        ldk.l   $r3,0x10000
        streamout.b $r3,$r1,$r2
        move.l  $r0,$r2

        return

.Lend:
        .size        _write,.Lend-_write
