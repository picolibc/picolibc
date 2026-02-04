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

#define _GNU_SOURCE
#include <sys/cdefs.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#ifndef __weak_reference
#define __fallback_sysconf sysconf
#endif

long
__fallback_sysconf(int name)
{
    switch (name) {
    case _SC_AIO_LISTIO_MAX:
        return _POSIX_AIO_LISTIO_MAX;
    case _SC_AIO_MAX:
        return _POSIX_AIO_MAX;
    case _SC_AIO_PRIO_DELTA_MAX:
        return 0;
    case _SC_ARG_MAX:
        return _POSIX_ARG_MAX;
    case _SC_ATEXIT_MAX:
        return 32;
    case _SC_CHILD_MAX:
        return _POSIX_CHILD_MAX;
    case _SC_DELAYTIMER_MAX:
        return _POSIX_DELAYTIMER_MAX;
    case _SC_HOST_NAME_MAX:
        return _POSIX_HOST_NAME_MAX;
    case _SC_IOV_MAX:
        return _XOPEN_IOV_MAX;
    case _SC_LOGIN_NAME_MAX:
        return _POSIX_LOGIN_NAME_MAX;
    case _SC_MQ_OPEN_MAX:
        return _POSIX_MQ_OPEN_MAX;
    case _SC_MQ_PRIO_MAX:
        return _POSIX_MQ_PRIO_MAX;
    case _SC_OPEN_MAX:
        return _POSIX_OPEN_MAX;
    case _SC_PAGE_SIZE:
        return 4096;
#if 0
    case _SC_PTHREAD_DESTRUCTOR_ITERATIONS:
        return _POSIX_THREAD_DESTRUCTOR_ITERATIONS;
    case _SC_PTHREAD_KEYS_MAX:
        return _POSIX_THREAD_KEYS_MAX;
    case _SC_PTHREAD_STACK_MIN:
        return 0;
    case _SC_PTHREAD_THREADS_MAX:
        return _POSIX_THREAD_THREADS_MAX;
#endif
    case _SC_RTSIG_MAX:
        return _POSIX_RTSIG_MAX;
    case _SC_SEM_NSEMS_MAX:
        return _POSIX_SEM_NSEMS_MAX;
    case _SC_SEM_VALUE_MAX:
        return _POSIX_SEM_VALUE_MAX;
    case _SC_SIGQUEUE_MAX:
        return _POSIX_SIGQUEUE_MAX;
    case _SC_SS_REPL_MAX:
        return _POSIX_SS_REPL_MAX;
    case _SC_STREAM_MAX:
        return _POSIX_STREAM_MAX;
    case _SC_SYMLOOP_MAX:
        return _POSIX_SYMLOOP_MAX;
    case _SC_TIMER_MAX:
        return _POSIX_TIMER_MAX;
    case _SC_TRACE_EVENT_NAME_MAX:
        return _POSIX_TRACE_EVENT_NAME_MAX;
    case _SC_TRACE_NAME_MAX:
        return _POSIX_TRACE_NAME_MAX;
    case _SC_TRACE_SYS_MAX:
        return _POSIX_TRACE_SYS_MAX;
    case _SC_TRACE_USER_EVENT_MAX:
        return _POSIX_TRACE_USER_EVENT_MAX;
    case _SC_TTY_NAME_MAX:
        return _POSIX_TTY_NAME_MAX;
    case _SC_TZNAME_MAX:
        return _POSIX_TZNAME_MAX;
#if 0
        /* pathconf bits */
    case _PC_FILESIZEBITS:
        return sizeof(off_t) * 8;
    case _PC_LINK_MAX:
        return _POSIX_LINK_MAX;
    case _PC_MAX_CANON:
        return _POSIX_MAX_CANON;
    case _PC_MAX_INPUT:
        return _POSIX_MAX_INPUT;
    case _PC_NAME_MAX:
        return _POSIX_NAME_MAX;
    case _PC_PATH_MAX:
        return _POSIX_PATH_MAX;
    case _PC_PIPE_BUF:
        return _POSIX_PIPE_BUF;
    case _PC_POSIX_ALLOC_SIZE_MIN:
        return 0;
    case _PC_POSIX_REC_INCR_XFER_SIZE:
        return 1;
    case _PC_POSIX_REC_MAX_XFER_SIZE:
        return INT64_MAX;
    case _PC_POSIX_REC_MIN_XFER_SIZE:
        return 0;
    case _PC_POSIX_REC_XFER_ALIGN:
        return 1;
    case _PC_SYMLINK_MAX:
        return _POSIX_SYMLINK_MAX;
#endif
    case _SC_BC_BASE_MAX:
        return _POSIX2_BC_BASE_MAX;
    case _SC_BC_DIM_MAX:
        return _POSIX2_BC_DIM_MAX;
    case _SC_BC_SCALE_MAX:
        return _POSIX2_BC_SCALE_MAX;
    case _SC_BC_STRING_MAX:
        return _POSIX2_BC_STRING_MAX;
    case _SC_CHARCLASS_NAME_MAX:
        return _POSIX2_CHARCLASS_NAME_MAX;
    case _SC_COLL_WEIGHTS_MAX:
        return _POSIX2_COLL_WEIGHTS_MAX;
    case _SC_EXPR_NEST_MAX:
        return _POSIX2_EXPR_NEST_MAX;
    case _SC_LINE_MAX:
        return _POSIX2_LINE_MAX;
    case _SC_NGROUPS_MAX:
        return _POSIX_NGROUPS_MAX;
    case _SC_RE_DUP_MAX:
        return _POSIX_RE_DUP_MAX;
    default:
        errno = EINVAL;
        return -1;
    }
}

#ifdef __weak_reference
__weak_reference(__fallback_sysconf, sysconf);
#endif
