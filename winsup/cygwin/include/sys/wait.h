/* sys/wait.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>
#include <sys/resource.h>
#include <cygwin/wait.h>

#ifdef __cplusplus
extern "C" {
#endif

pid_t wait (int *__status);
pid_t waitpid (pid_t __pid, int *__status, int __options);
pid_t wait3 (int *__status, int __options, struct rusage *__rusage);
pid_t wait4 (pid_t __pid, int *__status, int __options, struct rusage *__rusage);

#ifdef _LIBC
pid_t _wait (int *);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_WAIT_H */
