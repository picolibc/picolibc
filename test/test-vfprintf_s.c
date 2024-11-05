/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024, Synopsys Inc.
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

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#define MAX_ERROR_MSG 100

char handler_msg[MAX_ERROR_MSG] = "";

static void
custom_constraint_handler(const char *restrict msg, void *restrict ptr,
                          errno_t error)
{
    (void)ptr;
    (void)error;
    strcpy(handler_msg, msg);
}

#define TEST_RES(cond, msg, handler_res, test_id)                              \
    if ((!(cond)) || (handler_res == 1)) {                                     \
        printf("Test %d Failed: %s\n", test_id, msg);                          \
        return 1;                                                              \
    } else {                                                                   \
        printf("Test %d Passed: %s\n", test_id, msg);                          \
    }

static int
test_handler_called(int handler_called, char *expected_msg, int test_id)
{
    int ret = 0;
    if (handler_called == 0) {
        (void)expected_msg;
        if (handler_msg[0] != '\0') {
            printf(
                "ERROR: Custom constraint handler called without error detiction!\n");
            printf("Test %d Failed: Error msg is incorrect\n", test_id);
            ret = 1;
        }
    } else {
        if (handler_msg[0] == '\0') {
            (void)expected_msg;
            printf("ERROR: Custom constraint handler not called\n");
            printf("Test %d Failed: Error msg is incorrect\n", test_id);
            ret = 1;
        } else {
            if (strcmp(expected_msg, handler_msg) != 0) {
                printf(
                    "ERROR: Custom constraint handler called with incorrect msg: %s\n",
                    handler_msg);
                printf("Test %d Failed: Error msg is incorrect\n", test_id);
                ret = 1;
            } else {
                (void)expected_msg;
                printf(
                    "Custom constraint handler called with correct msg: %s\n",
                    handler_msg);
                handler_msg[0] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

static int
test_vfprintf_s(FILE *restrict stream, const char *restrict fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vfprintf_s(stream, fmt, args);
    va_end(args);

    return ret;
}

static char buf[512];

int
main(void)
{
    int test_id = 0;
    int handler_res = 0;
    errno_t res;
    FILE *f;
    f = fmemopen(buf, sizeof(buf), "w");

    if (!f) {
        perror("Error opening file");
        return 1;
    }

    set_constraint_handler_s(custom_constraint_handler);

    // Test case 1: Normal formatting
    test_id++;
    res = test_vfprintf_s(f, "Hello, %s!", "world");
    handler_res = test_handler_called(0, "", test_id);
    TEST_RES(res == (int)strlen("Hello, world!"), "Normal formatting",
             handler_res, test_id);

    // Test case 2: Formatting with Null stream
    test_id++;
    res = test_vfprintf_s(NULL, "Hello, %s!", "world");
    handler_res = test_handler_called(1, "output stream is null", test_id);
    TEST_RES(res == -1, "Formatting with Null stream", handler_res, test_id);

    // Test case 3: Formatting with Null format string
    test_id++;
    res = test_vfprintf_s(f, NULL, "world");
    handler_res = test_handler_called(1, "null format string", test_id);
    TEST_RES(res == -1, "Formatting with Null format string", handler_res,
             test_id);

    printf("All vfprintf_s tests passed!\n");
    fclose(f);
    return 0;
}
