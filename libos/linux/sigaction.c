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

#include "local-sigaction.h"

#if __SIZEOF_POINTER__ == 2 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2)
#define _USE_ATOMIC_SIGNAL
#endif

#if __SIZEOF_POINTER__ == 4 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
#define _USE_ATOMIC_SIGNAL
#endif

#if __SIZEOF_POINTER__ == 8 && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
#define _USE_ATOMIC_SIGNAL
#endif

typedef union {
    _sig_func_ptr   sa_handler;
    _sig_action_ptr sa_sigaction;
} sig_func_t;

#ifdef _USE_ATOMIC_SIGNAL
#include <stdatomic.h>
static _Atomic sig_func_t _sig_func[_NSIG];
#else
static sig_func_t _sig_func[_NSIG];
#endif

static void
_signal_handler(int sig)
{
    sig_func_t func;

    sig = _signal_from_linux(sig);
#ifdef _USE_ATOMIC_SIGNAL
    func = (sig_func_t)atomic_load(&_sig_func[sig]);
#else
    func = _sig_func[sig];
#endif
    (*func.sa_handler)(sig);
}

static void
_signal_sigaction(int sig, struct __kernel_siginfo *kinfo, void *ucontext)
{
    siginfo_t  info = {};
    sig_func_t func;

    sig = _signal_from_linux(sig);
#ifdef _USE_ATOMIC_SIGNAL
    func = (sig_func_t)atomic_load(&_sig_func[sig]);
#else
    func = _sig_func[sig];
#endif
    SIMPLE_MAP_SIGINFO(&info, kinfo);
    SIMPLE_MAP_SIGVAL(&info.si_value, &(kinfo->si_value));
    (*func.sa_sigaction)(sig, &info, ucontext);
}

extern void __restore_rt(void);
extern void __sa_restore(void);

static const struct {
    int pflag;
    int lflag;
} flags[] = {
    { .pflag = SA_NOCLDSTOP, .lflag = LINUX_SA_NOCLDSTOP },
    { .pflag = SA_ONSTACK,   .lflag = LINUX_SA_ONSTACK   },
    { .pflag = SA_RESETHAND, .lflag = LINUX_SA_RESETHAND },
    { .pflag = SA_RESTART,   .lflag = LINUX_SA_RESTART   },
    { .pflag = SA_SIGINFO,   .lflag = LINUX_SA_SIGINFO   },
    { .pflag = SA_NOCLDWAIT, .lflag = LINUX_SA_NOCLDWAIT },
    { .pflag = SA_NODEFER,   .lflag = LINUX_SA_NODEFER   },
};

#define NFLAG (sizeof(flags) / sizeof(flags[0]))

int
sigaction(int sig, const struct sigaction *act, struct sigaction *oldact)
{
    struct __kernel_sigaction  kact = {};
    struct __kernel_sigaction *pact = NULL;
    struct __kernel_sigaction  koldact;
    struct __kernel_sigaction *poldact = NULL;
    int                        ksig;
    int                        ret;
    size_t                     f;
    sig_func_t                 func, oldfunc;

    if (sig < 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }

    ksig = _signal_to_linux(sig);

    if (act) {
        kact.sa_flags = LINUX_SA_RESTORER;

        if (act->sa_flags & SA_SIGINFO) {
            func.sa_sigaction = act->sa_sigaction;
            kact.sa_handler = (void *)_signal_sigaction;
            kact.sa_flags |= LINUX_SA_SIGINFO;
        } else {
            func.sa_handler = act->sa_handler;
            kact.sa_handler = _signal_handler;
        }

        if (act->sa_handler == SIG_DFL)
            kact.sa_handler = LINUX_SIG_DFL;
        else if (act->sa_handler == SIG_IGN)
            kact.sa_handler = LINUX_SIG_IGN;

        for (f = 0; f < NFLAG; f++)
            if (act->sa_flags & flags[f].pflag)
                kact.sa_flags |= flags[f].lflag;

        _sigmask_to_linux(&kact.sa_mask, &act->sa_mask);

        kact.sa_restorer = __sa_restore;

        pact = &kact;
    }

    if (oldact)
        poldact = &koldact;

    ret = syscall(LINUX_SYS_rt_sigaction, ksig, pact, poldact, __KERNEL_NSIG_BYTES);

    if (ret == 0) {
#ifdef _USE_ATOMIC_SIGNAL
        oldfunc = (sig_func_t)atomic_exchange(&_sig_func[sig], func);
#else
        oldfunc = _sig_func[sig];
        _sig_func[sig] = func;
#endif
        if (oldact) {
            oldact->sa_handler = oldfunc.sa_handler;
            _sigmask_from_linux(&oldact->sa_mask, &koldact.sa_mask);

            oldact->sa_flags = 0;

            for (f = 0; f < NFLAG; f++)
                if (koldact.sa_flags & flags[f].lflag)
                    oldact->sa_flags |= flags[f].pflag;
        }
    }

    return ret;
}
