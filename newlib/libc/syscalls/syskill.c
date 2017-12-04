/* connector for kill */

#include <reent.h>
#include <signal.h>

int
kill (int pid,
     int sig)
{
  return _kill_r (_REENT, pid, sig);
}
