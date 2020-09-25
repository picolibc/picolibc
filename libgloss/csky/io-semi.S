/* Copyright (c) 2020  C-SKY Microsystems All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

/* int __hosted (int sys_operate, int *args); */
.text
.global __hosted
.type   __hosted, @function
.align 2

__hosted:
    nop
    bkpt
    .long 0x7fff7fff
    rts
