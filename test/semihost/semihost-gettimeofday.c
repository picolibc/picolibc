/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>

int
main(void)
{
        int             loop;
        struct timeval  prev, cur;
        int             ret;

        ret = gettimeofday(&prev, NULL);
        if (ret < 0) {
                printf("gettimeofday return %d\n", ret);
                exit(2);
        }
        if (prev.tv_sec < 1600000000LL) {
                printf("gettimeofday return value before 2020-9-13 %lld\n",
                       (long long) prev.tv_sec);
                exit(2);
        }
	for (loop = 0; loop < 10000; loop++) {
                ret = gettimeofday(&cur, NULL);
                if (ret < 0) {
                        printf("gettimeofday return %d\n", ret);
                        exit(2);
                }
                if (cur.tv_sec > prev.tv_sec + 1000) {
                        printf("gettimeofday: seconds changed by %lld\n",
                               (long long) (cur.tv_sec - prev.tv_sec));
                        exit(2);
                }
                if (cur.tv_sec > prev.tv_sec ||
                    (cur.tv_sec == prev.tv_sec &&
                     cur.tv_usec > prev.tv_usec))
                {
                        printf("gettimeofday: ok\n");
                        exit(0);
                }
                if (cur.tv_sec < prev.tv_sec ||
                    (cur.tv_sec == prev.tv_sec &&
                     cur.tv_usec < prev.tv_usec))
                {
                        printf("gettimeofday: clock went backwards\n");
                        exit(2);
		}
	}
	printf("gettimeofday: clock never changed\n");
	exit(1);
}
