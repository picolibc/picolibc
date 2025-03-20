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

static __inline uintxlen_t __libc_detect_null(uintxlen_t w)
{
  uintxlen_t mask = 0x7f7f7f7f;
  if (sizeof (w) == 8)
    mask = ((mask << 16) << 16) | mask;
  return ~(((w & mask) + mask) | w | mask);
}

#endif
