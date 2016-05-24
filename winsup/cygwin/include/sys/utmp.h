/* sys/utmp.h

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

extern struct utmp *getutent (void);
extern struct utmp *getutid (const struct utmp *);
extern struct utmp *getutline (const struct utmp *);
extern struct utmp *pututline (const struct utmp *);
extern void endutent (void);
extern void setutent (void);
extern int utmpname (const char *);

void login (const struct utmp *);
int logout (const char *);
int login_tty (int);
void updwtmp (const char *, const struct utmp *);
void logwtmp (const char *, const char *, const char *);

#ifdef __cplusplus
}
#endif
#endif /* UTMP_H */
