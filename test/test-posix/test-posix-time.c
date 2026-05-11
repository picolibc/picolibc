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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            exit(1);                                 \
        }                                            \
    } while (0)

/*
 * time() is implemented in terms of gettimeofday().
 */
#ifdef __PICOLIBC__
__typeof(gettimeofday) __fake_gettimeofday __weak;
#define not_fake_gettimeofday ((gettimeofday) != __fake_gettimeofday)
#else
#define not_fake_gettimeofday 1
#endif

/* 2020-09-13 00:00:00 UTC — a known past date */
#define NOT_BEFORE ((time_t)1600000000)

int
main(void)
{
    time_t stored = 0;
    time_t t1, t2;

    /* time(NULL) must not return -1 */
    t1 = time(NULL);
    check(t1 != (time_t)-1, "time(NULL) failed");

    /* time(&stored) must store the same value it returns */
    t2 = time(&stored);
    check(t2 != (time_t)-1, "time(&stored) failed");
    check(stored == t2, "time(&stored) did not store return value");

    if (not_fake_gettimeofday) {
        /* Wall-clock time must be after our known past date */
        check(t1 >= NOT_BEFORE, "time() returned value before 2020-09-13");

        /* A second call must not go backwards */
        t2 = time(NULL);
        check(t2 >= t1, "time() went backwards");

        printf("time() returned %lld, test passed\n", (long long)t1);
    } else {
        printf("fake gettimeofday present, skipping wall-clock checks\n");
    }

    exit(0);
}
