/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
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
