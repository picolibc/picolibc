/*
 *  $Id$
 */

#include <sys/types.h>

#ifndef __POSIX_SYS_TIME_h
#define __POSIX_SYS_TIME_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Get the CPU dependent types for clock_t and time_t
 *
 *  NOTE:  These must be visible by including <time.h>.
 */

/* Get _CLOCK_T_ and _TIME_T_.  */
#include <machine/types.h>
 
#ifndef __clock_t_defined
typedef _CLOCK_T_ clock_t;
#define __clock_t_defined
#endif
 
#ifndef __time_t_defined
typedef _TIME_T_ time_t;
#define __time_t_defined
#endif

/*
 *  14.1.1 Time Value Specification Structures, P1003.1b-1993, p. 261
 */

struct timespec {
  time_t  tv_sec;   /* Seconds */
  long    tv_nsec;  /* Nanoseconds */
};

struct itimerspec {
  struct timespec  it_interval;  /* Timer period */
  struct timespec  it_value;     /* Timer expiration */
};

/* XXX should really be ifdef'ed */

/*
 *  BSD based stuff
 */

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

struct timeval {
  int tv_sec;
  int tv_usec;
};

int gettimeofday(
  struct timeval  *tp,
  struct timezone *tzp
);

/* Convenience macros for operations on timevals.
   NOTE: `timercmp' does not work for >= or <=.  */
#define        timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#define        timerclear(tvp)         ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define        timercmp(a, b, CMP)                                                   \
  (((a)->tv_sec == (b)->tv_sec) ?                                            \
   ((a)->tv_usec CMP (b)->tv_usec) :                                         \
   ((a)->tv_sec CMP (b)->tv_sec))
#define        timeradd(a, b, result)                                                \
  do {                                                                       \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                            \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                         \
    if ((result)->tv_usec >= 1000000)                                        \
      {                                                                              \
       ++(result)->tv_sec;                                                   \
       (result)->tv_usec -= 1000000;                                         \
      }                                                                              \
  } while (0)
#define        timersub(a, b, result)                                                \
  do {                                                                       \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                            \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                         \
    if ((result)->tv_usec < 0) {                                             \
      --(result)->tv_sec;                                                    \
      (result)->tv_usec += 1000000;                                          \
    }                                                                        \
  } while (0)
#ifdef __cplusplus
}
#endif 

#endif
/* end of include */
