/* connector for times */

#include <reent.h>
#include <sys/times.h>

clock_t
_DEFUN (times, (buf),
     struct tms *buf)
{
  return _times_r (_REENT, buf);
}
