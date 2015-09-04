/*
 * sim-close.S -- close interface for FT32 simulator
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
 * $r0          -- File descriptor to close.
 *
 * Output:
 * $r0          -- 0 or -1.
 * errno  -- Set if an error
 */

        .globl        _close
        .type        _close,@function
        .weak        close
        .text
_close:
close:
#         swi        SYS_close
        return
.Lend:
        .size        _close,.Lend-_close
