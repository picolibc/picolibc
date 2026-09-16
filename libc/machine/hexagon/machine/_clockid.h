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

#ifndef _MACHINE__CLOCKID_H
#define _MACHINE__CLOCKID_H

/* Tells <time.h> to skip its own CLOCK_* block. */
#define __machine_clockid_values_defined

#if __GNU_VISIBLE
#define CLOCK_REALTIME_COARSE (5)
#endif

#define CLOCK_REALTIME (0)

#if defined(_POSIX_CPUTIME)
#define CLOCK_PROCESS_CPUTIME_ID (3)
#endif

#if defined(_POSIX_THREAD_CPUTIME)
#define CLOCK_THREAD_CPUTIME_ID (2)
#endif

#if defined(_POSIX_MONOTONIC_CLOCK) || __GNU_VISIBLE
#define CLOCK_MONOTONIC (1)
#endif

#if __GNU_VISIBLE

#define CLOCK_MONOTONIC_RAW      (4)
#define CLOCK_MONOTONIC_COARSE   (6)
#define CLOCK_BOOTTIME           (32)
#define CLOCK_REALTIME_ALARM     (33)
#define CLOCK_BOOTTIME_ALARM     (34)
#define CLOCK_PROCESS_CPUTIME_ID (3)
#define CLOCK_THREAD_CPUTIME_ID  (2)

#endif

#endif /* _MACHINE__CLOCKID_H */
