/*
 *  $Id$
 */


#ifndef __POSIX_SYS_SCHEDULING_h
#define __POSIX_SYS_SCHEDULING_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/features.h>

#include <sys/types.h>
#include <sys/time.h>

/*
 *  13.2 Scheduling Policies, P1003.1b-1993, p. 250
 *
 *  NOTE:  SCHED_SPORADIC added by P1003.4b/D8, p. 34.
 */

#define SCHED_OTHER    0
#define SCHED_FIFO     1
#define SCHED_RR       2

#if defined(_POSIX_SPORADIC_SERVER)
#define SCHED_SPORADIC 3 
#endif

/*
 *  13.1 Scheduling Parameters, P1003.1b-1993, p. 249
 *
 *  NOTE:  Fields whose name begins with "ss_" added by P1003.4b/D8, p. 33.
 */

struct sched_param {
  int sched_priority;           /* Process execution scheduling priority */

#if defined(_POSIX_SPORADIC_SERVER)
  int ss_low_priority;          /* Low scheduling priority for sporadic */
                                /*   server */
  struct timespec ss_replenish_period; 
                                /* Replenishment period for sporadic server */
  struct timespec ss_initial_budget;   /* Initial budget for sporadic server */
#endif
};

#ifdef __cplusplus
}
#endif 

#endif
/* end of include file */

