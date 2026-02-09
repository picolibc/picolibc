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

#define _DEFAULT_SOURCE
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define _NSIG_WORDS ((_NSIG + sizeof(unsigned) * 8 - 1) / (sizeof(unsigned) * 8))

static _sig_func_ptr _sig_func[_NSIG];
static unsigned      _sig_mask[_NSIG_WORDS];
static unsigned      _sig_pending[_NSIG_WORDS];

#ifndef __weak_reference
#define __fallback_signal signal
#define __fallback_raise  raise
#endif

_sig_func_ptr
__fallback_signal(int sig, _sig_func_ptr func)
{
    if (sig < 0 || sig >= _NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }

    _sig_func_ptr old = _sig_func[sig];
    _sig_func[sig] = func;
    return old;
}

#ifdef __weak_reference
__weak_reference(__fallback_signal, signal);
#endif

int
__fallback_raise(int sig)
{
    if (sig < 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }

    int      w;
    unsigned b;

    w = sig / (sizeof(_sig_mask[0]) * 8);
    b = 1U << (sig % (sizeof(_sig_mask[0]) * 8));

    /* Mark signal as pending */
    _sig_pending[w] |= b;

    /* Keep processing until the signal is no longer pending */
    while (_sig_pending[w] & b) {

        /* If signal is currently blocked, don't deliver it */
        if (_sig_mask[w] & b)
            break;

        /* Mark signal as no longer pending */
        _sig_pending[w] &= ~b;

        _sig_func_ptr func = _sig_func[sig];

        if (func == SIG_IGN)
            break;
        else if (func == SIG_DFL)
            _exit(128 + sig);

        /* Block signal while the handler is active */
        _sig_mask[w] |= b;

        /* Invoke the handler */
        (*func)(sig);

        _sig_mask[w] &= ~b;
    }

    return 0;
}

#ifdef __weak_reference
__weak_reference(__fallback_raise, raise);
#endif
