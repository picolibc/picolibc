/* Operating system stubs, set up for the MRI simulator */

#include <_ansi.h>
#include <errno.h>

int _isatty(int file)
{
  return 1;
}

int
_unlink (const char *path)
{
  errno = EIO;
  return -1;
}
