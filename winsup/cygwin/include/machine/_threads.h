/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2011 Ed Schouten <ed@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _MACHINE__THREADS_H_
#define	_MACHINE__THREADS_H_

#include <sys/types.h>
#include <sys/lock.h>

/*
 * The C11 threads interface.
 *
 * This interface is implemented as a light-weight wrapper around
 * <pthread.h>.  To prevent namespace pollution, the once_flag object,
 * its corresponding ONCE_FLAG_INIT and TSS_DTOR_ITERATIONS have been
 * copied from this header file.  They must be kept in sync.
 */

typedef pthread_cond_t	cnd_t;
typedef pthread_mutex_t	mtx_t;
typedef pthread_t	thrd_t;
typedef pthread_key_t	tss_t;

/* pthread_once_t */
typedef struct {
  mtx_t mutex;
  int state;
} once_flag;

/* PTHREAD_ONCE_INIT */
#define	ONCE_FLAG_INIT { ((pthread_mutex_t)19), 0 }

/* PTHREAD_DESTRUCTOR_ITERATIONS */
#define	TSS_DTOR_ITERATIONS 4

#endif /* _MACHINE__THREADS_H_ */
