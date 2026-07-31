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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

static struct {
    const char *command;
    bool        exited;
    int         exit;
    bool        signaled;
    int         sig;
} tests[] = {
    { .command = "exit 0", .exited = true, .exit = 0, .signaled = false, .sig = 0 },
    { .command = "exit 1", .exited = true, .exit = 1, .signaled = false, .sig = 0 },
    {
     .command = "kill -INT $$",
     .exited = false,
     .exit = 0,
     .signaled = true,
     .sig = SIGINT,
     },
    {
     .command = "kill -USR1 $$",
     .exited = false,
     .exit = 0,
     .signaled = true,
     .sig = SIGUSR1,
     },
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))

int
main(void)
{
    size_t t;
    int    rc;
    int    ret = 0;

    for (t = 0; t < NTESTS; t++) {
        rc = system(tests[t].command);

        if (WIFEXITED(rc) && tests[t].exited && WEXITSTATUS(rc) == tests[t].exit) {
            printf("'%s' exited as expected with status %d\n", tests[t].command, WEXITSTATUS(rc));
        } else if (WIFSIGNALED(rc) && tests[t].signaled && WTERMSIG(rc) == tests[t].sig) {
            printf("'%s' terminated as expected with signal %d\n", tests[t].command, WTERMSIG(rc));
        } else {
            if (WIFEXITED(rc))
                printf("'%s' 0x%x exited with status %d\n", tests[t].command, rc, WEXITSTATUS(rc));
            if (WIFSIGNALED(rc))
                printf("'%s' 0x%x terminated with signal %d\n", tests[t].command, rc, WTERMSIG(rc));
            if (tests[t].exited)
                printf("    expected to exit with status %d\n", tests[t].exit);
            if (tests[t].signaled)
                printf("    expected to terminate with signal %d\n", tests[t].sig);
            ret = 1;
        }
    }
    return ret;
}
