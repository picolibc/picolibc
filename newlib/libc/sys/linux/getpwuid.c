/* FIXME: dummy stub for now.  */
#include <errno.h>
#include <pwd.h>

struct passwd *
_DEFUN (getpwuid, (uid),
	uid_t uid)
{
  errno = ENOSYS;
  return NULL;
}

