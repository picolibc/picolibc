/* isatty.c */

/* Dumb implementation so programs will at least run.  */

#include <sys/stat.h>
#include <reent.h>

int
_isatty_r (struct _reent *ptr, int fd)
{
  struct stat buf;

  if (_fstat_r (ptr, fd, &buf) < 0)
    return 0;
  if (S_ISCHR (buf.st_mode))
    return 1;
  return 0;
}
