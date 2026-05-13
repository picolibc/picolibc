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

#define _DEFAULT_SOURCE
#include <sys/resource.h>
#include <stdio.h>
#include <assert.h>

static const int resources[] = {
    RLIMIT_CORE, RLIMIT_CPU, RLIMIT_DATA, RLIMIT_FSIZE, RLIMIT_NOFILE, RLIMIT_STACK, RLIMIT_AS,
};

#define NUM_RESOURCE (sizeof(resources) / sizeof(resources[0]))

#ifdef __PICOLIBC__
__typeof(getrlimit) __fake_getrlimit __weak;
#define is_fake(sym) ((sym) == __fake_##sym)
#else
#define is_fake(sym) 0
#endif

int
main(void)
{
    struct rlimit rlimit;
    int           ret;
    size_t        r;

    if (is_fake(getrlimit)) {
        printf("fake getrlimit, skipping checks\n");
        return 77;
    }

    for (r = 0; r < NUM_RESOURCE; r++) {
        rlimit.rlim_cur = 0x88888888;
        rlimit.rlim_max = 0x77777777;
        ret = getrlimit(resources[r], &rlimit);
        assert(ret == 0);
        if (rlimit.rlim_max != RLIM_INFINITY)
            assert(rlimit.rlim_cur <= rlimit.rlim_max);
    }
    return 0;
}
