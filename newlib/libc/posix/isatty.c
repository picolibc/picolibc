/* isatty.c */

#include <unistd.h>
#include <reent.h>

int
_DEFUN(isatty, (fd), int fd)
{
  return _isatty (fd);
}
