/*
 *  XXX address -D__CPU__ versus -DCPU
 *
 *  $Id$
 */

#ifndef __POSIX_SYS_TYPES_h
#define __POSIX_SYS_TYPES_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/features.h>
#include <_ansi.h>
#include <stddef.h>
#include <machine/types.h>

/*
 *  Define __go32_types if we are in go32 configuration.  It will be undefined
 *  at the end of this file.
 */

#ifdef __i386__
#if !defined (__unix__) || defined (_WIN32)
#define __go32_types__
#endif
#endif

#ifndef  _POSIX_SOURCE
#define physadr   physadr_t
#define quad      quad_t
 
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned short  ushort;   /* System V compatibility */
typedef unsigned int    uint;     /* System V compatibility */
# endif /*!_POSIX_SOURCE */

typedef long  daddr_t;
typedef char *  caddr_t;
 
/*
 *  2.5: Primitive System Data Types, P1003.1b-1993, p. 32.
 */

#ifdef __go32_types__
typedef unsigned long ino_t;
#else
#ifdef __sparc__
typedef unsigned long ino_t;
#else
typedef unsigned short  ino_t;
#endif
#endif

typedef unsigned long long dev_t;/* device numbers 32-bit major and minor */
typedef long  off_t;             /* file sizes */
 
typedef unsigned short uid_t;    /* user IDs */
typedef unsigned short gid_t;    /* group IDs */

typedef int pid_t;               /* process and process group IDs */
typedef long key_t;
 
#ifdef __go32_types__
typedef char *addr_t;
typedef int   mode_t;
#else
#if defined (__sparc__) && !defined (__sparc_v9__)
#ifdef __svr4__
typedef unsigned long mode_t;            /* some file attributes */
#else
typedef unsigned short mode_t;           /* some file attributes */
#endif
#else
typedef unsigned mode_t;                /* some file attributes */
#endif
#endif /* ! __go32_types__ */
 
typedef unsigned int nlink_t;  /* link counts */
 
#ifndef __SIZE_T
#define __SIZE_T
typedef int size_t;            /* see C Standard XXX */
#endif

typedef int ssize_t;           /* count of bytes (memory space) or error */

/*
 *  This is just copied from the standard newlib <sys/types.h>
 */

# ifndef  _POSIX_SOURCE
 
#  define NBBY  8   /* number of bits in a byte */
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#  ifndef FD_SETSIZE
# define  FD_SETSIZE  60
#  endif
 
typedef long  fd_mask;
#define NFDBITS (sizeof (fd_mask) * NBBY) /* bits per mask */
#ifndef howmany
#define  howmany(x,y)  (((x)+((y)-1))/(y))
#endif
 
