/* FIXME: dummy stub for now.  */
#include <errno.h>
#include <pwd.h>

struct passwd *
_DEFUN (getpwnam, (name),
	_CONST char *name)
{
  errno = ENOSYS;
  return NULL;
}

