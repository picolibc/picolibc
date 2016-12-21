/*
 * Copyright (c) 2015, 2016 embedded brains GmbH.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_LOCK_H_
#define _SYS_LOCK_H_

#include <sys/cdefs.h>
#include <stddef.h>

__BEGIN_DECLS

struct timespec;

struct _Thread_Control;

struct _Thread_queue_Heads;

struct _Ticket_lock_Control {
	unsigned int _next_ticket;
	unsigned int _now_serving;
};

struct _Thread_queue_Queue {
	struct _Ticket_lock_Control _Lock;
	struct _Thread_queue_Heads *_heads;
	struct _Thread_Control *_owner;
	const char *_name;
};

struct _Mutex_Control {
	struct _Thread_queue_Queue _Queue;
};

struct _Mutex_recursive_Control {
	struct _Mutex_Control _Mutex;
	unsigned int _nest_level;
};

struct _Condition_Control {
	struct _Thread_queue_Queue _Queue;
};

struct _Semaphore_Control {
	struct _Thread_queue_Queue _Queue;
	unsigned int _count;
};

struct _Futex_Control {
	struct _Thread_queue_Queue _Queue;
};

#define _TICKET_LOCK_INITIALIZER { 0, 0 }

#define _THREAD_QUEUE_INITIALIZER { _TICKET_LOCK_INITIALIZER, 0, 0, 0 }

#define _THREAD_QUEUE_NAMED_INITIALIZER(_name) \
    { _TICKET_LOCK_INITIALIZER, 0, 0, _name }

#define _MUTEX_INITIALIZER { _THREAD_QUEUE_INITIALIZER }

#define _MUTEX_NAMED_INITIALIZER(_name) \
    { _THREAD_QUEUE_NAMED_INITIALIZER(_name) }

#define _MUTEX_RECURSIVE_INITIALIZER { _MUTEX_INITIALIZER, 0 }

#define _MUTEX_RECURSIVE_NAMED_INITIALIZER(_name) \
    { _MUTEX_NAMED_INITIALIZER(_name), 0 }

#define _CONDITION_INITIALIZER { _THREAD_QUEUE_INITIALIZER }

#define _CONDITION_NAMED_INITIALIZER(_name) \
    { _THREAD_QUEUE_NAMED_INITIALIZER(_name) }

#define _SEMAPHORE_INITIALIZER(_count) { _THREAD_QUEUE_INITIALIZER, _count }

#define _SEMAPHORE_NAMED_INITIALIZER(_name, _count) \
    { _THREAD_QUEUE_NAMED_INITIALIZER(_name), _count }

#define _FUTEX_INITIALIZER { _THREAD_QUEUE_INITIALIZER }

#define _FUTEX_NAMED_INITIALIZER(_name) \
    { _THREAD_QUEUE_NAMED_INITIALIZER(_name) }

static __inline void
_Mutex_Initialize(struct _Mutex_Control *_mutex)
{
	struct _Mutex_Control _init = _MUTEX_INITIALIZER;

	*_mutex = _init;
}

static __inline void
_Mutex_Initialize_named(struct _Mutex_Control *_mutex, const char *_name)
{
	struct _Mutex_Control _init = _MUTEX_NAMED_INITIALIZER(_name);

	*_mutex = _init;
}

void _Mutex_Acquire(struct _Mutex_Control *);

int _Mutex_Acquire_timed(struct _Mutex_Control *, const struct timespec *);

int _Mutex_Try_acquire(struct _Mutex_Control *);

void _Mutex_Release(struct _Mutex_Control *);

static __inline void
_Mutex_Destroy(struct _Mutex_Control *_mutex)
{

	(void)_mutex;
}

static __inline void
_Mutex_recursive_Initialize(struct _Mutex_recursive_Control *_mutex)
{
	struct _Mutex_recursive_Control _init = _MUTEX_RECURSIVE_INITIALIZER;

	*_mutex = _init;
}

static __inline void
_Mutex_recursive_Initialize_named(struct _Mutex_recursive_Control *_mutex,
    const char *_name)
{
	struct _Mutex_recursive_Control _init =
	    _MUTEX_RECURSIVE_NAMED_INITIALIZER(_name);

	*_mutex = _init;
}

void _Mutex_recursive_Acquire(struct _Mutex_recursive_Control *);

int _Mutex_recursive_Acquire_timed(struct _Mutex_recursive_Control *,
    const struct timespec *);

int _Mutex_recursive_Try_acquire(struct _Mutex_recursive_Control *);

void _Mutex_recursive_Release(struct _Mutex_recursive_Control *);

static __inline void
_Mutex_recursive_Destroy(struct _Mutex_recursive_Control *_mutex)
{

	(void)_mutex;
}

static __inline void
_Condition_Initialize(struct _Condition_Control *_cond)
{
	struct _Condition_Control _init = _CONDITION_INITIALIZER;

	*_cond = _init;
}

static __inline void
_Condition_Initialize_named(struct _Condition_Control *_cond,
    const char *_name)
{
	struct _Condition_Control _init = _CONDITION_NAMED_INITIALIZER(_name);

	*_cond = _init;
}

void _Condition_Wait(struct _Condition_Control *, struct _Mutex_Control *);

int _Condition_Wait_timed(struct _Condition_Control *,
    struct _Mutex_Control *, const struct timespec *);

void _Condition_Wait_recursive(struct _Condition_Control *,
    struct _Mutex_recursive_Control *);

int _Condition_Wait_recursive_timed(struct _Condition_Control *,
    struct _Mutex_recursive_Control *, const struct timespec *);

void _Condition_Signal(struct _Condition_Control *);

void _Condition_Broadcast(struct _Condition_Control *);

static __inline void
_Condition_Destroy(struct _Condition_Control *_cond)
{

	(void)_cond;
}

static __inline void
_Semaphore_Initialize(struct _Semaphore_Control *_semaphore,
    unsigned int _count)
{
	struct _Semaphore_Control _init = _SEMAPHORE_INITIALIZER(_count);

	*_semaphore = _init;
}

static __inline void
_Semaphore_Initialize_named(struct _Semaphore_Control *_semaphore,
    const char *_name, unsigned int _count)
{
	struct _Semaphore_Control _init =
	    _SEMAPHORE_NAMED_INITIALIZER(_name, _count);

	*_semaphore = _init;
}

void _Semaphore_Wait(struct _Semaphore_Control *);

void _Semaphore_Post(struct _Semaphore_Control *);

static __inline void
_Semaphore_Destroy(struct _Semaphore_Control *_semaphore)
{

	(void)_semaphore;
}

static __inline void
_Futex_Initialize(struct _Futex_Control *_futex)
{
	struct _Futex_Control _init = _FUTEX_INITIALIZER;

	*_futex = _init;
}

static __inline void
_Futex_Initialize_named(struct _Futex_Control *_futex, const char *_name)
{
	struct _Futex_Control _init = _FUTEX_NAMED_INITIALIZER(_name);

	*_futex = _init;
}

int _Futex_Wait(struct _Futex_Control *, int *, int);

int _Futex_Wake(struct _Futex_Control *, int);

static __inline void
_Futex_Destroy(struct _Futex_Control *_futex)
{

	(void)_futex;
}

int _Sched_Count(void);

int _Sched_Index(void);

int _Sched_Name_to_index(const char *, size_t);

int _Sched_Processor_count(int);

/* Newlib internal locks */

