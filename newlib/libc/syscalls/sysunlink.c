/* connector for unlink */

#include <reent.h>

int
unlink (file)
     char *file;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _unlink_r (_REENT, file);
#else
  return _unlink (file);
#endif
}
