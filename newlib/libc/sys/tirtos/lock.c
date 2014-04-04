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
/*
 *  ======== lock.c ========
 *  TIRTOS provides an implementation for the sys/lock APIs if re-entrancy
 *  support is enabled. If re-entrancy support is disabled or an app is built
 *  without TIRTOS but using the libraries built for TIRTOS, these empty
 *  stubs are required to succesfully link the app.
 */

#include <sys/lock.h>

/* Empty stubs for sys/lock APIs */

void __libc_lock_init(_LOCK_T *lock)
{
    return;
}

void __libc_lock_init_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_close(_LOCK_T *lock)
{
    return;
}

void __libc_lock_close_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_acquire(_LOCK_T *lock)
{
    return;
}

void __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    return;
}

void __libc_lock_release(_LOCK_T *lock)
{
    return;
}

void __libc_lock_release_recursive(_LOCK_RECURSIVE_T *lock)\
{
    return;
}

int __libc_lock_try_acquire(_LOCK_T *lock)
{
    return -1;
}

int __libc_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    return -1;
}
