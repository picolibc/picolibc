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

#pragma once

#include <stdint.h>
#include <semihost.h>

#define SYS_CLOCK		(0x10)
#define SYS_CLOSE		(0x02)
#define SYS_ELAPSED		(0x30)
#define SYS_ERRNO		(0x13)
#define SYS_EXIT		(0x18)
#define SYS_EXIT_EXTENDED	(0x20)
#define SYS_FLEN		(0x0C)
#define SYS_GET_CMDLINE		(0x15)
#define SYS_HEAPINFO		(0x16)
#define SYS_ISERROR		(0x08)
#define SYS_ISTTY		(0x09)
#define SYS_OPEN		(0x01)
#define SYS_READ		(0x06)
#define SYS_READC		(0x07)
#define SYS_REMOVE		(0x0E)
#define SYS_RENAME		(0x0F)
#define SYS_SEEK		(0x0A)
#define SYS_SYSTEM		(0x12)
#define SYS_TICKFREQ		(0x31)
#define SYS_TIME		(0x11)
#define SYS_TMPNAM		(0x0D)
#define SYS_WRITE		(0x05)
#define SYS_WRITEC		(0x03)
#define SYS_WRITE0		(0x04)

#define SHFB_MAGIC_0	0x53
#define SHFB_MAGIC_1	0x48
#define SHFB_MAGIC_2	0x46
#define SHFB_MAGIC_3	0x42

#ifdef __aarch64__
typedef unsigned long long int sh_param_t;
#else
typedef uintptr_t sh_param_t;
#endif

uintptr_t
sys_semihost(uintptr_t op, uintptr_t param);

int
_map_stdio(int fd);
