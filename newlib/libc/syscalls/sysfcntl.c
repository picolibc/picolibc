/* connector for fcntl */
/* only called from stdio/fdopen.c, so arg can be int. */

#include <reent.h>

int
fcntl (fd, flag, arg)
     int fd;
     int flag;
     int arg;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _fcntl_r (_REENT, fd, flag, arg);
#else
  return _fcntl (fd, flag, arg);
#endif
}
