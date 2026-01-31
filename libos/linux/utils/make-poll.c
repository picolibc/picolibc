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
#include <poll.h>
#include <stdio.h>

int
main(void)
{
#ifdef POLLIN
    printf("#define LINUX_POLLIN %d\n", POLLIN);
#endif
#ifdef POLLPRI
    printf("#define LINUX_POLLPRI %d\n", POLLPRI);
#endif
#ifdef POLLOUT
    printf("#define LINUX_POLLOUT %d\n", POLLOUT);
#endif
#ifdef POLLERR
    printf("#define LINUX_POLLERR %d\n", POLLERR);
#endif
#ifdef POLLHUP
    printf("#define LINUX_POLLHUP %d\n", POLLHUP);
#endif
#ifdef POLLNVAL
    printf("#define LINUX_POLLNVAL %d\n", POLLNVAL);
#endif
#ifdef POLLRDNORM
    printf("#define LINUX_POLLRDNORM %d\n", POLLRDNORM);
#endif
#ifdef POLLRDBAND
    printf("#define LINUX_POLLRDBAND %d\n", POLLRDBAND);
#endif
#ifdef POLLWRNORM
    printf("#define LINUX_POLLWRNORM %d\n", POLLWRNORM);
#endif
#ifdef POLLWRBAND
    printf("#define LINUX_POLLWRBAND %d\n", POLLWRBAND);
#endif
#ifdef POLLMSG
    printf("#define LINUX_POLLMSG %d\n", POLLMSG);
#endif
#ifdef POLLREMOVE
    printf("#define LINUX_POLLREMOVE %d\n", POLLREMOVE);
#endif
#ifdef POLLRDHUP
    printf("#define LINUX_POLLRDHUP %d\n", POLLRDHUP);
#endif
    return 0;
}
