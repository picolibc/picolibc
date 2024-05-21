/*
 * hl_clock.c -- provide _clock() and _times().
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
#include <time.h>
#include <sys/times.h>

#include "../arc-timer.h"

#include "hl_toolchain.h"
#include "hl_api.h"

/* Implements HL_SYSCALL_CLOCK.  */
static __always_inline clock_t
_hl_clock (void)
{
  uint32_t ret;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_CLOCK, ":i", (uint32_t *) &ret);

  _hl_delete ();

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      return -1;
    }

  return ret;
}

/*
 * This implementation returns timer 0 value if it exists or
 * host clock() value converted to target clocks.
 *
 * Please note that this value cannot be converted to seconds
 * using CLOCKS_PER_SEC.
 */
clock_t
_clock (void)
{
  if (_arc_timer_default_present ())
    return _arc_timer_default_read ();
  else
    return _hl_clock ();
}

/* All time is counted as user time.  */
clock_t
_times (struct tms *tp)
{
  clock_t ret;

  if (tp == NULL)
    {
      errno = EFAULT;
      return -1;
    }

  ret = _clock ();

  if (ret == (clock_t) -1)
    return ret;

  tp->tms_utime = ret;
  tp->tms_stime = 0;
  tp->tms_cutime = 0;
  tp->tms_cstime = 0;

  return ret;
}
