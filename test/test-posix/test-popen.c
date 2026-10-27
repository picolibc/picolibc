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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FOPEN.TXT"
#endif

#define TEST_STRING "hello, world\n"

static void
check_popen(int mode, size_t size, int code)
{
    char  command[32];
    FILE *p;
    int   status;
    int   child_status;
    pid_t child;

    snprintf(command, sizeof(command), "exit %d", code);
    p = popen(command, "w");
    assert(p != NULL);
    assert(setvbuf(p, NULL, mode, size) == 0);

    status = pclose(p);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == code);

    errno = 0;
    child = waitpid(-1, &child_status, WNOHANG);
    assert(child == -1);
    assert(errno == ECHILD);
}

static void
check_waitpid_failure(void)
{
    FILE *p;
    int   status;
    pid_t child;

    p = popen("exit 7", "w");
    assert(p != NULL);

    child = waitpid(-1, &status, 0);
    assert(child > 0);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 7);

    errno = 0;
    assert(pclose(p) == -1);
    assert(errno == ECHILD);
}

int
main(void)
{
    FILE *out;
    FILE *in;
    char  buf[128];
    char *ret;

    out = fopen(TEST_FILE_NAME, "w");
    assert(out != NULL);
    fputs(TEST_STRING, out);
    fclose(out);

    in = popen("cat " TEST_FILE_NAME, "r");
    assert(in != NULL);
    ret = fgets(buf, sizeof(buf), in);
    assert(ret == buf);
    assert(strcmp(buf, TEST_STRING) == 0);
    assert(pclose(in) == 0);

    check_popen(_IOFBF, 1, 3);
    check_popen(_IONBF, 0, 4);
    check_popen(_IOFBF, 4096, 5);
    check_waitpid_failure();
    return 0;
}
