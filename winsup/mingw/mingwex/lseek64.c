#include <io.h>

off64_t lseek64
(int fd, off64_t offset, int whence) 
{
  return _lseeki64(fd, (__int64) offset, whence);
}

