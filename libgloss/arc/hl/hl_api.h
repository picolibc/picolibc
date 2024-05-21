/*
 * hl_api.h -- provide high-level API for Hostlink IO.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <stdint.h>
#include "hl_toolchain.h"

#ifndef _HL_API_H
#define _HL_API_H

#define HL_SYSCALL_OPEN		0
#define HL_SYSCALL_CLOSE	1
#define HL_SYSCALL_READ		2
#define HL_SYSCALL_WRITE	3
#define HL_SYSCALL_LSEEK	4
#define HL_SYSCALL_UNLINK	5
#define HL_SYSCALL_ISATTY	6
#define HL_SYSCALL_TMPNAM	7
#define HL_SYSCALL_GETENV	8
#define HL_SYSCALL_CLOCK	9
#define HL_SYSCALL_TIME		10
#define HL_SYSCALL_RENAME	11
#define HL_SYSCALL_ARGC		12
#define HL_SYSCALL_ARGV		13
#define HL_SYSCALL_RETCODE	14
#define HL_SYSCALL_ACCESS	15
#define HL_SYSCALL_GETPID	16
#define HL_SYSCALL_GETCWD	17
#define HL_SYSCALL_USER		18

#define HL_GNUIO_EXT_VENDOR_ID	1025

#define HL_GNUIO_EXT_FSTAT	1

/*
 * Main functions to work with regular syscalls and user-defined hostlink
 * messages.
 */
volatile __uncached char *_hl_message (uint32_t syscall,
				       const char *format, ...);
uint32_t _user_hostlink (uint32_t vendor, uint32_t opcode,
			 const char *format, ...);


/* Fuctions for direct work with the Hostlink buffer.  */
volatile __uncached char *_hl_pack_int (volatile __uncached char *p,
					uint32_t x);
volatile __uncached char *_hl_pack_ptr (volatile __uncached char *p,
					const void *s, uint16_t len);
volatile __uncached char *_hl_pack_str (volatile __uncached char *p,
					const char *s);
volatile __uncached char *_hl_unpack_int (volatile __uncached char *p,
					  uint32_t *x);
volatile __uncached char *_hl_unpack_ptr (volatile __uncached char *p,
					  void *s, uint32_t *plen);
volatile __uncached char *_hl_unpack_str (volatile __uncached char *p,
					  char *s);
uint32_t _hl_get_ptr_len (volatile __uncached char *p);

/* Low-level functions from hl_gw.  */
extern uint32_t _hl_iochunk_size (void);
extern void _hl_delete (void);

#endif /* !_HL_API_H  */
