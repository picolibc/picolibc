/* connector for lseek */

#include <reent.h>
#include <unistd.h>

off_t
lseek (fd, pos, whence)
     int fd;
     off_t pos;
     int whence;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _lseek_r (_REENT, fd, pos, whence);
#else
  return _lseek (fd, pos, whence);
#endif
}
