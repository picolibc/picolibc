/* connector for getentropy */

#include <reent.h>
#include <sys/types.h>
#include <sys/time.h>

int
getentropy (void *buf,
     size_t buflen)
{
  return _getentropy_r (_REENT, buf, buflen);
}
