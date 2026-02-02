/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#include "local-linux.h"
#include <signal.h>
#include <linux/linux-signal.h>
#include <linux/linux-sigaction.h>

#if __SIZEOF_POINTER__ == 2 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2)
#define _USE_ATOMIC_SIGNAL
#endif

#if __SIZEOF_POINTER__ == 4 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
#define _USE_ATOMIC_SIGNAL
#endif

#if __SIZEOF_POINTER__ == 8 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
#define _USE_ATOMIC_SIGNAL
#endif

#ifdef _USE_ATOMIC_SIGNAL
#include <stdatomic.h>
static _Atomic _sig_func_ptr _sig_func[_NSIG];
#else
static _sig_func_ptr _sig_func[_NSIG];
#endif

static void
_signal_handler(int sig)
{
    sig = _signal_from_linux(sig);
    _sig_func_ptr func;
#ifdef USE_ATOMIC_SIGNAL
    func = (_sig_func_ptr)atomic_exchange(&_sig_func[sig], SIG_DFL);
#else
    func = _sig_func[sig];
    _sig_func[sig] = SIG_DFL;
#endif
    (*func)(sig);
}

extern void __restore_rt(void);
extern void __sa_restore(void);

#ifndef LINUX_SA_RESTORER
#define LINUX_SA_RESTORER 0x04000000
#endif
#ifndef LINUX_SA_RESTART
#define LINUX_SA_RESTART 0x10000000
#endif
#ifndef LINUX_NSIG
#define LINUX_NSIG 64
#endif
#define LINUX_NSIG_WORDS                                                         \
    ((LINUX_NSIG + sizeof(unsigned long) * 8 - 1) / (sizeof(unsigned long) * 8))
#define LINUX_NSIG_BYTES (LINUX_NSIG_WORDS * sizeof(unsigned long))

_sig_func_ptr
signal(int sig, _sig_func_ptr func)
{
    _sig_func_ptr             old, kfunc;
    int                       ksig;
    _sig_func_ptr             ret;
    struct __kernel_sigaction new_action = {};

    if (sig < 0 || sig >= _NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }

    ksig = _signal_to_linux(sig);

    if (func == SIG_DFL)
        kfunc = LINUX_SIG_DFL;
    else if (func == SIG_IGN)
        kfunc = LINUX_SIG_IGN;
    else
        kfunc = _signal_handler;

    new_action.sa_handler = kfunc;
    __kernel_sa_set_mask(&new_action, ksig);
    new_action.sa_flags = LINUX_SA_RESTART | LINUX_SA_RESTORER;
    new_action.sa_restorer = __sa_restore;

#if defined(LINUX_SYS_sigaction)
#define SYSCALL LINUX_SYS_sigaction
#else
#define SYSCALL LINUX_SYS_rt_sigaction
#endif

    ret = (_sig_func_ptr)syscall(SYSCALL, ksig, &new_action, NULL, LINUX_NSIG_BYTES);
    if (ret == LINUX_SIG_ERR)
        return SIG_ERR;

#ifdef _USE_ATOMIC_SIGNAL
    old = (_sig_func_ptr)atomic_exchange(&_sig_func[sig], func);
#else
    old = _sig_func[sig];
    _sig_func[sig] = func;
#endif

    return old;
}
