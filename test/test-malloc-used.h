/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#ifndef _TEST_MALLOC_USED_H_
#define _TEST_MALLOC_USED_H_

#include <stddef.h>
#if defined(__PICOLIBC__) || defined(__GLIBC__)
#include <malloc.h>
#elif defined(__FreeBSD__)
#include <stdint.h>
#include <malloc_np.h>
#endif

/*
 * Bytes allocated and not yet freed, or (size_t)-1 where the C library
 * can't say
 */
static inline size_t
malloc_used(void)
{
#if defined(__PICOLIBC__)
    return mallinfo().uordblks;
#elif defined(__GLIBC__) && (__GLIBC__ > 2 || __GLIBC_MINOR__ >= 33)
    return mallinfo2().uordblks;
#elif defined(__FreeBSD__)
    uint64_t epoch = 1;
    size_t   esize = sizeof(epoch), used, usize = sizeof(used);

    /* jemalloc counts blocks in the thread cache, and updates its stats on epoch */
    mallctl("thread.tcache.flush", NULL, NULL, NULL, 0);
    mallctl("epoch", &epoch, &esize, &epoch, esize);
    if (mallctl("stats.allocated", &used, &usize, NULL, 0) != 0)
        return (size_t)-1;
    return used;
#else
    return (size_t)-1;
#endif
}

#endif /* _TEST_MALLOC_USED_H_ */
