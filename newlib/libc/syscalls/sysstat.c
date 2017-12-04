/* connector for stat */

#include <reent.h>
#include <sys/stat.h>
#include <unistd.h>

int
stat (const char *file,
     struct stat *pstat)
{
  return _stat_r (_REENT, file, pstat);
}
