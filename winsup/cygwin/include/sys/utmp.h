/* sys/utmp.h

   Copyright 2001 Red Hat, Inc.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef UTMP_H
#define UTMP_H

#include <sys/types.h>
#include <time.h>
#include <paths.h>

#define UTMP_FILE _PATH_UTMP
#define WTMP_FILE _PATH_WTMP

#ifdef __cplusplus
extern "C" {
#endif

#define UT_LINESIZE	16
#define UT_NAMESIZE	16
#define UT_HOSTSIZE	256
#define UT_IDLEN	2
#define ut_name ut_user

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

#define RUN_LVL         1
#define BOOT_TIME       2
#define NEW_TIME        3
#define OLD_TIME        4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8

extern struct utmp *_getutline (struct utmp *);
extern struct utmp *getutent (void);
extern struct utmp *getutid (struct utmp *);
extern struct utmp *getutline (struct utmp *);
extern void endutent (void);
extern void pututline (struct utmp *);
extern void setutent (void);
extern void utmpname (const char *);

void login (struct utmp *);
int logout (char *);
int login_tty (int);
void logwtmp (char *, char *, char *);

#ifdef __cplusplus
}
#endif
#endif /* UTMP_H */
