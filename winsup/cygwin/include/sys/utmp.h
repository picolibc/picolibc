/* sys/utmp.h

   Copyright 2001, 2004 Red Hat, Inc.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef UTMP_H
#define UTMP_H

#include <cygwin/utmp.h>

#define UTMP_FILE _PATH_UTMP

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ut_name
#define ut_name		ut_user
#endif


struct utmp
{
 short	ut_type;
 pid_t	ut_pid;
 char	ut_line[UT_LINESIZE];
 char  ut_id[UT_IDLEN];
 time_t ut_time;
 char	ut_user[UT_NAMESIZE];
 char	ut_host[UT_HOSTSIZE];
 long	ut_addr;
};

extern struct utmp *_getutline (struct utmp *);
extern struct utmp *getutent (void);
extern struct utmp *getutid (struct utmp *);
extern struct utmp *getutline (struct utmp *);
extern struct utmp *pututline (struct utmp *);
extern void endutent (void);
extern void setutent (void);
extern void utmpname (const char *);

void login (struct utmp *);
int logout (char *);
int login_tty (int);
void updwtmp (const char *, const struct utmp *);
void logwtmp (const char *, const char *, const char *);

#ifdef __cplusplus
}
#endif
#endif /* UTMP_H */
