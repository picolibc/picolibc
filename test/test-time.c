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

/* Validate contents of time.h */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE   700
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

clock_t          clock_value = CLOCKS_PER_SEC;
size_t           size_value;
time_t           time_value;

clockid_t        clockid_value;
timer_t          timer_value;

locale_t         locale_value;

pid_t            pid_value;

struct sigevent *sigevent_pointer = NULL;

struct tm        tm_value = {
           .tm_sec = 0,
           .tm_min = 0,
           .tm_hour = 0,
           .tm_mday = 1,
           .tm_mon = 0,
           .tm_year = 100,
           .tm_wday = 6,
           .tm_yday = 0,
           .tm_isdst = 0,
};

struct timespec timespec_value = {
    .tv_sec = 0,
    .tv_nsec = 0,
};

struct itimerspec itimerspec_value = {
    .it_interval = {
        .tv_sec = 0,
        .tv_nsec = 0,
    },
    .it_value = {
        .tv_sec = 0,
        .tv_nsec = 0,
    },
};

#ifdef _POSIX_MONOTONIC_CLOCK
clockid_t monotonic_value = CLOCK_MONOTONIC;
#endif
#ifdef _POSIX_CPUTIME
clockid_t process_cputime_id_value = CLOCK_PROCESS_CPUTIME_ID;
#endif
clockid_t realtime_value = CLOCK_REALTIME;
#ifdef _POSIX_THREAD_CPUTIME
clockid_t thread_cputime_id_value = CLOCK_THREAD_CPUTIME_ID;
#endif

int             timer_abstime_value = TIMER_ABSTIME;

static locale_t get_global_locale(void);

