/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

/* test getdate, strptime and strftime functions */

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <locale.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "GETDATE.TXT"
#endif

struct date {
    int          line;
    struct tm    tm;
    int64_t      time;
    char * const string;
    char * const format;
};

#define TIME_TM_YEAR_BASE 1900

const struct date dates[] = {
    {
        .line = __LINE__,
        .tm = {
            .tm_mday  = 3,  // 3rd
            .tm_mon   = 1,  // of February
            .tm_year  = 1970 - TIME_TM_YEAR_BASE,
            .tm_wday  = 2,  // Tuesday
            .tm_yday  = 33,
        },
        .time = 2851200,
        .string = "Tue Feb 03 1970 00:00:00",
        .format = "%a %b %d %Y %H:%M:%S",
    },
    {
        .line = __LINE__,
        .tm = {
            .tm_mday  = 3,  // 3rd
            .tm_mon   = 1,  // of February
            .tm_year  = 1970 - TIME_TM_YEAR_BASE,
            .tm_wday  = 2,  // Tuesday
            .tm_yday  = 33,
        },
        .time = 2851200,
        .string = "Tuesday February 03 1970 00:00:00",
        .format = "%A %B %d %Y %H:%M:%S",
    },
    {
        .line = __LINE__,
        .tm = {
            // This is some arbitrary point in time with daylight saving time.
            .tm_mday  = 1,      // 1st
            .tm_mon   = 4,      // of May
            .tm_year  = 2021 - TIME_TM_YEAR_BASE,
            .tm_isdst = 1,      // Daylight saving time is in effect. That only affects local time.
            .tm_wday = 6,       // Sunday
            .tm_yday = 120,
        },
        .time = 1619827200,
        .string = "17 6 2021 00:00:00",
        .format = "%U %u %Y %H:%M:%S",
    },
    {
        .line = __LINE__,
        .tm = {
            // This is some arbitrary point in time with daylight saving time.
            .tm_mday  = 1,      // 1st
            .tm_mon   = 4,      // of May
            .tm_year  = 2021 - TIME_TM_YEAR_BASE,
            .tm_isdst = 1,      // Daylight saving time is in effect. That only affects local time.
            .tm_wday = 6,       // Sunday
            .tm_yday = 120,
        },
        .time = 1619827200,
        .string = "17 6 Sat May 01 2021 00:00:00",
        .format = "%V %u %a %b %d %Y %H:%M:%S",
    },
    {
        .line = __LINE__,
        .tm = {
            .tm_mday = 1,       // 1st day of the month
            .tm_mon = 0,        // January
            .tm_year = 1970 - TIME_TM_YEAR_BASE,
            .tm_wday = 4,       // Thursday
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .time = 0,
        .string = "00 4 Thu Jan 01 1970 00:00:00",
        .format = "%W %w %a %b %d %Y %H:%M:%S",
    },
    {
        .line = __LINE__,
        .tm = {
            .tm_sec  =  7,
            .tm_min  = 14,
            .tm_hour =  3,
            .tm_mon   = 0,      // January.
            .tm_mday  = 19,     // 19th
            .tm_year  = 2038 - TIME_TM_YEAR_BASE,
            .tm_isdst = 0,
            .tm_wday = 2,
            .tm_yday = 18,
        },
        .time = 0x7FFFFFFF,
        .string = "Tue Jan 19 2038 03:14:07",
        .format = "%a %b %d %Y %T",
    },

    /* Test some unusual format elements */
    {
        .line = __LINE__,
        .tm = {
            .tm_mday = 13,
            .tm_mon = 2,        // March
            .tm_year = 1970 - TIME_TM_YEAR_BASE,
            .tm_wday = 5,       // Friday
            .tm_yday = 71,
            .tm_isdst = 0,
        },
        .time = 0,
        .string = "19 70 10 5 12:00:00 AM",
        .format = "%C %y %W %w %r",
    },
    {
        .line = __LINE__,
        .tm = {
            .tm_hour = 15,
            .tm_min = 12,
            .tm_sec = 42,
            .tm_mday = 16,
            .tm_mon = 2,        // March
            .tm_year = 2013 - TIME_TM_YEAR_BASE,
            .tm_wday = 6,       // Saturday
            .tm_yday = 74,
            .tm_isdst = 0,
        },
        .time = 0,
        .string = "20 13 10 6 03:12:42 PM",
        .format = "%C %y %W %w %r",
    },
    /* tests requiring time_t larger than 32 bits */
    {
        .line = __LINE__,
        .tm = {
            .tm_sec  =  8,
            .tm_min  = 14,
            .tm_hour =  3,
            .tm_mon   = 0,      // January.
            .tm_mday  = 19,     // 19th
            .tm_year  = 2038 - TIME_TM_YEAR_BASE,
            .tm_isdst = 0,
            .tm_wday = 2,
            .tm_yday = 18,
        },
        .time = 0x80000000,
        .string = "Tue Jan 19 2038 03:14:08",
        .format = "%a %b %d %Y %T",
    },
    {
        .line = __LINE__,
        .tm = {
            .tm_mon   = 1,  // February.
            .tm_mday  = 29,
            .tm_year  = 2104 - TIME_TM_YEAR_BASE,
            .tm_isdst = 0,
            .tm_wday  = 5,
            .tm_yday  = 59,
        },
        .time = 4233686400,
        .string = "Fri Feb 29 2104 00:00:00",
        .format = "%a %b %d %Y %H:%M:%S",
    },
};

#define NUM_DATES (sizeof(dates) / sizeof(dates[0]))

static bool
tm_eq(const struct tm *a, const struct tm *b)
{
    if (a->tm_sec != b->tm_sec)
        return false;
    if (a->tm_min != b->tm_min)
        return false;
    if (a->tm_hour != b->tm_hour)
        return false;
    if (a->tm_mday != b->tm_mday)
        return false;
    if (a->tm_mon != b->tm_mon)
        return false;
    if (a->tm_year != b->tm_year)
        return false;
    if (a->tm_wday != b->tm_wday)
        return false;
    if (a->tm_yday != b->tm_yday)
        return false;
    /* these functions can't compute this field */
    /*
        if (a->tm_isdst != b->tm_isdst)
            return false;
    */
    return true;
}

#define TIME_STR 128

static bool
asctime_trim(const struct tm *tm, char buf[TIME_STR])
{
    char *a = buf;

    if (!strftime(buf, TIME_STR, "%a %b %e %H:%M:%S %Y wday %w yday %j", tm))
        return false;

    while (*a) {
        if (*a == '\n') {
            *a = '\0';
            break;
        }
        a++;
    }
    return true;
}

static int
test_getdate(const struct date *date)
{
    struct tm *got;
    char       gotbuf[TIME_STR], expectbuf[TIME_STR];
    int        ret = 0;

#ifndef TESTS_ENABLE_POSIX_IO
    return 1;
#endif
    got = getdate(date->string);
    if (!got) {
        printf("getdate error %d\n", getdate_err);
    }
    if (got && tm_eq(got, &date->tm)) {
        ret = 1;
    } else {
        if (!got || !asctime_trim(got, gotbuf))
            strcpy(gotbuf, "invalid");
        if (!asctime_trim(&date->tm, expectbuf))
            strcpy(expectbuf, "invalid");
        printf("%s:%d: getdate(\"%s\") = '%s' expect '%s'\n", __FILE__, date->line, date->string,
               gotbuf, expectbuf);
    }
    if (got)
        memset(got, 0, sizeof(struct tm));
    return ret;
}

static int
test_strptime(const struct date *date)
{
    struct tm got;
    char      gotbuf[TIME_STR], expectbuf[TIME_STR];
    char     *ret;

    memset(&got, 0, sizeof(struct tm));
    ret = strptime(date->string, date->format, &got);
    if (!ret)
        printf("strptime error\n");
    if (!ret || !tm_eq(&got, &date->tm)) {
        if (!ret || !asctime_trim(&got, gotbuf))
            strcpy(gotbuf, "invalid");
        if (!asctime_trim(&date->tm, expectbuf))
            strcpy(expectbuf, "invalid");
        printf("%s:%d: strptime(\"%s\", \"%s\") = '%s' expect '%s'\n", __FILE__, date->line,
               date->string, date->format, gotbuf, expectbuf);
        return 0;
    }
    return 1;
}

static int
test_strftime(const struct date *date)
{
    struct tm got;
    char      gotbuf[TIME_STR], expectbuf[TIME_STR];
    size_t    ret;

    memset(&got, 0, sizeof(struct tm));
    ret = strftime(gotbuf, TIME_STR, date->format, &date->tm);
    if (ret != strlen(date->string) || strcmp(gotbuf, date->string) != 0) {
        if (!asctime_trim(&date->tm, expectbuf))
            strcpy(expectbuf, "invalid");
        printf("%s:%d: strftime(%s, %s) = '%s' expect '%s'\n", __FILE__, date->line, expectbuf,
               date->format, gotbuf, date->string);
        return 0;
    }
    return 1;
}

int
main(void)
{
    size_t i;
    int    ret = 0;

    (void)setlocale(LC_ALL, "C.UTF-8");

#ifdef TESTS_ENABLE_POSIX_IO
    char *datemsk_name = TEST_FILE_NAME;
    FILE *datemsk;
    datemsk = fopen(datemsk_name, "w");
    if (!datemsk) {
        perror(datemsk_name);
        return 1;
    }
    for (i = 0; i < NUM_DATES; i++) {
        if (fprintf(datemsk, "%s\n", dates[i].format) < 0) {
            perror(datemsk_name);
            remove(datemsk_name);
            return 1;
        }
    }

    if (fclose(datemsk) != 0) {
        perror(datemsk_name);
        remove(datemsk_name);
        return 1;
    }

    setenv("DATEMSK", datemsk_name, 1);
#endif

    for (i = 0; i < NUM_DATES; i++) {
        if (sizeof(time_t) == 4 && dates[i].time > (int64_t)0x7fffffff)
            continue;
        if (!test_getdate(&dates[i]))
            ret = 1;
        if (!test_strptime(&dates[i]))
            ret = 1;
        if (!test_strftime(&dates[i]))
            ret = 1;
    }

#ifdef TESTS_ENABLE_POSIX_IO
    remove(datemsk_name);
#endif
    return ret;
}
