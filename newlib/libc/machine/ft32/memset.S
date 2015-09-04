/* A memset.c for FT32
   Copyright (C) 2014 FTDI (support@ftdichip.com)

   The authors hereby grant permission to use, copy, modify, distribute,
   and license this software and its documentation for any purpose, provided
   that existing copyright notices are retained in all copies and that this
   notice is included verbatim in any distributions. No written agreement,
   license, or royalty fee is required for any of the authorized uses.
   Modifications to this software may be copyrighted by their authors
   and need not follow the licensing terms described here, provided that
   the new terms are clearly indicated on the first page of each file where
   they apply.  */

        .text
        .global        memset
        .type        memset,@function
memset:
        ldk    $r3,32764
1:
        cmp    $r2,$r3
        jmpc   lte,2f
        memset.b $r0,$r1,$r3
        add    $r0,$r0,$r3
        sub    $r2,$r2,$r3
        jmp    1b
2:
        memset.b $r0,$r1,$r2
        return
.Lend2:
        .size        memset,.Lend2-memset
