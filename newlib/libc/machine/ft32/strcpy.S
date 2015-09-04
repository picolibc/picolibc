/* A strcpy.c for FT32
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
        .global        strcpy
        .type        strcpy,@function
strcpy:
        move.l   $r5,$r0
        stpcpy.b $r5,$r1
        return
.Lend2:
        .size        strcpy,.Lend2-strcpy

