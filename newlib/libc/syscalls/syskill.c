/* connector for kill */

#include <reent.h>
#include <signal.h>

int
_DEFUN (kill, (pid, sig),
     int pid,
     int sig)
{
  return _kill_r (_REENT, pid, sig);
}
