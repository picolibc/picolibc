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
#include <errno.h>
#include <stdbool.h>
#include <string.h>

static const int resources[] = {
    RLIMIT_CORE, RLIMIT_CPU, RLIMIT_DATA, RLIMIT_FSIZE, RLIMIT_NOFILE, RLIMIT_STACK, RLIMIT_AS,
};

#define IS_MEM(r)    ((r) == RLIMIT_DATA || (r) == RLIMIT_STACK || (r) == RLIMIT_AS)

#define NUM_RESOURCE (sizeof(resources) / sizeof(resources[0]))

#ifdef __PICOLIBC__
__typeof(setrlimit) __fake_setrlimit __weak;
#define is_fake(sym) ((sym) == __fake_##sym)
#else
#define is_fake(sym) 0
#endif

static bool
is_qemu(void)
{
    FILE *f = fopen("/proc/cpuinfo", "r");

    if (!f)
        return false;
    static char line[1024];
    bool        ret = false;

    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, "BogoMIPS")) {
            if (strstr(line, "100.00")) {
                printf("Running under QEMU, tests relaxed\n");
                ret = true;
            }
            break;
        }
    }
    fclose(f);
    return ret;
}

int
main(void)
{
    struct rlimit rlimit;
    int           ret;
    size_t        r;
    bool          qemu;

    if (is_fake(setrlimit)) {
        printf("fake setrlimit, skipping checks\n");
        return 77;
    }
    qemu = is_qemu();

    for (r = 0; r < NUM_RESOURCE; r++) {

        /* Fetch the current values */
        ret = getrlimit(resources[r], &rlimit);
        assert(ret == 0);

        /* Set the soft limit to a small value, within the hard limit */
        rlimit.rlim_cur = rlimit.rlim_max < 1 ? rlimit.rlim_max : 1;
        ret = setrlimit(resources[r], &rlimit);
        assert(ret == 0);

        /* Reset the soft limit back to the hard limit */
        rlimit.rlim_cur = rlimit.rlim_max;
        ret = setrlimit(resources[r], &rlimit);
        assert(ret == 0);

        /*
         * Set soft and hard limits, with soft limit < hard limit. Raising
         * the hard limit needs privilege, so only when it's at least 2.
         */
        if (rlimit.rlim_max >= 2) {
            rlimit.rlim_max = 2;
            rlimit.rlim_cur = 1;
            ret = setrlimit(resources[r], &rlimit);
            assert(ret == 0);
        }

        /* Set soft and hard limits, with soft limit > hard limit (should fail) */
        rlimit.rlim_max = 1;
        rlimit.rlim_cur = 2;
        ret = setrlimit(resources[r], &rlimit);
        assert((qemu && IS_MEM(resources[r])) || (ret == -1 && errno == EINVAL));
    }
    return 0;
}
