/* connector for lseek */

#include <reent.h>
#include <unistd.h>

off_t
lseek (int fd,
     off_t pos,
     int whence)
{
  return _lseek_r (_REENT, fd, pos, whence);
}
