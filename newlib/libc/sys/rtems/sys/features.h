/*
 *  This file lists the symbols which may be defined to indicate
 *  the presence of POSIX features subsets.  If defined, the 
 *  feature must be supported.
 *
 *  NOTE:  This file lists all feature constants.  The ones not supported
 *         should be commented out.
 *
 *  XXX: These are all "Compile-Time Symbolic Constants".  Need to 
 *       address "Execution-Time" ones.
 *
 *  $Id$
 */

#ifndef __RTEMS_POSIX_FEATURES_h
#define __RTEMS_POSIX_FEATURES_h

/*
 *  XXX: Temporary function so we can break when something that is
 *       not implemented is invoked.
 */

int POSIX_MP_NOT_IMPLEMENTED( void );
int POSIX_NOT_IMPLEMENTED( void );
int POSIX_BOTTOM_REACHED( void );

/****************************************************************************
 ****************************************************************************
 *                                                                          *
 *         P1003.1b-1993 defines the constants below this comment.          *
 *                                                                          *
 **************************************************************************** 
 ****************************************************************************/

/*
 *  Newlib may already have this set defined.
 */

#ifndef _POSIX_JOB_CONTROL
#define _POSIX_JOB_CONTROL
#endif

#ifndef _POSIX_SAVED_IDS
#define _POSIX_SAVED_IDS
#endif

#define _POSIX_ASYNCHRONOUS_IO
#define _POSIX_FSYNC
#define _POSIX_MAPPED_FILES
#define _POSIX_MEMLOCK
#define _POSIX_MEMLOCK_RANGE
#define _POSIX_MEMORY_PROTECTION
#define _POSIX_MESSAGE_PASSING
#define _POSIX_PRIORITIZED_IO
#define _POSIX_PRIORITY_SCHEDULING
#define _POSIX_REALTIME_SIGNALS
#define _POSIX_SEMAPHORES
#define _POSIX_SHARED_MEMORY_OBJECTS
#define _POSIX_SYNCHRONIZED_IO
#define _POSIX_TIMERS

/*
 *  This indicates the version number of the POSIX standard we are
 *  trying to be compliant with.
 *
 *  NOTE: Newlib may already have this set defined.
 */

#ifdef _POSIX_VERSION
#undef _POSIX_VERSION
#define _POSIX_VERSION           199309L
#endif

/****************************************************************************
 ****************************************************************************
 *                                                                          *
 *         P1003.1c/D10 defines the constants below this comment.           *
 *                                                                          *
 **************************************************************************** 
 ****************************************************************************/

#define _POSIX_THREADS
#define _POSIX_THREAD_ATTR_STACKADDR
#define _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_PRIORITY_SCHEDULING
#define _POSIX_THREAD_PRIO_INHERIT
#define _POSIX_THREAD_PRIO_PROTECT
#define _POSIX_THREAD_PROCESS_SHARED
#define _POSIX_THREAD_SAFE_FUNCTIONS

/****************************************************************************
 ****************************************************************************
 *                                                                          *
 *         P1003.4b/D8 defines the constants below this comment.            *
 *                                                                          *
 **************************************************************************** 
 ****************************************************************************/

#define _POSIX_SPAWN
#define _POSIX_TIMEOUTS
#define _POSIX_CPUTIME
#define _POSIX_THREAD_CPUTIME
#define _POSIX_SPORADIC_SERVER
#define _POSIX_THREAD_SPORADIC_SERVER
#define _POSIX_DEVICE_CONTROL
#define _POSIX_DEVCTL_DIRECTION
#define _POSIX_INTERRUPT_CONTROL
#define _POSIX_ADVISORY_INFO

#endif
/* end of include file */
