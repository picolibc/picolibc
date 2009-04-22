/* qemu-time.c -- stubs so clock can be linked in.
 *
 * Copyright (c) 2008 Anthony Green
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
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "glue.h"

/* This is the base address of the mc146818 RTC i/o port. */
#define RTC_PORT 0x400

#define RTC_SECONDS             0x00
#define RTC_SECONDS_ALARM       0x01
#define RTC_MINUTES             0x02
#define RTC_MINUTES_ALARM       0x03
#define RTC_HOURS               0x04
#define RTC_HOURS_ALARM         0x05
#define RTC_DAY_OF_WEEK         0x06
#define RTC_DATE_OF_MONTH       0x07
#define RTC_MONTH               0x08
#define RTC_YEAR                0x09
#define RTC_CONFIG_A            0x0A
#define RTC_CONFIG_B            0x0B
#define RTC_CONFIG_C            0x0C
#define RTC_CONFIG_D            0x0D

/*
 * _times -- FIXME
 */
int
_DEFUN (_times, _times (buf),
	struct tms *buf)
{
  errno = EINVAL;
  return (-1);
}

/* Read a value from the RTC port.  */
static unsigned char 
rtc_read (unsigned char reg)
{
  *(volatile unsigned char *)(RTC_PORT) = reg;
  return *(volatile unsigned char *)(RTC_PORT + 1);
}

/* Write a value to the RTC port.  */
static void 
rtc_write (unsigned char reg, unsigned char val)
{
  *(volatile unsigned char *)(RTC_PORT) = reg;
  *(volatile unsigned char *)(RTC_PORT + 1) = val;
}

/* Convert BCD to Decimal.  */
#define BCD2DEC(BYTE) ((((BYTE >> 4) & 0xF) * 10) + (BYTE & 0xF))

/*
 * time -- return current time in seconds.
 */
time_t
_DEFUN (time, time (t),
	time_t *t)
{
  struct tm tm;
  time_t ret;

  tm.tm_sec = BCD2DEC(rtc_read(RTC_SECONDS));
  tm.tm_min = BCD2DEC(rtc_read(RTC_MINUTES));
  tm.tm_hour = BCD2DEC(rtc_read(RTC_HOURS));
  tm.tm_wday = BCD2DEC(rtc_read(RTC_DAY_OF_WEEK)) - 1;
  tm.tm_mday = BCD2DEC(rtc_read(RTC_DATE_OF_MONTH));
  tm.tm_mon = BCD2DEC(rtc_read(RTC_MONTH)) - 1;
  tm.tm_year = BCD2DEC(rtc_read(RTC_YEAR)) + 100;

  /* FIXME.  Not really sure how to handle this.  */
  tm.tm_isdst = 1;

  ret = mktime(&tm);

  if (t)
    *t = ret;

  return ret;
}

/*
 * _gettimeofday -- implement in terms of time, which means we can't
 * return the microseconds.
 */
int
_DEFUN (_gettimeofday, _gettimeofday (tv, tz),
	struct timeval *tv _AND
	void *tzvp)
{
  struct timezone *tz = tzvp;
  struct tm tm;
  if (tz)
    tz->tz_minuteswest = tz->tz_dsttime = 0;

  tv->tv_usec = 0;
  tv->tv_sec = time(0);

  return 0;
}
