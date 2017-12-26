#include <machine/syscall.h>
#include <sys/time.h>

int
nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
  unsigned long current_time, end_time;
  asm ("rdtime %0" : "+r" (current_time));
  end_time = current_time + rqtp->tv_sec * 1000000000ULL + rqtp->tv_nsec;
  while (current_time <= end_time) asm ("rdtime %0" : "+r" (current_time));
  return 0;
}
