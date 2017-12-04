/* connector for fstat */

#include <reent.h>
#include <sys/stat.h>
#include <unistd.h>

int
fstat (int fd,
     struct stat *pstat)
{
  return _fstat_r (_REENT, fd, pstat);
}
