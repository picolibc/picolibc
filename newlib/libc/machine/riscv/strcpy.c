/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <string.h>
#include <stdint.h>
#include "sys/asm.h"

char *strcpy(char *dst, const char *src)
{
  char *dst0 = dst;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
#if !(__riscv_misaligned_slow || __riscv_misaligned_fast)
  int misaligned = ((uintxlen_t)dst | (uintxlen_t)src) & (sizeof (uintxlen_t) - 1);
  if (__builtin_expect(!misaligned, 1))
#endif
    {
      uintxlen_t *pdst = (uintxlen_t *)dst;
      const uintxlen_t *psrc = (const uintxlen_t *)src;

      while (!__libc_detect_null(*psrc))
        *pdst++ = *psrc++;

      dst = (char *)pdst;
      src = (const char *)psrc;

      if (!(*dst++ = src[0])) return dst0;
      if (!(*dst++ = src[1])) return dst0;
      if (!(*dst++ = src[2])) return dst0;
      if (sizeof (*pdst ) == 4) goto out;
      if (!(*dst++ = src[3])) return dst0;
      if (!(*dst++ = src[4])) return dst0;
      if (!(*dst++ = src[5])) return dst0;
      if (!(*dst++ = src[6])) return dst0;

out:
      *dst = 0;
      return dst0;
    }
#endif /* not PREFER_SIZE_OVER_SPEED */

  char ch;
  do
    {
      ch = *src;
      src++;
      dst++;
      *(dst - 1) = ch;
    } while (ch);

  return dst0;
}
