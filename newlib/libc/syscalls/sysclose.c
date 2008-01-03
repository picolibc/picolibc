/* connector for close */

#include <reent.h>

int
_DEFUN (close, (fd),
     int fd)
{
  return _close_r (_REENT, fd);
}
