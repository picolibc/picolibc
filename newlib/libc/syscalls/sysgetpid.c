/* connector for getpid */

#include <reent.h>
#include <unistd.h>

int
_DEFUN_VOID (getpid)
{
  return _getpid_r (_REENT);
}
