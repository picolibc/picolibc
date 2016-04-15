#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/_types.h>

#if !defined(__time_t_defined) && !defined(_TIME_T_DECLARED)
typedef	_TIME_T_	time_t;
#define	__time_t_defined
#define	_TIME_T_DECLARED
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct timeval {
  long tv_sec;
  long tv_usec;
};

typedef struct timestruc {
  time_t tv_sec;
  long tv_nsec;
} timestruc_t;

#ifdef __cplusplus
}
#endif

#endif /* _SYS_TIME_H */
