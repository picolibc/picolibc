/*-
 * Copyright (c) 2016 embedded brains GmbH
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
 */

/*
 * Newlib ports may override the default implementations via the following
 * define and macros
 *  o _ARC4RANDOM_DATA,
 *  o _ARC4RANDOM_GETENTROPY_FAIL(),
 *  o _ARC4RANDOM_ALLOCATE(rsp, rspx), and
 *  o _ARC4RANDOM_FORKDETECT().
 */
#include <machine/_arc4random.h>

#include <sys/lock.h>
#include <signal.h>

#ifndef _ARC4_LOCK_INIT

#define _ARC4_LOCK_INIT __LOCK_INIT(static, __arc4random_mutex);

#define _ARC4_LOCK() __lock_acquire(__arc4random_mutex)

#define _ARC4_UNLOCK() __lock_release(__arc4random_mutex)

#endif /* _ARC4_LOCK_INIT */

#ifndef __SINGLE_THREAD__
_ARC4_LOCK_INIT
#endif

#ifdef _ARC4RANDOM_DATA
_ARC4RANDOM_DATA
#else
static struct {
	struct _rs rs;
	struct _rsx rsx;
} _arc4random_data;
#endif

static inline void
_getentropy_fail(void)
{
#ifdef _ARC4RANDOM_GETENTROPY_FAIL
	_ARC4RANDOM_GETENTROPY_FAIL();
#else
	raise(SIGKILL);
#endif
}

static inline int
_rs_allocate(struct _rs **rsp, struct _rsx **rsxp)
{
#ifdef _ARC4RANDOM_ALLOCATE
	_ARC4RANDOM_ALLOCATE(rsp, rsxp);
#else
	*rsp = &_arc4random_data.rs;
	*rsxp = &_arc4random_data.rsx;
	return (0);
#endif
}

static inline void
_rs_forkdetect(void)
{
#ifdef _ARC4RANDOM_FORKDETECT
	_ARC4RANDOM_FORKDETECT();
#endif
}
