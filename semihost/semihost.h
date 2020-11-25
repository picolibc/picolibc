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

#include <stdio.h>
#include <stdbool.h>

/* Semihost exceptions for sys_semihost_exit */
#define ADP_Stopped_BranchThroughZero	0x20000
#define ADP_Stopped_UndefinedInstr	0x20001
#define ADP_Stopped_SoftwareInterrupt	0x20002
#define ADP_Stopped_PrefetchAbort	0x20003
#define ADP_Stopped_DataAbort		0x20004
#define ADP_Stopped_AddressException	0x20005
#define ADP_Stopped_IRQ			0x20006
#define ADP_Stopped_FIQ			0x20007
#define ADP_Stopped_BreakPoint		0x20020
#define ADP_Stopped_WatchPoint		0x20021
#define ADP_Stopped_StepComplete	0x20022
#define ADP_Stopped_RunTimeErrorUnknown	0x20023
#define ADP_Stopped_InternalError	0x20024
#define ADP_Stopped_UserInterruption	0x20025
#define ADP_Stopped_ApplicationExit	0x20026
#define ADP_Stopped_StackOverflow	0x20027
#define ADP_Stopped_DivisionByZero	0x20028
#define ADP_Stopped_OSSpecific		0x20029

/* Semihost extensions */
#define SH_EXT_EXIT_EXTENDED		0
#define SH_EXT_STDOUT_STDERR		1
#define SH_EXT_NUM			2

/* Open modes */
#define SH_OPEN_R			0
#define SH_OPEN_R_B			1
#define SH_OPEN_R_PLUS			2
#define SH_OPEN_R_PLUS_B		3
#define SH_OPEN_W			4
#define SH_OPEN_W_B			5
#define SH_OPEN_W_PLUS			6
#define SH_OPEN_W_PLUS_B		7
#define SH_OPEN_A			8
#define SH_OPEN_A_B			9
#define SH_OPEN_A_PLUS			10
#define SH_OPEN_A_PLUS_B		11

uintptr_t
sys_semihost_clock(void);

int
sys_semihost_close(int fd);

uint64_t
sys_semihost_elapsed(void);

int
sys_semihost_errno(void);

void
sys_semihost_exit(uintptr_t exception, uintptr_t subcode) _ATTRIBUTE((__noreturn__));

void
sys_semihost_exit_extended(uintptr_t code) _ATTRIBUTE((__noreturn__));

bool
sys_semihost_feature(uint8_t feature);

uintptr_t
sys_semihost_flen(int fd);

int
sys_semihost_get_cmdline(char *buf, int size);

int
sys_semihost_getc(FILE *file);

struct sys_semihost_block {
	void	*heap_base;
	void	*heap_limit;
	void	*stack_base;
	void	*stack_limit;
};

void
sys_semihost_heapinfo(struct sys_semihost_block *block);

int
sys_semihost_iserror(intptr_t status);

int
sys_semihost_istty(int fd);

int
sys_semihost_open(const char *pathname, int semiflags);

int
sys_semihost_putc(char c, FILE *file);

uintptr_t
sys_semihost_read(int fd, void *buf, size_t count);

int
sys_semihost_remove(const char *pathname);

int
sys_semihost_rename(const char *old_pathname, const char *new_pathname);

int
sys_semihost_seek(int fd, uintptr_t pos);

int
sys_semihost_system(const char *command);

uintptr_t
sys_semihost_tickfreq(void);

uintptr_t
sys_semihost_time(void);

int
sys_semihost_tmpnam(char *pathname, int identifier, int maxpath);

uintptr_t
sys_semihost_write(int fd, const void *buf, uintptr_t count);

void
sys_semihost_write0(const char *string);
