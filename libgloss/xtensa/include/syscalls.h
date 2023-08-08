/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef XTENSA_SIMCALL_H
#define XTENSA_SIMCALL_H

#ifdef OPENOCD_SEMIHOSTING
/* This data based on libgloss project (file aarch64/svc.h) */

#define ESP_SEMIHOSTING_SYS_DRV_INFO 0x100
#define ADP_STOPPED_APPLICATION_EXIT 0x20026

#define SYS_close	0x02
#define SYS_clock	0x10
#define SYS_elapsed	0x30
#define SYS_errno	0x13
#define SYS_exit	0x18
#define SYS_exit_extended	0x20
#define SYS_flen	0x0C
#define SYS_get_cmdline	0x15
#define SYS_heapinfo	0x16
#define SYS_iserror	0x08
#define SYS_istty	0x09
#define SYS_open	0x01
#define SYS_read	0x06
#define SYS_readc	0x07
#define SYS_remove	0x0E
#define SYS_rename	0x0F
#define SYS_lseek	0x0A
#define SYS_system	0x12
#define SYS_tickfreq	0x31
#define SYS_time	0x11
#define SYS_tmpnam	0x0D
#define SYS_write	0x05
#define SYS_writec	0x03
#define SYS_write0	0x04

#else

/* This data based on QEMU project (file target/xtensa/xtensa-semi.c) */

#define SYS_exit	1
#define SYS_read	3
#define SYS_write	4
#define SYS_open	5
#define SYS_close	6
#define SYS_lseek	19
#define SYS_select_one	29
#define SYS_sendto	27
#define SYS_recvfrom	28
#define SYS_select_one 29
#define SYS_bind	30
#define SYS_ioctl	31

#define SYS_argc	1000
#define SYS_argv_size	1001
#define SYS_argv	1002
#define SYS_memset	1004

#endif /* OPENOCD_SEMIHOSTING */

#endif /* !XTENSA_SIMCALL_H */
