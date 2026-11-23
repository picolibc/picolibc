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
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wchar.h>

/*
 * wcsnlen never examines more than the first maxlen characters:
 * checked with arrays that end at a page that can't be read
 */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

/* Through a volatile pointer, so the compiler uses the library's function */
static size_t (* volatile p_wcsnlen)(const wchar_t *, size_t) = wcsnlen;

int
main(void)
{
    static const wchar_t abc[] = L"abc";
    long                 page = sysconf(_SC_PAGESIZE);
    char                *map;
    wchar_t             *end;
    size_t               i;

    check(p_wcsnlen(abc, 10) == 3, "wcsnlen before maxlen");
    check(p_wcsnlen(abc, 3) == 3, "wcsnlen at the null");
    check(p_wcsnlen(abc, 2) == 2, "wcsnlen stops at maxlen");
    check(p_wcsnlen(abc, 0) == 0, "wcsnlen with maxlen 0");

    check(page > 0, "sysconf(_SC_PAGESIZE)");
    map = mmap(NULL, 2 * (size_t)page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    check(map != MAP_FAILED, "mmap");
    check(mprotect(map + page, (size_t)page, PROT_NONE) == 0, "guard page");
    end = (wchar_t *)(map + page);
    for (i = 1; i <= 8; i++)
        end[-(long)i] = L'x';
    for (i = 0; i <= 8; i++)
        check(p_wcsnlen(end - i, i) == i, "wcsnlen reads only maxlen characters");
    check(munmap(map, 2 * (size_t)page) == 0, "munmap");

    printf("success\n");
    return 0;
}
