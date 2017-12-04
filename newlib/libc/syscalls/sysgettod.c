/* connector for gettimeofday */

#include <reent.h>
#include <sys/types.h>
#include <sys/time.h>

int
gettimeofday (struct timeval *ptimeval,
     void *ptimezone)
{
  return _gettimeofday_r (_REENT, ptimeval, ptimezone);
}