typedef struct fd_set {
  fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;
 
 
#define FD_SET(n, p)  ((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#define FD_CLR(n, p)  ((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#define FD_ZERO(p)  bzero((caddr_t)(p), sizeof (*(p)))
 
 
# endif /* _POSIX_SOURCE */

/*
 *  4.1.1 Get Process and Parent Process IDs, P1003.1b-1993, p. 83
 */

pid_t _EXFUN(getpid, (void) );
pid_t _EXFUN(getppid, (void) );

/*
 *  4.2.1 Get Real User, Effective User, Real Group, and Effective Group IDs, 
 *        P1003.1b-1993, p. 84
 */

#ifndef __go32_types__
uid_t _EXFUN(getuid, (void));
uid_t _EXFUN(geteuid, (void));
gid_t _EXFUN(getgid, (void));
gid_t _EXFUN(getegid, (void));
#endif

/*
 *  4.2.2 Set User and Group IDs, P1003.1b-1993, p. 84
 */

int _EXFUN(setuid, (uid_t uid));
int _EXFUN(setgid, (gid_t gid));

/*
 *  4.2.3 Get Supplementary IDs, P1003.1b-1993, p. 86
 */

int _EXFUN(getgroups, (int gidsetsize, gid_t grouplist[]) );

/*
 *  4.2.4 Get User Name, P1003.1b-1993, p. 87
 *
 *  NOTE:  P1003.1c/D10, p. 49 adds getlogin_r().
 */

char * _EXFUN(getlogin, (void) );

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
int _EXFUN(getlogin_r, (char *name, size_t namesize) );
#endif

/*
 *  4.3.1 Get Process Group IDs, P1003.1b-1993, p. 89
 */

pid_t  _EXFUN(getpgrp, (void) );

/*
 *  4.3.2 Create Session and Set Process Group ID, P1003.1b-1993, p. 88
 */

pid_t  _EXFUN(setsid, (void) );

/*
 *  4.3.3 Set Process Group ID for Job Control, P1003.1b-1993, p. 89
 */

int  _EXFUN(setpgid, (pid_t pid, pid_t pgid) );

/*
 *  14.1.3 Clock Type Definition, P1003.1b-1993, p. 262
 *
 *  Set the CPU dependent types for clockid_t and timer_t based on the
 *  unsigned 32 bit integer type defined in <sys/config.h>
 */
 
/* Set _CLOCK_T_ and _TIME_T_.  */
#define _CLOCKID_T_ __uint32_t
#define _TIMER_T_   __uint32_t
 
#ifndef __clockid_t_defined
typedef _CLOCKID_T_ clockid_t;
#define __clockid_t_defined
#endif
 
#ifndef __timer_t_defined
typedef _TIMER_T_ timer_t;
#define __timer_t_defined
#endif

/*
 *  Get rid of this variable
 */

#undef __go32_types__


#if defined(_POSIX_THREADS)

#include <sys/sched.h>

/*
 *  2.5 Primitive System Data Types,  P1003.1c/D10, p. 19.
 */

typedef __uint32_t pthread_t;            /* identify a thread */

/*
 *  XXX
 *
 *  NOTE: P1003.4b/D8, p. 54 adds cputime_clock_allowed attribute.
 */

/* P1003.1c/D10, p. 118-119 */
#define PTHREAD_SCOPE_PROCESS 0
#define PTHREAD_SCOPE_SYSTEM  1

/* P1003.1c/D10, p. 111 */
#define PTHREAD_INHERIT_SCHED  1      /* scheduling policy and associated */
                                      /*   attributes are inherited from */
                                      /*   the calling thread. */
#define PTHREAD_EXPLICIT_SCHED 2      /* set from provided attribute object */

/* P1003.1c/D10, p. 141 */
#define PTHREAD_CREATE_DETACHED 0
#define PTHREAD_CREATE_JOINABLE  1

typedef struct {
  int is_initialized;
  void *stackaddr;
  int stacksize;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;

#if defined(_POSIX_THREAD_CPUTIME)
  int  cputime_clock_allowed;  /* see time.h */
#endif
  int  detachstate;

} pthread_attr_t;

/*
 *  XXX
 */

#if defined(_POSIX_THREAD_PROCESS_SHARED)
/*
 *  NOTE: P1003.1c/D10, p. 81 defines following values for process_shared.
 */

#define PTHREAD_PROCESS_PRIVATE 0 /* visible within only the creating process */
#define PTHREAD_PROCESS_SHARED  1 /* visible too all processes with access to */
                                  /*   the memory where the resource is */
                                  /*   located */
#endif

/*
 *  13.6.1 Mutex Initialization Scheduling Attributes, P1003.1c/Draft 10, p. 128
 */
 
#if defined(_POSIX_THREAD_PRIO_PROTECT)
/*
 *  Values for protocol.
 */
 
#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2
#endif

/*
 * XXX
 */

typedef __uint32_t pthread_mutex_t;      /* identify a mutex */

typedef struct {
  int   is_initialized;
#if defined(_POSIX_THREAD_PROCESS_SHARED)
  int   process_shared;  /* allow mutex to be shared amongst processes */
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT)
  int   prio_ceiling;
  int   protocol;
#endif
  int   recursive;
} pthread_mutexattr_t;

/*
 *  XXX 
 */

typedef __uint32_t pthread_cond_t;       /* identify a condition variable */

typedef struct {
  int   is_initialized;
#if defined(_POSIX_THREAD_PROCESS_SHARED)
  int   process_shared;       /* allow this to be shared amongst processes */
#endif
} pthread_condattr_t;         /* a condition attribute object */

/*
 *  XXX 
 */

typedef __uint32_t pthread_key_t;        /* thread-specific data keys */

/*
 *  XXX 
 */

typedef struct {
  int   is_initialized;  /* is this structure initialized? */
  int   init_executed;   /* has the initialization routine been run? */
} pthread_once_t;       /* dynamic package initialization */

#endif /* defined(_POSIX_THREADS) */

#ifdef __cplusplus
}
#endif 

#endif
/* end of include file */
