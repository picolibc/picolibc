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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/resource.h>
#include <stdio.h>

int
main(void)
{
    printf("#define LINUX_RLIMIT_CPU %d\n", RLIMIT_CPU);
    printf("#define LINUX_RLIMIT_FSIZE %d\n", RLIMIT_FSIZE);
    printf("#define LINUX_RLIMIT_DATA %d\n", RLIMIT_DATA);
    printf("#define LINUX_RLIMIT_STACK %d\n", RLIMIT_STACK);
    printf("#define LINUX_RLIMIT_CORE %d\n", RLIMIT_CORE);
    printf("#define LINUX_RLIMIT_NOFILE %d\n", RLIMIT_NOFILE);
    printf("#define LINUX_RLIMIT_AS %d\n", RLIMIT_AS);
    printf("#define LINUX_RLIM_INFINITY %lluU\n", (rlim_t)RLIM_INFINITY);
    printf("#define LINUX_RLIM_SAVED_MAX %lluU\n", (rlim_t)RLIM_SAVED_MAX);
    printf("#define LINUX_RLIM_SAVED_CUR %lluU\n", (rlim_t)RLIM_SAVED_CUR);
    printf("\n");
    printf("static inline int\n"
           "_rlimit_to_linux(int resource)\n"
           "{\n"
           "    switch(resource) {\n"
           "    case RLIMIT_CPU: return LINUX_RLIMIT_CPU;\n"
           "    case RLIMIT_FSIZE: return LINUX_RLIMIT_FSIZE;\n"
           "    case RLIMIT_DATA: return LINUX_RLIMIT_DATA;\n"
           "    case RLIMIT_STACK: return LINUX_RLIMIT_STACK;\n"
           "    case RLIMIT_CORE: return LINUX_RLIMIT_CORE;\n"
           "    case RLIMIT_NOFILE: return LINUX_RLIMIT_NOFILE;\n"
           "    case RLIMIT_AS: return LINUX_RLIMIT_AS;\n"
           "    default: return -1;\n"
           "    }\n"
           "}\n");
    return 0;
}
