/* bsd_log.cc

   Copyright 2003, 2004, 2012 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#define _KERNEL 1
#define __BSD_VISIBLE 1
#include <stdio.h>
#include <stdlib.h>

int32_t log_level = 8; /* Illegal value.  Don't change! */
tun_bool_t log_debug = TUN_UNDEF;
tun_bool_t log_syslog = TUN_UNDEF;
tun_bool_t log_stderr = TUN_UNDEF;

void
loginit (tun_bool_t opt_stderr, tun_bool_t opt_syslog)
{
  if (log_debug == TUN_UNDEF)
    TUNABLE_BOOL_FETCH ("kern.log.debug", &log_debug);
  if (log_debug == TUN_UNDEF)
    log_debug = TUN_FALSE;

  if (opt_stderr != TUN_UNDEF)
    log_stderr = opt_stderr;
  else
    TUNABLE_BOOL_FETCH ("kern.log.stderr", &log_stderr);
  if (log_stderr == TUN_UNDEF)
    log_stderr = TUN_FALSE;

  if (opt_syslog != TUN_UNDEF)
    log_syslog = opt_syslog;
  else
    TUNABLE_BOOL_FETCH ("kern.log.syslog", &log_syslog);
  if (log_syslog == TUN_UNDEF)
    log_syslog = TUN_FALSE;

  if (log_level == 8)
    TUNABLE_INT_FETCH ("kern.log.level", &log_level);
  if (log_level == 8)
    log_level = 6;
}

void
_vlog (const char *file, int line, int level,
	const char *fmt, va_list ap)
{
  char buf[16384];
  char *pos;

  if ((level == LOG_DEBUG && log_debug != TUN_TRUE)
      || (level != LOG_DEBUG && level >= log_level))
    return;
  pos = stpcpy (buf, "cygserver: ");
  if (file && log_debug == TUN_TRUE)
    pos += snprintf (pos, 16384 - (pos - buf), "%s, line %d: ", file, line);
  vsnprintf (pos, 16384 - (pos - buf), fmt, ap);
  if (log_syslog == TUN_TRUE && level != LOG_DEBUG)
    syslog (level, buf);
  if (log_stderr == TUN_TRUE || level == LOG_DEBUG)
    {
      fputs (buf, stderr);
      fputc ('\n', stderr);
    }
}

void
_log (const char *file, int line, int level, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  _vlog (file, line, level, fmt, ap);
}

void
_vpanic (const char *file, int line, const char *fmt, va_list ap)
{
  _vlog (file, line, LOG_CRIT, fmt, ap);
  exit (1);
}

void
_panic (const char *file, int line, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  _vpanic (file, line, fmt, ap);
}
#endif /* __OUTSIDE_CYGWIN__ */
