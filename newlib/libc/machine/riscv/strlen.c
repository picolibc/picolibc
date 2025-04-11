/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <picolibc.h>

#include <string.h>
#include <stdint.h>
#include "rv_string.h"

size_t strlen(const char *str)
{
  const char *start = str;

#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  while (*str++)
    ;
  return str - start - 1;
#else
  if (__builtin_expect ((uintxlen_t)str & (sizeof (uintxlen_t) - 1), 0)) do
    {
      char ch = *str;
      str++;
      if (!ch)
      return str - start - 1;
    } while ((uintxlen_t)str & (sizeof (uintxlen_t) - 1));

  uintxlen_t *ps = (uintxlen_t *)str;
  uintxlen_t psval;

  while (!__libc_detect_null ((psval = *ps)))
      ps++;
  __asm__ volatile ("" : "+r"(ps)); /* prevent "optimization" */

  str = (const char *)ps;

  #if __riscv_zbb
    psval = ~__LIBC_RISCV_ZBB_ORC_B(psval);
    psval = __LIBC_RISCV_ZBB_CNT_Z(psval);

    str += (psval >> 3);
  #else
    while (psval & 0xff) {
        psval >>= 8;
        str++;
    }
  #endif
    return str - start;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
