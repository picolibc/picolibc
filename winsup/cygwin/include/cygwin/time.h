/* time.h

   Copyright 2005 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_TIME_H
#define _CYGWIN_TIME_H
#ifdef __cplusplus
extern "C"
{
#endif

int nanosleep (const struct timespec  *, struct timespec *);
int clock_setres (clockid_t, struct timespec *);
int clock_getres (clockid_t, struct timespec *);

#define TIMER_RELTIME  0 /* For compatibility with HP/UX, Solaris, others? */

#ifndef __STRICT_ANSI__
# ifndef daylight
#   define daylight _daylight
# endif

# ifndef timezonevar
char *timezone (void);
# elif !defined(timezone)
#   define timezone _timezone
# endif
#endif /*__STRICT_ANSI__*/

#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_TIME_H*/
