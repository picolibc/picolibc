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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/*
 * sbrk, and so malloc, keep working when a mapping right after the
 * break stops it from growing
 */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

#define PAGE   ((uintptr_t)4096)
#define SIZE   (4 * 1024)
#define NSMALL 1000
#define SMALL  1000

static void *small[NSMALL];

int
main(void)
{
    uintptr_t brk_end;
    void     *block;
    char     *p;

    /* Map a page just past the current break */
    brk_end = ((uintptr_t)sbrk(0) + PAGE - 1) & ~(PAGE - 1);
    block = mmap((void *)brk_end, PAGE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check(block != MAP_FAILED, "mmap");
    /* If something else is already there, the break is blocked anyway */
    if ((uintptr_t)block != brk_end)
        check(munmap(block, PAGE) == 0, "munmap");

    p = sbrk(SIZE);
    check(p != (void *)-1, "sbrk beyond the blocked break");
    memset(p, 0xa5, SIZE);
    check(sbrk(0) == p + SIZE, "break after growing");

    /*
     * Growing the last free chunk gets memory which doesn't follow
     * it; malloc has to keep that memory without taking its lock again
     */
    p = malloc(64 * 1024);
    check(p != NULL, "malloc after the blocked break");
    free(p);
    p = malloc(SIZE / 2);
    check(p != NULL, "malloc growing the last chunk");
    memset(p, 0x5a, SIZE / 2);
    free(p);

    for (int i = 0; i < NSMALL; i++)
        check((small[i] = malloc(SMALL)) != NULL, "many small allocations");
    for (int i = 0; i < NSMALL; i++)
        free(small[i]);

    printf("success\n");
    return 0;
}
