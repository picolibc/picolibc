#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>	/* for time_t */

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
