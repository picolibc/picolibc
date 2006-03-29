/* connector for read */

#include <reent.h>
#include <unistd.h>

_READ_WRITE_RETURN_TYPE
read (fd, buf, cnt)
     int fd;
     void *buf;
     size_t cnt;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _read_r (_REENT, fd, buf, cnt);
#else
  return _read (fd, buf, cnt);
#endif
}
