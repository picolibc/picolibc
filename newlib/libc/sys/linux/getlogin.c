/* FIXME: dummy stub for now.  */
#include <errno.h>
#include <unistd.h>

char *
getlogin (void)
{
  errno = ENOSYS;
  return NULL;
}

