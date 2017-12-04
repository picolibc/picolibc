/* connector for write */

#include <reent.h>
#include <unistd.h>

_READ_WRITE_RETURN_TYPE
write (int fd,
     const void *buf,
     size_t cnt)
{
  return _write_r (_REENT, fd, buf, cnt);
}
