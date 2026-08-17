/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Polykarpos Vergos
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

/*
 * SSIZE_MAX, like every macro exposed through <limits.h>, must be an
 * integer constant expression usable in #if preprocessing directives.
 * A cast-based definition ((__ssize_t)(__SIZE_MAX__ >> 1)) is not, and
 * broke code such as curl that tests it in #if (see issue #1358). The
 * #if below is the regression check: it fails to build if SSIZE_MAX is
 * not a valid constant expression.
 */

/* SSIZE_MAX is exposed under POSIX visibility. */
#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#if SSIZE_MAX < 32767
#error "SSIZE_MAX must be usable in #if and no smaller than _POSIX_SSIZE_MAX"
#endif

int
main(void)
{
    /* SSIZE_MAX is the maximum value of the signed type ssize_t, so it
       must be positive. */
    if (SSIZE_MAX <= 0)
        return 1;

    /* ssize_t differs from size_t only in signedness, so its maximum is
       exactly the top half of the size_t range. */
    if ((size_t)SSIZE_MAX != (SIZE_MAX >> 1))
        return 2;

    return 0;
}
