/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdatomic.h>
#include <inttypes.h>
#include <stdio.h>

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
_Atomic uint32_t atomic_4;
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
_Atomic uint16_t atomic_2;
#endif

int
main(void)
{
    int ret = 0;
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    uint32_t zero_4 = 0;
    if (atomic_compare_exchange_strong(&atomic_4, &zero_4, 1)) {
        printf("atomic_compare_exchange 4 worked, value %" PRIu32 " zero %" PRIu32 "\n", atomic_4,
               zero_4);
        uint32_t old;
        old = atomic_exchange_explicit(&atomic_4, 0, memory_order_relaxed);
        if (atomic_4 == 0 && old == 1) {
            printf("atomic_exchange_explicit 4 worked %" PRIu32 " value now %" PRIu32 "\n", old,
                   atomic_4);
        } else {
            printf("atomic_exchange_explicit failed %" PRIu32 " value now %" PRIu32 "\n", old,
                   atomic_4);
            ret = 1;
        }
    } else {
        printf("atomic_compare_exchange 4 failed, value %" PRIu32 "\n", atomic_4);
        ret = 1;
    }
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
    uint16_t zero_2 = 0;
    if (atomic_compare_exchange_strong(&atomic_2, &zero_2, 1)) {
        printf("atomic_compare_exchange 2 worked, value %" PRIu16 " zero %" PRIu16 "\n", atomic_2,
               zero_2);
        uint16_t old;
        old = atomic_exchange_explicit(&atomic_2, 0, memory_order_relaxed);
        if (atomic_2 == 0 && old == 1) {
            printf("atomic_exchange_explicit 2 worked %" PRIu16 " value now %" PRIu16 "\n", old,
                   atomic_2);
        } else {
            printf("atomic_exchange_explicit 2 failed %" PRIu16 " value now %" PRIu16 "\n", old,
                   atomic_2);
            ret = 1;
        }
    } else {
        printf("atomic_compare_exchange 2 failed, value %" PRIu16 "\n", atomic_2);
        ret = 1;
    }
#endif
    return ret;
}
