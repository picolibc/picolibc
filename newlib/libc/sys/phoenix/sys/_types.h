/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef	_SYS__TYPES_H
#define _SYS__TYPES_H

#include <machine/_types.h>

#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int			_ssize_t;
#else
typedef long		_ssize_t;
#endif

typedef long		_off_t;
typedef long		_fpos_t;

typedef long int	__off_t;
typedef long int	__loff_t;
typedef	long		__suseconds_t;
#define	_CLOCK_T_	unsigned long

#define __need_wint_t
#include <stddef.h>

/* Conversion state information. */
typedef struct {
	int __count;
	union {
		wint_t __wch;
		unsigned char __wchb[4];
	} __value;
} _mbstate_t;

struct __flock_mutex_t_tmp;
typedef struct {
	int __a;
	int __b;
	struct {
		long int __c1;
		int __c2;
	} __c;
	int __d;
	struct __flock_mutex_t_tmp *__e;
} __flock_mutex_t;

typedef struct {
	__flock_mutex_t mutex;
} _flock_t;

#endif
