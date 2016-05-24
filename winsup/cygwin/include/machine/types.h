/* machine/types.h
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_TYPES_H
#error "must be included via <sys/types.h>"
#endif /* !_SYS_TYPES_H */

#ifdef __cplusplus
extern "C"
{
#endif

#include <endian.h>
#include <bits/wordsize.h>
#include <sys/_timespec.h>

#ifndef __timespec_t_defined
#define __timespec_t_defined
typedef struct timespec timespec_t;
#endif /*__timespec_t_defined*/

#ifndef __timestruc_t_defined
#define __timestruc_t_defined
typedef struct timespec timestruc_t;
#endif /*__timestruc_t_defined*/

typedef __loff_t loff_t;

#if defined (__INSIDE_CYGWIN__) && !defined (__x86_64__)
struct __flock32 {
	short	 l_type;	/* F_RDLCK, F_WRLCK, or F_UNLCK */
	short	 l_whence;	/* flag to choose starting offset */
	_off_t	 l_start;	/* relative offset, in bytes */
	_off_t	 l_len;		/* length, in bytes; 0 means lock to EOF */
	short	 l_pid;		/* returned with F_GETLK */
	short	 l_xxx;		/* reserved for future use */
};
#endif

struct flock {
	short	 l_type;	/* F_RDLCK, F_WRLCK, or F_UNLCK */
	short	 l_whence;	/* flag to choose starting offset */
	off_t	 l_start;	/* relative offset, in bytes */
	off_t	 l_len;		/* length, in bytes; 0 means lock to EOF */
	pid_t	 l_pid;		/* returned with F_GETLK */
};

#ifndef __BIT_TYPES_DEFINED
#define __BIT_TYPES_DEFINED__ 1

#ifndef __vm_offset_t_defined
#define __vm_offset_t_defined
typedef unsigned long vm_offset_t;
#endif /*__vm_offset_t_defined*/

#ifndef __vm_size_t_defined
#define __vm_size_t_defined
typedef unsigned long vm_size_t;
#endif /*__vm_size_t_defined*/

#ifndef __vm_object_t_defined
#define __vm_object_t_defined
typedef void *vm_object_t;
#endif /* __vm_object_t_defined */

#ifndef __addr_t_defined
#define __addr_t_defined
typedef char *addr_t;
#endif

#endif /*__BIT_TYPES_DEFINED*/

#if !defined(__INSIDE_CYGWIN__) || !defined(__cplusplus)

typedef struct __pthread_t {char __dummy;} *pthread_t;
typedef struct __pthread_mutex_t {char __dummy;} *pthread_mutex_t;

typedef struct __pthread_key_t {char __dummy;} *pthread_key_t;
typedef struct __pthread_attr_t {char __dummy;} *pthread_attr_t;
typedef struct __pthread_mutexattr_t {char __dummy;} *pthread_mutexattr_t;
typedef struct __pthread_condattr_t {char __dummy;} *pthread_condattr_t;
typedef struct __pthread_cond_t {char __dummy;} *pthread_cond_t;
typedef struct __pthread_barrierattr_t {char __dummy;} *pthread_barrierattr_t;
typedef struct __pthread_barrier_t {char __dummy;} *pthread_barrier_t;

  /* These variables are not user alterable. This means you!. */
typedef struct
{
  pthread_mutex_t mutex;
  int state;
}
pthread_once_t;
typedef struct __pthread_spinlock_t {char __dummy;} *pthread_spinlock_t;
typedef struct __pthread_rwlock_t {char __dummy;} *pthread_rwlock_t;
typedef struct __pthread_rwlockattr_t {char __dummy;} *pthread_rwlockattr_t;

#else

/* pthreads types */

typedef class pthread *pthread_t;
typedef class pthread_mutex *pthread_mutex_t;
typedef class pthread_key *pthread_key_t;
typedef class pthread_attr *pthread_attr_t;
typedef class pthread_mutexattr *pthread_mutexattr_t;
typedef class pthread_condattr *pthread_condattr_t;
typedef class pthread_cond *pthread_cond_t;
typedef class pthread_barrier *pthread_barrier_t;
typedef class pthread_barrierattr *pthread_barrierattr_t;
typedef class pthread_once pthread_once_t;
typedef class pthread_spinlock *pthread_spinlock_t;
typedef class pthread_rwlock *pthread_rwlock_t;
typedef class pthread_rwlockattr *pthread_rwlockattr_t;

/* semaphores types */
typedef class semaphore *sem_t;
#endif /* __INSIDE_CYGWIN__ */

/* this header needs the dev_t typedef */
#include <sys/sysmacros.h>

#ifdef __cplusplus
}
#endif
