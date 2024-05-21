/*
 * hl_gettimeofday.c -- provide _gettimeofday().
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

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* Get host time().  Implements HL_SYSCALL_TIME.  */
static __always_inline int
_hl_time (uint32_t *host_timer)
{
  int ret;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_TIME, ":i", (uint32_t *) host_timer);

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      ret = -1;
    }
  else
    {
      ret = 0;
    }

  _hl_delete ();

  return ret;
}

/* gettimeofday() implementation.  Clears *tz if specified.  */
int
_gettimeofday (struct timeval *tv, struct timezone *tz)
{
  int ret;
  uint32_t host_timer;

  if (tz)
    memset (tz, 0, sizeof (*tz));

  ret = _hl_time (&host_timer);

  if (ret == 0)
    {
      tv->tv_sec = host_timer;
      tv->tv_usec = 0;
    }

  return ret;
}
