/* cygwin/utmp.h

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#ifndef CYGWIN_UTMP_H
#define CYGWIN_UTMP_H

#include <sys/types.h>
#include <time.h>
#include <paths.h>

#define WTMP_FILE _PATH_WTMP

#ifdef __cplusplus
extern "C" {
#endif

#define UT_LINESIZE	16
#define UT_NAMESIZE	16
#define UT_HOSTSIZE	256
#define UT_IDLEN	2

#define RUN_LVL         1
#define BOOT_TIME       2
#define NEW_TIME        3
#define OLD_TIME        4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8

#ifdef __cplusplus
}
#endif
#endif /* CYGWIN_UTMP_H */
