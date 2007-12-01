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

/* GNU extensions. */
time_t timelocal (struct tm *);
time_t timegm (struct tm *);

#define TIMER_RELTIME  0 /* For compatibility with HP/UX, Solaris, others? */

#ifndef __STRICT_ANSI__

extern int daylight __asm__ ("__daylight");

#ifndef __timezonefunc__
extern long timezone __asm__ ("__timezone");
#endif

#endif /*__STRICT_ANSI__*/

#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_TIME_H*/
