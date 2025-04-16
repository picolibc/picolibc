/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static bool
near(double got, double target, double close)
{
	if (got < target - close)
		return false;
	if (got > target + close)
		return false;
	return true;
}

static double
random_double(void)
{
    return (double) random() / 0x7fffffffL;
}

static double
rand_double(void)
{
    return (double) rand() / RAND_MAX;
}

static const struct {
        char    *name;
        double  (*func)(void);
        double  mean_err;
        double  stddev_err;
} funcs[] = {
        {
                .name = "drand48",
                .func = drand48,
                .mean_err = 0.01,
                .stddev_err = 0.02,
        },
        {
                .name = "random",
                .func = random_double,
                .mean_err = 0.01,
                .stddev_err = 0.02,
        },
        {
                .name = "rand",
                .func = rand_double,
                .mean_err = 0.01,
                .stddev_err = 0.02,
        },
};

int
main(void)
{
	int32_t i;
	int ret = 0;
        unsigned f;

        for (f = 0; f < sizeof(funcs) / sizeof(funcs[0]); f++) {
                double s1 = 0;
                double s2 = 0;
                int pass = 1;
#define N	1000
                printf ("Testing %s...", funcs[f].name);
                fflush(stdout);
                for (i = 0; i < N; i++) {
                        double d = funcs[f].func();
                        s1 += d;
                        s2 += d*d;
                }
                double mean = s1 / N;
                double stddev = sqrt((N * s2 - s1*s1) / ((double) N * ((double) N - 1)));
                if (!near(mean, .5, funcs[f].mean_err)) {
                        printf(" bad mean %g", mean);
                        ret = 1;
                        pass = 0;
                }
                if (!near(stddev, .28, funcs[f].stddev_err)) {
                        printf(" bad stddev %g", stddev);
                        ret = 2;
                        pass = 0;
                }
                if (pass)
                        printf(" pass");
                printf("\n");
        }
	return ret;
}
