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
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* poll waits forever with a negative timeout and reports every entry */

#define check(condition, message)                      \
    do {                                               \
        if (!(condition)) {                            \
            printf("%s: %s\n", (message), #condition); \
            exit(1);                                   \
        }                                              \
    } while (0)

int
main(void)
{
    struct pollfd pfd[3];
    int           fds[2];
    int           status;
    pid_t         pid;
    char          c;

    check(pipe(fds) == 0, "pipe");

    /* Nothing ready: a short timeout expires */
    pfd[0].fd = fds[0];
    pfd[0].events = POLLIN;
    check(poll(pfd, 1, 20) == 0 && pfd[0].revents == 0, "poll times out");

    /*
     * With an infinite timeout, poll waits until the pipe is readable,
     * here after more than a second. One entry is ready, after one
     * with no events and one ignored.
     */
    pid = fork();
    check(pid >= 0, "fork");
    if (pid == 0) {
        struct timespec ts = { .tv_sec = 1, .tv_nsec = 500 * 1000 * 1000 };

        nanosleep(&ts, NULL);
        _exit(write(fds[1], "x", 1) == 1 ? 0 : 1);
    }
    pfd[0].fd = fds[1];
    pfd[0].events = POLLIN;
    pfd[1].fd = -1;
    pfd[1].events = POLLIN;
    pfd[2].fd = fds[0];
    pfd[2].events = POLLIN;
    check(poll(pfd, 3, -1) == 1, "poll waiting forever");
    check(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "writer");
    check(pfd[0].revents == 0 && pfd[1].revents == 0, "entries without events");
    check(pfd[2].revents == POLLIN, "ready entry");
    check(pfd[0].events == POLLIN && pfd[2].events == POLLIN, "events kept");

    check(read(fds[0], &c, 1) == 1 && c == 'x', "read");
    check(close(fds[1]) == 0, "close write end");
    pfd[2].revents = 0;
    check(poll(&pfd[2], 1, -1) == 1 && (pfd[2].revents & POLLHUP), "hang up reported");

    printf("success\n");
    return 0;
}
