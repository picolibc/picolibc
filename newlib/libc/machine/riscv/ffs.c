/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/
#include <strings.h>

int
ffs (int word)
{
#if __riscv_xlen == 32
  return (__builtin_ffs (word));
#else
  int i;

  if (!word)
    return 0;

  i = 0;
  for (;;)
    {
      if (((1 << i++) & word) != 0)
       return i;
    }
  return 0;
#endif
}
