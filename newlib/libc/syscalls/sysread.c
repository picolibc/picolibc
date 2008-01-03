/* connector for read */

#include <reent.h>
#include <unistd.h>

_READ_WRITE_RETURN_TYPE
_DEFUN (read, (fd, buf, cnt),
     int fd _AND
     void *buf _AND
     size_t cnt)
{
  return _read_r (_REENT, fd, buf, cnt);
}
