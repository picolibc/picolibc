/* connector for wait */

#include <reent.h>

int
_DEFUN (wait, (status),
        int *status)
{
  return _wait_r (_REENT, status);
}
