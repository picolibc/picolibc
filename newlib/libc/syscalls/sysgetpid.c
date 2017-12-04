/* connector for getpid */

#include <reent.h>
#include <unistd.h>

int
getpid (void)
{
  return _getpid_r (_REENT);
}
