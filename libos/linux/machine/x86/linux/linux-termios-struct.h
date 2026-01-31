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

#ifndef _LINUX_TERMIOS_STRUCT_H_
#define _LINUX_TERMIOS_STRUCT_H_

#define LINUX_NCCS 19

typedef unsigned char __kernel_cc_t;
typedef unsigned int  __kernel_speed_t;
typedef unsigned int  __kernel_tcflag_t;

struct __kernel_termios {
    __kernel_tcflag_t c_iflag;          /* input mode flags */
    __kernel_tcflag_t c_oflag;          /* output mode flags */
    __kernel_tcflag_t c_cflag;          /* control mode flags */
    __kernel_tcflag_t c_lflag;          /* local mode flags */
    __kernel_cc_t     c_line;           /* line discipline */
    __kernel_cc_t     c_cc[LINUX_NCCS]; /* control characters */
    __kernel_speed_t  c_ispeed;
    __kernel_speed_t  c_ospeed;
};

#endif /* _LINUX_TERMIOS_STRUCT_H_ */
