/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include <stdbool.h>

struct uart {
        volatile uint32_t       rx;
        volatile uint32_t       tx;
        volatile uint32_t       status;
};

#define UART    ((struct uart *) 0x84000000)
#define STATUS_RXVALID    0x01
#define STATUS_TXFULL     0x08

static inline bool
uart_tx_ready(void)
{
        return (UART->status & STATUS_TXFULL) == 0;
}

static inline void
uart_put(char c)
{
        while (!uart_tx_ready())
                ;
        UART->tx = (uint32_t) (unsigned char) c;
}

static inline bool
uart_rx_ready(void)
{
        return !!(UART->status & STATUS_RXVALID);
}

static inline char
uart_get(void)
{
        while (!uart_rx_ready())
                ;
        return (char) UART->rx;
}

#endif /* _UART_H_ */
