/* utmpx.h

   Copyright 2004 Red Hat, Inc.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef UTMPX_H
#define UTMPX_H

#include <cygwin/utmp.h>
#include <sys/time.h>

#define UTMPX_FILE _PATH_UTMP

#ifdef __cplusplus
extern "C" {
#endif

/* Must be kept in sync with struct utmp as defined in sys/utmp.h! */
struct utmpx
{
 short	ut_type;	
 pid_t	ut_pid;		
 char	ut_line[UT_LINESIZE];
 char   ut_id[UT_IDLEN];
 time_t ut_time;	
 char	ut_user[UT_NAMESIZE];	
 char	ut_host[UT_HOSTSIZE];	
 long	ut_addr;	
 struct timeval ut_tv;
};

extern void endutxent (void);
extern struct utmpx *getutxent (void);
extern struct utmpx *getutxid (const struct utmpx *id);
extern struct utmpx *getutxline (const struct utmpx *line);
extern struct utmpx *pututxline (const struct utmpx *utmpx);
extern void setutxent (void);

#ifdef __cplusplus
}
#endif
#endif /* UTMPX_H */
