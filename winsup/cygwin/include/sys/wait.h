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

#ifdef __INSIDE_CYGWIN__

typedef int *__wait_status_ptr_t;

#elif defined(__cplusplus)

/* Attribute __transparent_union__ is only supported for C.  */
typedef void *__wait_status_ptr_t;

#else /* !__INSIDE_CYGWIN__ && !__cplusplus */

/* Allow `int' and `union wait' for the status.  */
typedef union
  {
    int *__int_ptr;
    union wait *__union_wait_ptr;
  } __wait_status_ptr_t  __attribute__ ((__transparent_union__));

#endif /* __INSIDE_CYGWIN__ */

pid_t wait (__wait_status_ptr_t __status);
pid_t waitpid (pid_t __pid, __wait_status_ptr_t __status, int __options);
pid_t wait3 (__wait_status_ptr_t __status, int __options, struct rusage *__rusage);
pid_t wait4 (pid_t __pid, __wait_status_ptr_t __status, int __options, struct rusage *__rusage);

#ifdef _COMPILING_NEWLIB
pid_t _wait (__wait_status_ptr_t __status);
#endif

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

extern "C++" {
inline int __wait_status_to_int (int __status)
  { return __status; }
inline int __wait_status_to_int (const union wait & __status)
  { return __status.w_status; }
}

#else /* !__cplusplus */

#define __wait_status_to_int(__status)  (__extension__ \
  (((union { __typeof(__status) __sin; int __sout; }) { .__sin = (__status) }).__sout))

#endif /* __cplusplus */

#endif /* _SYS_WAIT_H */
