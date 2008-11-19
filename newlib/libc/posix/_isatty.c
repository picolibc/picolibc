/* isatty.c */

/* Dumb implementation so programs will at least run.  */

#include <sys/stat.h>
#include <errno.h>

int
_DEFUN(_isatty, (fd), int fd)
{
  struct stat buf;

  if (fstat (fd, &buf) < 0) {
    errno = EBADF;
    return 0;
  }
  if (S_ISCHR (buf.st_mode))
    return 1;
  errno = ENOTTY;
  return 0;
}
