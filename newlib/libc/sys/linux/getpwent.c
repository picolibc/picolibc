/* FIXME: dummy stub for now.  */
#include <errno.h>
#include <pwd.h>

struct passwd *
getpwnam (const char *name)
{
  errno = ENOSYS;
  return NULL;
}

/* FIXME: dummy stub for now.  */
struct passwd *
getpwuid (uid_t uid)
{
  errno = ENOSYS;
  return NULL;
}

/* FIXME: dummy stub for now.  */
struct passwd *
getpwent (uid_t uid)
{
  errno = ENOSYS;
  return NULL;
}

