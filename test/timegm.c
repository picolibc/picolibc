/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_TEST	1024

const struct _test {
	struct tm	tm;
	time_t		time;
} tests[NUM_TEST] = {
#ifndef GENERATE
	#include "timegm.h"
#endif
};

int main(void)
{
	unsigned i;
	int ret = 0;

#ifdef GENERATE
	for (i = 0; i < NUM_TEST; i++) {
		tests[i].time = random();
		gmtime_r(&tests[i].time, &tests[i].tm);
		printf ("    {\n");
		printf ("        .time = %ld,\n", tests[i].time);
		printf ("        .tm = {\n");
                printf ("            .tm_sec = %d,\n", tests[i].tm.tm_sec); /* Seconds (0-60) */
                printf ("            .tm_min = %d,\n", tests[i].tm.tm_min); /* Minutes (0-59) */
                printf ("            .tm_hour = %d,\n", tests[i].tm.tm_hour); /* Hours (0-23) */
                printf ("            .tm_mday = %d,\n", tests[i].tm.tm_mday); /* Day of the month (1-31) */
                printf ("            .tm_mon = %d,\n", tests[i].tm.tm_mon); /* Month (0-11) */
                printf ("            .tm_year = %d,\n", tests[i].tm.tm_year); /* Year - 1900 */
                printf ("            .tm_wday = %d,\n", tests[i].tm.tm_wday); /* Day of the week (0-6, Sunday = 0) */
                printf ("            .tm_yday = %d,\n", tests[i].tm.tm_yday); /* Day in the year (0-365, 1 Jan = 0) */
                printf ("            .tm_isdst = %d,\n", tests[i].tm.tm_isdst); /* Daylight saving time */
		printf ("        },\n");
		printf ("    },\n");
	}
#endif
	for (i = 0; i < NUM_TEST; i++) {
		struct tm	*ptm;
		time_t		time;

		/* gmtime */
		time = tests[i].time;
		ptm = gmtime(&time);
		if (ptm->tm_sec != tests[i].tm.tm_sec) {
			printf ("tm_sec: got %d want %d,\n", ptm->tm_sec, tests[i].tm.tm_sec); ret++;
		}
		if (ptm->tm_min != tests[i].tm.tm_min) {
			printf ("tm_min: got %d want %d,\n", ptm->tm_min, tests[i].tm.tm_min);
			ret++;
		}
		if (ptm->tm_hour != tests[i].tm.tm_hour) {
			printf ("tm_hour: got %d want %d,\n", ptm->tm_hour, tests[i].tm.tm_hour);
			ret++;
		}
		if (ptm->tm_mday != tests[i].tm.tm_mday) {
			printf ("tm_mday: got %d want %d,\n", ptm->tm_mday, tests[i].tm.tm_mday);
			ret++;
		}
		if (ptm->tm_mon != tests[i].tm.tm_mon) {
			printf ("tm_mon: got %d want %d,\n", ptm->tm_mon, tests[i].tm.tm_mon);
			ret++;
		}
		if (ptm->tm_year != tests[i].tm.tm_year) {
			printf ("tm_year: got %d want %d,\n", ptm->tm_year, tests[i].tm.tm_year);
			ret++;
		}
		if (ptm->tm_wday != tests[i].tm.tm_wday) {
			printf ("tm_wday: got %d want %d,\n", ptm->tm_wday, tests[i].tm.tm_wday);
			ret++;
		}
		if (ptm->tm_yday != tests[i].tm.tm_yday) {
			printf ("tm_yday: got %d want %d,\n", ptm->tm_yday, tests[i].tm.tm_yday);
			ret++;
		}
		if (ptm->tm_isdst != tests[i].tm.tm_isdst) {
			printf ("tm_isdst: got %d want %d,\n", ptm->tm_isdst, tests[i].tm.tm_isdst);
			ret++;
		}

                struct tm tmp = tests[i].tm;

		/* timegm */
		time = timegm(&tmp);
		if (time != tests[i].time) {
			printf("time: got %ld want %ld\n", (long) time, (long) tests[i].time);
			ret++;
		}
	}
	return ret;
}
