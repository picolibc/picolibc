/* connector for fcntl */
/* only called from stdio/fdopen.c, so arg can be int. */

#include <reent.h>
#include <errno.h>

int
fcntl (int fd,
     int flag,
     int arg)
{
#ifdef HAVE_FCNTL
  return _fcntl_r (_REENT, fd, flag, arg);
#else /* !HAVE_FCNTL */
  errno = ENOSYS;
  return -1;
#endif /* !HAVE_FCNTL */
}
