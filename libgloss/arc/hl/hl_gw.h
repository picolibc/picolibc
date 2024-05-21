/*
 * hl_gw.h -- Hostlink gateway, low-level hostlink functions.
 * This header should not be used directly, please use hl_api.h instead.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#ifndef _HL_GW_H
#define _HL_GW_H

#include <stdint.h>

#include "hl_toolchain.h"

/* Get hostlink payload pointer and size available for using.  */
volatile __uncached void *_hl_payload (void);

/* Maximum amount of data that can be sent via hostlink in one message.  */
uint32_t _hl_iochunk_size (void);

/*
 * How many bytes are available in the hostlink payload buffer.
 * This may be bigger than iochunk size because hostlink payload also contains
 * reserved space for service information.
 */
uint32_t _hl_payload_left (volatile __uncached void *p);

/* Send and receive hostlink packet.  */
void _hl_send (volatile __uncached void *p);
volatile __uncached char *_hl_recv (void);

/* Mark target2host buffer as "No message here".  */
void _hl_delete (void);

#endif /* !_HL_GW_H  */
