#include <machine/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include "internal_syscall.h"

/* Get the current time.  Only relatively correct.  */
int
_gettimeofday(struct timeval *tp, void *tzp)
{
#if __riscv_xlen == 32
  struct __timespec64
  {
    int64_t tv_sec;         /* Seconds */
# if BYTE_ORDER == BIG_ENDIAN
    int32_t __padding;      /* Padding */
    int32_t tv_nsec;        /* Nanoseconds */
# else
    int32_t tv_nsec;        /* Nanoseconds */
    int32_t __padding;      /* Padding */
# endif
  };
  struct __timespec64 ts64;
  int rv;
  rv = syscall_errno (SYS_clock_gettime64, 2, 0, (long)&ts64, 0, 0, 0, 0);
  tp->tv_sec = ts64.tv_sec;
  tp->tv_usec = ts64.tv_nsec * 1000;
  return rv;
#else
  return syscall_errno (SYS_gettimeofday, 1, tp, 0, 0, 0, 0, 0);
#endif
}
