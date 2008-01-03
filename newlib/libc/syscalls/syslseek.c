/* connector for lseek */

#include <reent.h>
#include <unistd.h>

off_t
_DEFUN (lseek, (fd, pos, whence),
     int fd _AND
     off_t pos _AND
     int whence)
{
  return _lseek_r (_REENT, fd, pos, whence);
}
