/* time.h

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

/* Not defined in main time.h */
int clock_setres (clockid_t, struct timespec *);

/* GNU extensions. */
time_t timelocal (struct tm *);
time_t timegm (struct tm *);

#define TIMER_RELTIME  0 /* For compatibility with HP/UX, Solaris, others? */

#if __SVID_VISIBLE
extern int stime (const time_t *);
#endif

#if __SVID_VISIBLE || __XSI_VISIBLE
extern int daylight __asm__ (_SYMSTR (_daylight));

#ifndef __timezonefunc__
extern long timezone __asm__ (_SYMSTR (_timezone));
#endif

#endif /* __SVID_VISIBLE || __XSI_VISIBLE */

#ifdef __cplusplus
}
#endif
#endif /*_CYGWIN_TIME_H*/
