/* connector for wait */

#include <reent.h>
#include <sys/wait.h>

pid_t
_DEFUN (wait, (status),
        int *status)
{
  return _wait_r (_REENT, status);
}
