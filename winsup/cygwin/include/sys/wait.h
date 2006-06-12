/* sys/wait.h

   Copyright 1997, 1998, 2001, 2002, 2003, 2004, 2006 Red Hat, Inc.

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

pid_t wait (int *);
pid_t waitpid (pid_t, int *, int);
pid_t wait3 (int *__status, int __options, struct rusage *__rusage);
pid_t wait4 (pid_t __pid, int *__status, int __options, struct rusage *__rusage);

union wait
  {
    int w_status;
    struct
      {
	unsigned int __w_termsig:7; /* Terminating signal.  */
	unsigned int __w_coredump:1; /* Set if dumped core.  */
	unsigned int __w_retcode:8; /* Return code if exited normally.  */
	unsigned int:16;
      } __wait_terminated;
    struct
      {
	unsigned int __w_stopval:8; /* W_STOPPED if stopped.  */
	unsigned int __w_stopsig:8; /* Stopping signal.  */
	unsigned int:16;
      } __wait_stopped;
  };

#define	w_termsig	__wait_terminated.__w_termsig
#define	w_coredump	__wait_terminated.__w_coredump
#define	w_retcode	__wait_terminated.__w_retcode
#define	w_stopsig	__wait_stopped.__w_stopsig
#define	w_stopval	__wait_stopped.__w_stopval

#ifdef __cplusplus
};
#endif

#endif
