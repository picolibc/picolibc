/* types.h

   Copyright 2001, 2002 Red Hat Inc.
   Written by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _CYGWIN_TYPES_H
#define _CYGWIN_TYPES_H

typedef long __off32_t;
typedef long long __off64_t;
#ifdef __CYGWIN_USE_BIG_TYPES__
typedef __off64_t off_t;
#else
typedef __off32_t off_t;
#endif

typedef long blksize_t;

typedef long __blkcnt32_t;
typedef long long __blkcnt64_t;
#ifdef __CYGWIN_USE_BIG_TYPES__
typedef __blkcnt64_t  blkcnt_t;
#else
typedef __blkcnt32_t  blkcnt_t;
#endif

typedef unsigned short __uid16_t;
typedef unsigned short __gid16_t;
typedef unsigned long  __uid32_t;
typedef unsigned long  __gid32_t;
#ifdef __CYGWIN_USE_BIG_TYPES__
typedef __uid32_t uid_t;
typedef __gid32_t gid_t;
#else
typedef __uid16_t uid_t;
typedef __gid16_t gid_t;
#endif

#if !defined(__INSIDE_CYGWIN__) || !defined(__cplusplus)

typedef void *pthread_t;
typedef void *pthread_mutex_t;

typedef void *pthread_key_t;
typedef void *pthread_attr_t;
typedef void *pthread_mutexattr_t;
typedef void *pthread_condattr_t;
typedef void *pthread_cond_t;

  /* These variables are not user alterable. This means you!. */
typedef struct
{
  pthread_mutex_t mutex;
  int state;
}
pthread_once_t;
typedef void *pthread_rwlock_t;
typedef void *pthread_rwlockattr_t;

#else

/* pthreads types */

typedef class pthread *pthread_t;
typedef class pthread_mutex *pthread_mutex_t;
typedef class pthread_key *pthread_key_t;
typedef class pthread_attr *pthread_attr_t;
typedef class pthread_mutexattr *pthread_mutexattr_t;
typedef class pthread_condattr *pthread_condattr_t;
typedef class pthread_cond *pthread_cond_t;
typedef class pthread_once pthread_once_t;
typedef class pthread_rwlock *pthread_rwlock_t;
typedef class pthread_rwlockattr *pthread_rwlockattr_t;

/* semaphores types */
typedef class semaphore *sem_t;
#endif /* __INSIDE_CYGWIN__ */
#endif /* _CYGWIN_TYPES_H */

#ifdef __cplusplus
}
#endif
