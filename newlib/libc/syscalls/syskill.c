/* connector for kill */

#include <reent.h>
#include <signal.h>

int
_DEFUN (kill, (pid, sig),
     int pid _AND
     int sig)
{
  return _kill_r (_REENT, pid, sig);
}
