/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   * Neither the name of Qualcomm Technologies, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Compile-time test for thread safety annotations.
 * When built with -Wthread-safety -Werror, this verifies that the
 * lock annotations on __retarget_lock_acquire/release are correct.
 */

#define _POSIX_C_SOURCE 199309L
#include <sys/lock.h>
#include <stdio.h>

#ifndef __SINGLE_THREAD

/*
 * Test balanced acquire/release of the libc recursive mutex.
 * This exercises the __acquire_capability/__release_capability
 * annotations on __retarget_lock_acquire_recursive and
 * __retarget_lock_release_recursive.
 */
static void
test_libc_lock(void)
{
    __LIBC_LOCK();
    __LIBC_UNLOCK();
}

#ifdef __STDIO_LOCKING

/*
 * Test balanced flockfile/funlockfile on a FILE (user API).
 */
static void
test_stdio_flockfile(FILE *f)
{
    flockfile(f);
    funlockfile(f);
}

#ifdef _LIBC
/*
 * Test balanced __flockfile/__funlockfile (internal API).
 */
static void
test_stdio_internal_flockfile(FILE *f)
{
    __flockfile(f);
    __funlockfile(f);
}
#endif /* _LIBC */

#endif /* __STDIO_LOCKING */

#endif /* __SINGLE_THREAD */

int
main(void)
{
#ifndef __SINGLE_THREAD
    test_libc_lock();
#ifdef __STDIO_LOCKING
    test_stdio_flockfile(stdout);
#ifdef _LIBC
    test_stdio_internal_flockfile(stdout);
#endif
#endif
#endif
    return 0;
}
