/* connector for write */

#include <reent.h>
#include <unistd.h>

int
write (fd, buf, cnt)
     int fd;
     const void *buf;
     size_t cnt;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _write_r (_REENT, fd, buf, cnt);
#else
  return _write (fd, buf, cnt);
#endif
}
