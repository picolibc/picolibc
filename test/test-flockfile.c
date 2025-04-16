/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define check(condition, message) do {                  \
        if (!(condition)) {                             \
            printf("%s: %s\n", message, #condition);    \
            exit(1);                                    \
        }                                               \
    } while(0)

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FLOCK.TXT"
#endif

/* When __STDIO_LOCKING isn't defined, use flockfile/funlockfile */
#if defined(__TINY_STDIO) && !defined(__STDIO_LOCKING)
#define LOCK(out) flockfile(out)
#define UNLOCK(out) funlockfile(out)
#else
#define LOCK(out)
#define UNLOCK(out)
#endif

static FILE *out;

static void *write_func(void *arg)
{
    char *val = arg;
    char *v;
    int i;
    for(i = 0; i < 10; i++) {
        LOCK(out);
        fprintf(out, "%s", val);
        UNLOCK(out);
    }
    for(i = 0; i < 10; i++) {
        LOCK(out);
        fputs(val, out);
        UNLOCK(out);
    }
    for(i = 0; i < 10; i++) {
        flockfile(out);
        v = val;
        while (*v) {
            putc(*v++, out);
            usleep(rand() & 15);
        }
        funlockfile(out);
    }
    return NULL;
}

#define STRING  "hello, world\n"

int
start_thread(void *(*func)(void *), void *arg);

int
stop_thread(void);

#ifdef NO_NEWLIB
#include <pthread.h>

static pthread_t thread;

int
start_thread(void *(*func)(void *), void *arg)
{
        return pthread_create(&thread, NULL, func, arg);
}

int
stop_thread(void)
{
        return pthread_join(thread, NULL);
}
#endif

#ifdef __TINY_STDIO

static int fd;

static int myput(char c, FILE *file)
{
    (void) file;
    write(fd, &c, 1);
    usleep(rand() & 31);
}

static int myget(FILE *file)
{
    (void) file;
    uint8_t c;
    if (read(fd, &c, 1) <= 0)
        return _FDEV_EOF;
    return c;
}

static int myclose(FILE *file)
{
    (void) file;
    close(fd);
    fd = -1;
    return 0;
}

static struct __file_close myout = FDEV_SETUP_CLOSE(myput, myget, NULL, myclose, _FDEV_SETUP_WRITE);
#endif /* __TINY_STDIO */

int
main(void)
{
    int ret;
    int status = 0;
    char buf[1024];
    FILE *in;

#ifdef __SINGLE_THREAD
    printf("Single thread mode, test skipped\n");
    return(77);
#endif

#ifdef __TINY_STDIO
    fd = creat(TEST_FILE_NAME, 0666);
    if (fd < 0) {
        perror(TEST_FILE_NAME);
        exit(1);
    }
    out = &myout.file;
#else
    out = fopen(TEST_FILE_NAME, "w");
    if (!out) {
        perror(TEST_FILE_NAME);
        exit(1);
    }
    setvbuf(out, NULL, _IONBF, 0);
#endif
    srand(1);
    ret = start_thread(write_func, STRING);
    srand(2);
    if (ret) {
        perror("pthread_create");
        exit(1);
    }
    write_func(STRING);
    stop_thread();
    fclose(out);
    in = fopen(TEST_FILE_NAME, "r");
    if (!in) {
        perror(TEST_FILE_NAME);
        exit(1);
    }
    while (fgets(buf, sizeof(buf), in)) {
        if (strcmp(buf, STRING) != 0) {
            printf("line mangled: %s", buf);
            status = 1;
        }
    }
    fclose(in);
    remove(TEST_FILE_NAME);
    exit(status);
}
