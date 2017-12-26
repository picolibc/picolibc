#include <machine/syscall.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include "internal_syscall.h"

extern int _gettimeofday(struct timeval *, void *);

/* Timing information for current process. From
   newlib/libc/include/sys/times.h the tms struct fields are as follows:

   - clock_t tms_utime  : user clock ticks
   - clock_t tms_stime  : system clock ticks
   - clock_t tms_cutime : children's user clock ticks
   - clock_t tms_cstime : children's system clock ticks

   Since maven does not currently support processes we set both of the
   children's times to zero. Eventually we might want to separately
   account for user vs system time, but for now we just return the total
   number of cycles since starting the program.  */
clock_t
_times(struct tms *buf)
{
  // when called for the first time, initialize t0
  static struct timeval t0;
  if (t0.tv_sec == 0)
    _gettimeofday (&t0, 0);

  struct timeval t;
  _gettimeofday (&t, 0);

  long long utime = (t.tv_sec - t0.tv_sec) * 1000000 + (t.tv_usec - t0.tv_usec);
  buf->tms_utime = utime * CLOCKS_PER_SEC / 1000000;
  buf->tms_stime = buf->tms_cstime = buf->tms_cutime = 0;

  return -1;
}
