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

#include "local-linux.h"
#include <poll.h>

static inline short
picolibc_to_linux(short e)
{
    short r = 0;
    if (e & POLLIN)
        r |= LINUX_POLLIN;
    if (e & POLLPRI)
        r |= LINUX_POLLPRI;
    if (e & POLLOUT)
        r |= LINUX_POLLOUT;

    if (e & POLLERR)
        r |= LINUX_POLLERR;
    if (e & POLLHUP)
        r |= LINUX_POLLHUP;
    if (e & POLLNVAL)
        r |= LINUX_POLLNVAL;

    if (e & POLLRDNORM)
        r |= LINUX_POLLRDNORM;
    if (e & POLLRDBAND)
        r |= LINUX_POLLRDBAND;
    if (e & POLLWRNORM)
        r |= LINUX_POLLWRNORM;
    if (e & POLLWRBAND)
        r |= LINUX_POLLWRBAND;

    if (e & POLLMSG)
        r |= LINUX_POLLMSG;
    if (e & POLLREMOVE)
        r |= LINUX_POLLREMOVE;
    if (e & POLLRDHUP)
        r |= LINUX_POLLRDHUP;

    return r;
}

static inline short
linux_to_picolibc(short e)
{
    short r = 0;
    if (e & LINUX_POLLIN)
        r |= POLLIN;
    if (e & LINUX_POLLPRI)
        r |= POLLPRI;
    if (e & LINUX_POLLOUT)
        r |= POLLOUT;

    if (e & LINUX_POLLERR)
        r |= POLLERR;
    if (e & LINUX_POLLHUP)
        r |= POLLHUP;
    if (e & LINUX_POLLNVAL)
        r |= POLLNVAL;

    if (e & LINUX_POLLRDNORM)
        r |= POLLRDNORM;
    if (e & LINUX_POLLRDBAND)
        r |= POLLRDBAND;
    if (e & LINUX_POLLWRNORM)
        r |= POLLWRNORM;
    if (e & LINUX_POLLWRBAND)
        r |= POLLWRBAND;

    if (e & LINUX_POLLMSG)
        r |= POLLMSG;
    if (e & LINUX_POLLREMOVE)
        r |= POLLREMOVE;
    if (e & LINUX_POLLRDHUP)
        r |= POLLRDHUP;

    return r;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    nfds_t n;
    int    ret;

    /* map from picolibc to linux */
    for (n = 0; n < nfds; n++)
        fds[n].events = picolibc_to_linux(fds[n].events);

    ret = syscall(LINUX_SYS_poll, fds, nfds, timeout);

    /* revert the events changes */
    for (n = 0; n < nfds; n++)
        fds[n].events = linux_to_picolibc(fds[n].events);

    if (ret > 0) {
        for (n = 0; n < (nfds_t) ret; n++)
            fds[n].revents = linux_to_picolibc(fds[n].revents);
    }
    return ret;
}
