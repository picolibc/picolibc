/*
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* 
 * Lock routines for TI-RTOS multi-threaded apps. The implementation
 * is provided by TI-RTOS kernel.
 */

/*
 * Dummy semaphore structure of size 32 bytes. This will be
 * type casted to Semaphore_struct in TI-RTOS.
 */
typedef struct {
    void *dummy[8];
} __dummy_sem_t;

typedef struct {
    __dummy_sem_t sem;
    unsigned char init_done;
} __lock_mutex_t;

typedef struct {
    void *owner;
    __dummy_sem_t sem;
    unsigned int count;
    unsigned char init_done;
} __lock_recursive_mutex_t;

typedef __lock_mutex_t _LOCK_T;
typedef __lock_recursive_mutex_t _LOCK_RECURSIVE_T;
 
#include <_ansi.h>

#define __LOCK_INIT(class,lock) \
    class _LOCK_T lock = { .sem.dummy = {0, 0, 0, 0, 0, 0, 0, 0}, \
        .init_done = 0 }

#define __LOCK_INIT_RECURSIVE(class,lock) \
    class _LOCK_RECURSIVE_T lock = { .owner = 0, \
        .sem.dummy = {0, 0, 0, 0, 0, 0, 0, 0}, .count = 0, \
        .init_done = 0 }

extern void __libc_lock_init(_LOCK_T *lock);
extern void __libc_lock_init_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_close(_LOCK_T *lock);
extern void __libc_lock_close_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_acquire(_LOCK_T *lock);
extern void __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock);
extern void __libc_lock_release(_LOCK_T *lock);
extern void __libc_lock_release_recursive(_LOCK_RECURSIVE_T *lock);

/* Returns 0 for success and non-zero for failure */
extern int __libc_lock_try_acquire(_LOCK_T *lock);
extern int __libc_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock);

#define __lock_init(lock) \
    __libc_lock_init(&(lock))
#define __lock_init_recursive(lock) \
    __libc_lock_init_recursive(&(lock))
#define __lock_close(lock) \
    __libc_lock_close(&(lock))
#define __lock_close_recursive(lock) \
    __libc_lock_close_recursive(&(lock))
#define __lock_acquire(lock) \
    __libc_lock_acquire(&(lock))
#define __lock_acquire_recursive(lock) \
    __libc_lock_acquire_recursive(&(lock))
#define __lock_try_acquire(lock) \
    __libc_lock_try_acquire(&(lock))
#define __lock_try_acquire_recursive(lock) \
    __libc_lock_try_acquire_recursive(&(lock))
#define __lock_release(lock) \
    __libc_lock_release(&(lock))
#define __lock_release_recursive(lock) \
    __libc_lock_release_recursive(&(lock))

#endif /* __SYS_LOCK_H__ */
