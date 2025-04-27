/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include "rv_string.h"

#ifdef _MACHINE_RISCV_MEMCPY_C_
#include <string.h>
#include <stdint.h>
#include "../../string/local.h"
#include "asm.h"
#include "xlenint.h"

#define unlikely(X) __builtin_expect(!!(X), 0)

#undef memcpy

void * __no_builtin
#if __riscv_misaligned_slow || __riscv_misaligned_fast
__disable_sanitizer
#endif
memcpy(void * __restrict aa, const void * __restrict bb, size_t n)
{
#define BODY(a, b, t)  \
    {                  \
        t tt = *b;     \
        a++, b++;      \
        *(a - 1) = tt; \
    }

    char       *a = (char *)aa;
    const char *b = (const char *)bb;
    char       *end = a + n;
    uintptr_t   msk = SZREG - 1;
#if __riscv_misaligned_slow || __riscv_misaligned_fast
    if (n < SZREG)
#else
    if (unlikely((((uintptr_t)a & msk) != ((uintptr_t)b & msk)) || n < SZREG))
#endif
    {
    small:
        if (__builtin_expect(a < end, 1))
            while (a < end)
                BODY(a, b, char);
        return aa;
    }

    if (unlikely(((uintptr_t)a & msk) != 0))
        while ((uintptr_t)a & msk)
            BODY(a, b, char);

    uintxlen_t       *la = (uintxlen_t *)a;
    const uintxlen_t *lb = (const uintxlen_t *)b;
    uintxlen_t       *lend = (uintxlen_t *)((uintptr_t)end & ~msk);

    if (unlikely(lend - la > 8)) {
        while (lend - la > 8) {
            uintxlen_t b0 = *lb++;
            uintxlen_t b1 = *lb++;
            uintxlen_t b2 = *lb++;
            uintxlen_t b3 = *lb++;
            uintxlen_t b4 = *lb++;
            uintxlen_t b5 = *lb++;
            uintxlen_t b6 = *lb++;
            uintxlen_t b7 = *lb++;
            uintxlen_t b8 = *lb++;
            *la++ = b0;
            *la++ = b1;
            *la++ = b2;
            *la++ = b3;
            *la++ = b4;
            *la++ = b5;
            *la++ = b6;
            *la++ = b7;
            *la++ = b8;
        }
    }

    while (la < lend)
        BODY(la, lb, uintxlen_t);

    a = (char *)la;
    b = (const char *)lb;
    if (unlikely(a < end))
        goto small;
    return aa;
}
#endif /* _MACHINE_RISCV_MEMCPY_C_ */
