/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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
#include <assert.h>
#include <stdlib.h>

/*
 * Validate lock usage in libc by creating fake locks
 * to be used during testing
 */

#define _LOCK_T intptr_t*

intptr_t __lock___libc_recursive_mutex;

#define MAX_LOCKS 32

static intptr_t locks[MAX_LOCKS];
static uint8_t in_use[MAX_LOCKS];

/* Create a new dynamic non-recursive lock */
void __retarget_lock_init(_LOCK_T *lock)
{
        int lock_id = 0;

        for (lock_id = 0; lock_id < MAX_LOCKS; lock_id++)
                if (!in_use[lock_id]) {
                        in_use[lock_id] = 1;
                        *lock = &locks[lock_id];
                        **lock = 0;
                        return;
                }
        assert(0);
}

/* Create a new dynamic recursive lock */
void __retarget_lock_init_recursive(_LOCK_T *lock)
{
        __retarget_lock_init(lock);
}

/* Close dynamic non-recursive lock */
void __retarget_lock_close(_LOCK_T lock)
{
        assert(*lock == 0);
        int lock_id = lock - locks;
        assert(0 <= lock_id && lock_id < MAX_LOCKS);
        in_use[lock_id] = 0;
}

/* Close dynamic recursive lock */
void __retarget_lock_close_recursive(_LOCK_T lock)
{
        __retarget_lock_close(lock);
}

/* Acquiure non-recursive lock */
void __retarget_lock_acquire(_LOCK_T lock)
{
        assert(*lock == 0);
        *lock = 1;
}

/* Acquiure recursive lock */
void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
        assert(*lock >= 0);
        ++(*lock);
}

/* Release non-recursive lock */
void __retarget_lock_release(_LOCK_T lock)
{
        assert(*lock == 1);
        *lock = 0;
}

/* Release recursive lock */
void __retarget_lock_release_recursive(_LOCK_T lock)
{
        assert(*lock > 0);
        --(*lock);
}

__attribute__((destructor))
static void lock_validate(void)
{
        int i;
        for (i = 0; i < MAX_LOCKS; i++) {
                assert(locks[i] == 0);
        }

        assert(__lock___libc_recursive_mutex == 0);
}