typedef struct _Mutex_Control _LOCK_T;

typedef struct _Mutex_recursive_Control _LOCK_RECURSIVE_T;

#define __LOCK_INIT(_qualifier, _designator) \
    _qualifier _LOCK_T _designator = _MUTEX_INITIALIZER

#define __LOCK_INIT_RECURSIVE(_qualifier, _designator) \
    _qualifier _LOCK_RECURSIVE_T _designator = _MUTEX_RECURSIVE_INITIALIZER

#define __lock_init(_lock) _Mutex_Initialize(&_lock)
#define __lock_acquire(_lock) _Mutex_Acquire(&_lock)
#define __lock_try_acquire(lock) _Mutex_Try_acquire(&_lock)
#define __lock_release(_lock) _Mutex_Release(&_lock)
#define __lock_close(_lock) _Mutex_Destroy(&_lock)

#define __lock_init_recursive(_lock) _Mutex_recursive_Initialize(&_lock)
#define __lock_acquire_recursive(_lock) _Mutex_recursive_Acquire(&_lock)
#define __lock_try_acquire_recursive(lock) _Mutex_recursive_Try_acquire(&_lock)
#define __lock_release_recursive(_lock) _Mutex_recursive_Release(&_lock)
#define __lock_close_recursive(_lock) _Mutex_recursive_Destroy(&_lock)

__END_DECLS

#ifdef _KERNEL
/* Header file provided outside of Newlib */
#include <machine/_kernel_lock.h>
#endif

#endif /* !_SYS_LOCK_H_ */
