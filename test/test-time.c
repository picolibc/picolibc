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

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE   700
#include <time.h>

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

int
main(void)
{
    char buf[256];

    /* reference every external symbol that time.t is supposed to declare */

    int  getdata_err_ret = getdate_err;
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

    struct tm *getdate_ret = getdate(buf);
    (void)getdate_ret;

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

    return 0;
}

#include <locale.h>

static locale_t
get_global_locale(void)
{
    return duplocale(LC_GLOBAL_LOCALE);
}
