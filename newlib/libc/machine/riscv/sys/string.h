/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#ifndef _SYS_STRING_H
#define _SYS_STRING_H

#include "asm.h"

#if __riscv_zbb
  #include <riscv_bitmanip.h>

  // Determine which intrinsics to use based on XLEN and endianness
  #if __riscv_xlen == 64
    #define __LIBC_RISCV_ZBB_ORC_B(x)       __riscv_orc_b_64(x)

    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define __LIBC_RISCV_ZBB_CNT_Z(x)     __riscv_ctz_64(x)
    #else
      #define __LIBC_RISCV_ZBB_CNT_Z(x)     __riscv_clz_64(x)
    #endif
  #else
    #define __LIBC_RISCV_ZBB_ORC_B(x)       __riscv_orc_b_32(x)

    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define __LIBC_RISCV_ZBB_CNT_Z(x)     __riscv_ctz_32(x)
    #else
      #define __LIBC_RISCV_ZBB_CNT_Z(x)     __riscv_clz_32(x)
    #endif
  #endif
#endif


static __inline uintxlen_t __libc_detect_null(uintxlen_t w)
{
#if __riscv_zbb
  /*
    If there are any zeros in each byte of the register, all bits will
    be unset for that byte value, otherwise, all bits will be set.
    If the value is -1, all bits are set, meaning no null byte was found.
  */
  return __LIBC_RISCV_ZBB_ORC_B(w) != -1;
#else
  uintxlen_t mask = 0x7f7f7f7f;
  #if __riscv_xlen == 64
    mask = ((mask << 16) << 16) | mask;
  #endif
  return ~(((w & mask) + mask) | w | mask);
#endif
}

#endif
