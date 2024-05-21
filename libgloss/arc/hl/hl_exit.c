/*
 * hl_exit.c -- provide _exit().
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

#include <stdint.h>
#include <unistd.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* From crt0.  */
extern void __noreturn __longcall _exit_halt (void);

/* Push retcode to host.  Implements HL_SYSCALL_RETCODE.  */
static __always_inline void
_hl_retcode (int32_t ret)
{
  (void) _hl_message (HL_SYSCALL_RETCODE, "i", (uint32_t) ret);

  _hl_delete ();
}

void __noreturn
_exit (int ret)
{
  _hl_retcode (ret);

  _exit_halt ();
}