static const struct {
    bool      tm_valid;
    bool      tm_noncanon;
    struct tm tm;
    struct tm want_tm;
    time_t    time;
} tests[] = {
    {
        .tm = {
            .tm_year = 1,
            .tm_mon = 11,
            .tm_mday = 13,
            .tm_hour = 20,
            .tm_min = 45,
            .tm_sec = 52,
            .tm_wday = 5,
            .tm_yday = 346,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = -2147483648,
    },
    {
        .tm = {
            .tm_year = 138,
            .tm_mon = 0,
            .tm_mday = 19,
            .tm_hour = 3,
            .tm_min = 14,
            .tm_sec = 07,
            .tm_wday = 2,
            .tm_yday = 18,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 2147483647,
    },
    {
        .tm = {
            .tm_year = -1,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 0,
            .tm_yday = 364,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = -2209075200,
    },
    {
        .tm = {
            .tm_year = 10001,
            .tm_mon = 1,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 5,
            .tm_yday = 31,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 313394745600,
    },
    {
        .tm = {
            .tm_year = 20001,
            .tm_mon = 1,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 5,
            .tm_yday = 31,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 628964265600,
    },
    {
        .tm = {
            .tm_year = 20071,
            .tm_mon = 1,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 1,
            .tm_yday = 31,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 631173254400,
    },
    {
        .tm = {
            .tm_year = 20070,
            .tm_mon = 1,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 0,
            .tm_yday = 31,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 631141718400,
    },
    {
        .tm = {
            .tm_year = 20069,
            .tm_mon = 1,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 6,
            .tm_yday = 31,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 631110182400,
    },
#if INT_MAX >= 2147483647
    {
        .tm = {
            .tm_year = 2147483647,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 23,
            .tm_min = 59,
            .tm_sec = 59,
            .tm_wday = 3,
            .tm_yday = 364,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 67768036191676799,
    },
    {
        .tm = {
            .tm_year = 2147483647,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 23,
            .tm_min = 59,
            .tm_sec = 60,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = false,
        .tm_noncanon = true,
        .time = 67768036191676800,
    },
#endif
    {
        .tm = {
            .tm_year = 32767,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 23,
            .tm_min = 59,
            .tm_sec = 59,
            .tm_wday = 2,
            .tm_yday = 364,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = 1031849193599,
    },
    {
        .tm = {
            .tm_year = 32767,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 23,
            .tm_min = 59,
            .tm_sec = 60,
            .tm_wday = 3,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
#if INT_MAX >= 32768
        .want_tm = {
            .tm_year = 32768,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 3,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = true,
#else
        .tm_valid = false,
#endif
        .tm_noncanon = true,
        .time = 1031849193600,
    },
#if INT_MIN <= -2147483648
    {
        .tm = {
            .tm_year = -2147483648,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 4,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = -67768040609740800,
    },
    {
        .tm = {
            .tm_year = -2147483648,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = -1,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = false,
        .tm_noncanon = false,
        .time = -67768040609740801,
    },
    {
        .tm = {
            .tm_year = -2147483648,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 1,
            .tm_sec = -1,
            .tm_wday = 0,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .want_tm = {
            .tm_year = -2147483648,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 59,
            .tm_wday = 4,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = true,
        .time = -67768040609740741,
    },
#endif
    {
        .tm = {
            .tm_year = -32768,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 0,
            .tm_wday = 5,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = false,
        .time = -1036267257600,
    },
    {
        .tm = {
            .tm_year = -32768,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = -1,
            .tm_wday = 5,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
#if INT_MIN < -32768
        .want_tm = {
            .tm_year = -32769,
            .tm_mon = 11,
            .tm_mday = 31,
            .tm_hour = 23,
            .tm_min = 59,
            .tm_sec = 59,
            .tm_wday = 4,
            .tm_yday = 364,
            .tm_isdst = 0,
        },
        .tm_valid = true,
#endif
        .tm_noncanon = true,
        .time = -1036267257601,
    },
    {
        .tm = {
            .tm_year = -32768,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 1,
            .tm_sec = -1,
            .tm_wday = 6,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .want_tm = {
            .tm_year = -32768,
            .tm_mon = 0,
            .tm_mday = 1,
            .tm_hour = 0,
            .tm_min = 0,
            .tm_sec = 59,
            .tm_wday = 5,
            .tm_yday = 0,
            .tm_isdst = 0,
        },
        .tm_valid = true,
        .tm_noncanon = true,
        .time = -1036267257541,
    },
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))

int
main(void)
{
    char buf[256];
    int  ret = 0;

    putenv("TZ=UTC");
    if (sizeof(time_t) > 4) {
        size_t t;

        for (t = 0; t < NTESTS; t++) {
            struct tm        local_tm = tests[t].tm;
            struct tm       *tm = &local_tm;
            time_t           got_time;
            time_t           want_time = tests[t].tm_valid ? tests[t].time : (time_t)-1;
            struct tm       *got_tm;
            const struct tm *want_tm = tests[t].tm_valid
                ? (tests[t].tm_noncanon ? &tests[t].want_tm : &tests[t].tm)
                : NULL;

            /* do this test first so if it breaks tm, we'll find out */
            got_time = timegm(tm);

#ifdef GENERATE
            printf("    {\n");
            printf("        .tm = {\n");
            printf("            .tm_year = %d,\n", tm->tm_year);
            printf("            .tm_mon = %d,\n", tm->tm_mon);
            printf("            .tm_mday = %d,\n", tm->tm_mday);
            printf("            .tm_hour = %d,\n", tm->tm_hour);
            printf("            .tm_min = %d,\n", tm->tm_min);
            printf("            .tm_sec = %d,\n", tm->tm_sec);
            printf("            .tm_wday = %d,\n", tm->tm_wday);
            printf("            .tm_yday = %d,\n", tm->tm_yday);
            printf("            .tm_isdst = %d,\n", tm->tm_isdst);
            printf("        },\n");
            if (tests[t].tm_noncanon) {
                printf("        .want_tm = {\n");
                printf("            .tm_year = %d,\n", tests[t].want_tm.tm_year);
                printf("            .tm_mon = %d,\n", tests[t].want_tm.tm_mon);
                printf("            .tm_mday = %d,\n", tests[t].want_tm.tm_mday);
                printf("            .tm_hour = %d,\n", tests[t].want_tm.tm_hour);
                printf("            .tm_min = %d,\n", tests[t].want_tm.tm_min);
                printf("            .tm_sec = %d,\n", tests[t].want_tm.tm_sec);
                printf("            .tm_wday = %d,\n", tests[t].want_tm.tm_wday);
                printf("            .tm_yday = %d,\n", tests[t].want_tm.tm_yday);
                printf("            .tm_isdst = %d,\n", tests[t].want_tm.tm_isdst);
                printf("        },\n");
            }
            printf("        .tm_valid = %s,\n", tests[t].tm_valid ? "true" : "false");
            printf("        .tm_noncanon = %s,\n", tests[t].tm_noncanon ? "true" : "false");
            printf("        .time = %lld,\n", (long long)tests[t].time);
            printf("    },\n");

            continue;
#endif

            if (want_tm == &tests[t].tm) {
                got_tm = tm;
                if (got_tm->tm_year != want_tm->tm_year || got_tm->tm_mon != want_tm->tm_mon
                    || got_tm->tm_mday != want_tm->tm_mday || got_tm->tm_hour != want_tm->tm_hour
                    || got_tm->tm_min != want_tm->tm_min || got_tm->tm_sec != want_tm->tm_sec
                    || got_tm->tm_wday != want_tm->tm_wday || got_tm->tm_yday != want_tm->tm_yday
                    || got_tm->tm_isdst != want_tm->tm_isdst) {
                    printf("timegm mangled tm:\n");
                    printf("   want %d/%d/%d %02d:%02d:%02d wday %d yday %d isdst %d\n",
                           want_tm->tm_year, want_tm->tm_mon, want_tm->tm_mday, want_tm->tm_hour,
                           want_tm->tm_min, want_tm->tm_sec, want_tm->tm_wday, want_tm->tm_yday,
                           want_tm->tm_isdst);
                    printf("    got %d/%d/%d %02d:%02d:%02d wday %d yday %d isdst %d\n",
                           got_tm->tm_year, got_tm->tm_mon, got_tm->tm_mday, got_tm->tm_hour,
                           got_tm->tm_min, got_tm->tm_sec, got_tm->tm_wday, got_tm->tm_yday,
                           got_tm->tm_isdst);
                    ret = 1;
                }
            }

            if (got_time != want_time) {
                printf("timegm %d/%d/%d %02d:%02d:%02d want %lld got %lld\n", tm->tm_year,
                       tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
                       (long long)want_time, (long long)got_time);
                ret = 1;
            }

            got_tm = gmtime(&tests[t].time);

            if ((got_tm && !want_tm) || (!got_tm && want_tm)
                || (got_tm && want_tm
                    && (got_tm->tm_year != want_tm->tm_year || got_tm->tm_mon != want_tm->tm_mon
                        || got_tm->tm_mday != want_tm->tm_mday
                        || got_tm->tm_hour != want_tm->tm_hour || got_tm->tm_min != want_tm->tm_min
                        || got_tm->tm_sec != want_tm->tm_sec || got_tm->tm_wday != want_tm->tm_wday
                        || got_tm->tm_yday != want_tm->tm_yday
                        || got_tm->tm_isdst != want_tm->tm_isdst))) {
                printf("gmtime %lld\n", (long long)tests[t].time);
                if (want_tm)
                    printf("   want %d/%d/%d %02d:%02d:%02d wday %d yday %d isdst %d\n",
                           want_tm->tm_year, want_tm->tm_mon, want_tm->tm_mday, want_tm->tm_hour,
                           want_tm->tm_min, want_tm->tm_sec, want_tm->tm_wday, want_tm->tm_yday,
                           want_tm->tm_isdst);
                else
                    printf("   want nothing\n");
                if (got_tm)
                    printf("    got %d/%d/%d %02d:%02d:%02d wday %d yday %d isdst %d\n",
                           got_tm->tm_year, got_tm->tm_mon, got_tm->tm_mday, got_tm->tm_hour,
                           got_tm->tm_min, got_tm->tm_sec, got_tm->tm_wday, got_tm->tm_yday,
                           got_tm->tm_isdst);
                else
                    printf("    got nothing\n");
                ret = 1;
            }
        }
    }

    /* reference every external symbol that time.t is supposed to declare */

    int getdata_err_ret = getdate_err;
    (void)getdata_err_ret;

    char *asctime_ret = asctime(&tm_value);
    (void)asctime_ret;

    char *asctime_r_ret = asctime_r(&tm_value, buf);
    (void)asctime_r_ret;

#ifndef __PICOLIBC__
    clock_t clock_ret = clock();
    (void)clock_ret;

#ifdef _POSIX_CPUTIME
    int clock_getcpuclockid_ret = clock_getcpuclockid(pid_value, &clockid_value);
    (void)clock_getcpuclockid_ret;
#endif

    int clock_getres_ret = clock_getres(clockid_value, &timespec_value);
    (void)clock_getres_ret;

    struct timespec clock_gettime_timespec_ret;
    int             clock_gettime_ret = clock_gettime(realtime_value, &clock_gettime_timespec_ret);
    (void)clock_gettime_ret;

    struct timespec clock_nanosleep_timespec_ret;
    int             clock_nanosleep_ret
        = clock_nanosleep(realtime_value, 0, &timespec_value, &clock_nanosleep_timespec_ret);
    (void)clock_nanosleep_ret;

    int clock_settime_ret = clock_settime(realtime_value, &timespec_value);
    (void)clock_settime_ret;
#endif /* !__PICOLIBC__ */

    char *ctime_ret = ctime(&time_value);
    (void)ctime_ret;

    char *ctime_r_ret = ctime_r(&time_value, buf);
    (void)ctime_r_ret;

    double difftime_ret = difftime(time_value, time_value);
    (void)difftime_ret;

#if 0
    struct tm *getdate_ret = getdate(buf);
    (void)getdate_ret;
#endif

    struct tm *gmtime_ret = gmtime(&time_value);
    (void)gmtime_ret;

    struct tm  gmtime_r_tm_ret;
    struct tm *gmtime_r_ret = gmtime_r(&time_value, &gmtime_r_tm_ret);
    (void)gmtime_r_ret;

    struct tm *localtime_ret = localtime(&time_value);
    (void)localtime_ret;

    struct tm  localtime_r_tm_ret;
    struct tm *localtime_r_ret = localtime_r(&time_value, &localtime_r_tm_ret);
    (void)localtime_r_ret;

    time_t mktime_ret = mktime(&tm_value);
    (void)mktime_ret;

#ifndef __PICOLIBC__
    struct timespec nanosleep_timespec_ret;
    int             nanosleep_ret = nanosleep(&timespec_value, &nanosleep_timespec_ret);
    (void)nanosleep_ret;
#endif

    size_t strftime_ret = strftime(buf, sizeof(buf), "%c", &tm_value);
    (void)strftime_ret;

    size_t strftime_l_ret = strftime_l(buf, sizeof(buf), "%c", &tm_value, get_global_locale());
    (void)strftime_l_ret;

    struct tm strptime_tm_ret;
    char     *strptime_ret = strptime("", "", &strptime_tm_ret);
    (void)strptime_ret;

#ifndef __PICOLIBC__
    time_t time_ret = time(&time_value);
    (void)time_ret;

    timer_t timer_create_timer_ret;
    int timer_create_ret = timer_create(clockid_value, sigevent_pointer, &timer_create_timer_ret);
    (void)timer_create_ret;

    int timer_delete_ret = timer_delete(timer_value);
    (void)timer_delete_ret;

    int timer_getoverrun_ret = timer_getoverrun(timer_value);
    (void)timer_getoverrun_ret;

    struct itimerspec timer_gettime_itimerspec_ret;
    int               timer_gettime_ret = timer_gettime(timer_value, &timer_gettime_itimerspec_ret);
    (void)timer_gettime_ret;

    struct itimerspec timer_settime_itimerspec_ret;
    int               timer_settime_ret
        = timer_settime(timer_value, 0, &itimerspec_value, &timer_settime_itimerspec_ret);
    (void)timer_settime_ret;
#endif

    tzset();

    return ret;
}

#include <locale.h>

static locale_t
get_global_locale(void)
{
    return duplocale(LC_GLOBAL_LOCALE);
}
