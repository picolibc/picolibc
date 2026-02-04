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

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>

int
main(void)
{
#ifdef _SC_ARG_MAX
    printf("#define LINUX_SC_ARG_MAX %d\n", _SC_ARG_MAX);
#endif
#ifdef _SC_CHILD_MAX
    printf("#define LINUX_SC_CHILD_MAX %d\n", _SC_CHILD_MAX);
#endif
#ifdef _SC_HOST_NAME_MAX
    printf("#define LINUX_SC_HOST_NAME_MAX %d\n", _SC_HOST_NAME_MAX);
#endif
#ifdef _SC_LOGIN_NAME_MAX
    printf("#define LINUX_SC_LOGIN_NAME_MAX %d\n", _SC_LOGIN_NAME_MAX);
#endif
#ifdef _SC_NGROUPS_MAX
    printf("#define LINUX_SC_NGROUPS_MAX %d\n", _SC_NGROUPS_MAX);
#endif
#ifdef _SC_CLK_TCK
    printf("#define LINUX_SC_CLK_TCK %d\n", _SC_CLK_TCK);
#endif
#ifdef _SC_OPEN_MAX
    printf("#define LINUX_SC_OPEN_MAX %d\n", _SC_OPEN_MAX);
#endif
#ifdef _SC_PAGESIZE
    printf("#define LINUX_SC_PAGESIZE %d\n", _SC_PAGESIZE);
#endif
#ifdef _SC_PAGE_SIZE
    printf("#define LINUX_SC_PAGE_SIZE %d\n", _SC_PAGE_SIZE);
#endif
#ifdef _SC_PAGESIZE
    printf("#define LINUX_SC_PAGESIZE. %d\n", _SC_PAGESIZE);
#endif
#ifdef _SC_RE_DUP_MAX
    printf("#define LINUX_SC_RE_DUP_MAX %d\n", _SC_RE_DUP_MAX);
#endif
#ifdef _SC_STREAM_MAX
    printf("#define LINUX_SC_STREAM_MAX %d\n", _SC_STREAM_MAX);
#endif
#ifdef _SC_SYMLOOP_MAX
    printf("#define LINUX_SC_SYMLOOP_MAX %d\n", _SC_SYMLOOP_MAX);
#endif
#ifdef _SC_TTY_NAME_MAX
    printf("#define LINUX_SC_TTY_NAME_MAX %d\n", _SC_TTY_NAME_MAX);
#endif
#ifdef _SC_TZNAME_MAX
    printf("#define LINUX_SC_TZNAME_MAX %d\n", _SC_TZNAME_MAX);
#endif
#ifdef _SC_VERSION
    printf("#define LINUX_SC_VERSION %d\n", _SC_VERSION);
#endif
#ifdef _SC_BC_BASE_MAX
    printf("#define LINUX_SC_BC_BASE_MAX %d\n", _SC_BC_BASE_MAX);
#endif
#ifdef _SC_BC_DIM_MAX
    printf("#define LINUX_SC_BC_DIM_MAX %d\n", _SC_BC_DIM_MAX);
#endif
#ifdef _SC_BC_SCALE_MAX
    printf("#define LINUX_SC_BC_SCALE_MAX %d\n", _SC_BC_SCALE_MAX);
#endif
#ifdef _SC_BC_STRING_MAX
    printf("#define LINUX_SC_BC_STRING_MAX %d\n", _SC_BC_STRING_MAX);
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    printf("#define LINUX_SC_COLL_WEIGHTS_MAX %d\n", _SC_COLL_WEIGHTS_MAX);
#endif
#ifdef _SC_EXPR_NEST_MAX
    printf("#define LINUX_SC_EXPR_NEST_MAX %d\n", _SC_EXPR_NEST_MAX);
#endif
#ifdef _SC_LINE_MAX
    printf("#define LINUX_SC_LINE_MAX %d\n", _SC_LINE_MAX);
#endif
#ifdef _SC_RE_DUP_MAX
    printf("#define LINUX_SC_RE_DUP_MAX %d\n", _SC_RE_DUP_MAX);
#endif
#ifdef _SC_2_VERSION
    printf("#define LINUX_SC_2_VERSION %d\n", _SC_2_VERSION);
#endif
#ifdef _SC_2_C_DEV
    printf("#define LINUX_SC_2_C_DEV %d\n", _SC_2_C_DEV);
#endif
#ifdef _SC_2_FORT_DEV
    printf("#define LINUX_SC_2_FORT_DEV %d\n", _SC_2_FORT_DEV);
#endif
#ifdef _SC_2_FORT_RUN
    printf("#define LINUX_SC_2_FORT_RUN %d\n", _SC_2_FORT_RUN);
#endif
#ifdef _SC_2_LOCALEDEF
    printf("#define LINUX_SC_2_LOCALEDEF %d\n", _SC_2_LOCALEDEF);
#endif
#ifdef _SC_2_SW_DEV
    printf("#define LINUX_SC_2_SW_DEV %d\n", _SC_2_SW_DEV);
#endif
#ifdef _SC_PHYS_PAGES
    printf("#define LINUX_SC_PHYS_PAGES %d\n", _SC_PHYS_PAGES);
#endif
#ifdef _SC_PAGESIZE
    printf("#define LINUX_SC_PAGESIZE %d\n", _SC_PAGESIZE);
#endif
#ifdef _SC_AVPHYS_PAGES
    printf("#define LINUX_SC_AVPHYS_PAGES %d\n", _SC_AVPHYS_PAGES);
#endif
#ifdef _SC_NPROCESSORS_CONF
    printf("#define LINUX_SC_NPROCESSORS_CONF %d\n", _SC_NPROCESSORS_CONF);
#endif
#ifdef _SC_NPROCESSORS_ONLN
    printf("#define LINUX_SC_NPROCESSORS_ONLN %d\n", _SC_NPROCESSORS_ONLN);
#endif
    return 0;
}
