/* sim-time.c -- stubs so clock can be linked in.
 *
 * Copyright (C) 2015 FTDI (support@ftdichip.com)
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
 */
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include "glue.h"

/*
 * _times -- no clock, so return an error.
 */
int
_DEFUN (_times, _times (buf),
        struct tms *buf)
{
  errno = EINVAL;
  return (-1);
}

/*
 * _gettimeofday -- implement in terms of time, which means we can't return the
 * microseconds.
 */
int
_DEFUN (_gettimeofday, _gettimeofday (tv, tz),
        struct timeval *tv _AND
        void *tzvp)
{
  struct timezone *tz = tzvp;
  if (tz)
    tz->tz_minuteswest = tz->tz_dsttime = 0;

  tv->tv_usec = 0;
  tv->tv_sec = 0;
  return 0;
}
