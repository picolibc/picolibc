/* connector for times */

#include <reent.h>
#include <sys/times.h>

clock_t
times (struct tms *buf)
{
  return _times_r (_REENT, buf);
}
