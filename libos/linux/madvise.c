/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "local-linux.h"
#include <sys/mman.h>

static int
_posix_advice_to_linux(int advice)
{
    switch (advice) {
    case POSIX_MADV_NORMAL:
        return LINUX_POSIX_MADV_NORMAL;
    case POSIX_MADV_RANDOM:
        return LINUX_POSIX_MADV_RANDOM;
    case POSIX_MADV_SEQUENTIAL:
        return LINUX_POSIX_MADV_SEQUENTIAL;
    case POSIX_MADV_WILLNEED:
        return LINUX_POSIX_MADV_WILLNEED;
    case POSIX_MADV_DONTNEED:
        return LINUX_POSIX_MADV_DONTNEED;
    default:
        return -1;
    }
}

/*
 * posix_madvise: POSIX.1-2008.
 *
 * Unlike most POSIX functions, posix_madvise() returns an error code
 * directly rather than setting errno and returning -1.  The syscall
 * trampoline sets errno and returns -1 on failure, so we read errno
 * back out and return it directly.
 */
int
posix_madvise(void *addr, size_t len, int advice)
{
    int linux_advice = _posix_advice_to_linux(advice);
    if (linux_advice < 0)
        return EINVAL;
    int ret = (int)syscall(LINUX_SYS_madvise, addr, len, linux_advice);
    if (ret < 0)
        return errno;
    return 0;
}
