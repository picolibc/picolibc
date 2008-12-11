/* connector for close */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (close, (fd),
     int fd)
{
  return _close_r (_REENT, fd);
}
