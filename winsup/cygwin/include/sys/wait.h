/* sys/wait.h

   Copyright 1997, 1998, 2001, 2002, 2003, 2004, 2006, 2011 Red Hat, Inc.

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

#ifdef __cplusplus

typedef int *__wait_status_ptr_t;

#else /* !__cplusplus */

/* Allow `int' and `union wait' for the status.  */
typedef union
  {
    int *__int_ptr;
    union wait *__union_wait_ptr;
  } __wait_status_ptr_t  __attribute__ ((__transparent_union__));

#endif /* __cplusplus */

pid_t wait (__wait_status_ptr_t __status);
pid_t waitpid (pid_t __pid, __wait_status_ptr_t __status, int __options);
pid_t wait3 (__wait_status_ptr_t __status, int __options, struct rusage *__rusage);
pid_t wait4 (pid_t __pid, __wait_status_ptr_t __status, int __options, struct rusage *__rusage);

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
}
#endif

/* Used in cygwin/wait.h, redefine to accept `int' and `union wait'.  */
#undef __wait_status_to_int

#ifdef __cplusplus

inline int __wait_status_to_int (int __status)
  { return __status; }
inline int __wait_status_to_int (const union wait & __status)
  { return __status.w_status; }

/* C++ wait() variants for `union wait'.  */
inline pid_t wait (union wait *__status)
  { return wait ((int *) __status); }
inline pid_t waitpid (pid_t __pid, union wait *__status, int __options)
  { return waitpid(__pid, (int *) __status, __options); }
inline pid_t wait3 (union wait *__status, int __options, struct rusage *__rusage)
  { return wait3 ((int *) __status, __options, __rusage); }
inline pid_t wait4 (pid_t __pid, union wait *__status, int __options, struct rusage *__rusage)
  { return wait4 (__pid, (int *) __status, __options, __rusage); }

#else /* !__cplusplus */

#define __wait_status_to_int(__status)  (__extension__ \
  (((union { __typeof(__status) __in; int __out; }) { .__in = (__status) }).__out))

#endif /* __cplusplus */

#endif /* _SYS_WAIT_H */
