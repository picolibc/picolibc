/* connector for execve */

#include <reent.h>
#include <unistd.h>

int
execve (const char *name,
     char *const argv[],
     char *const env[])
{
  return _execve_r (_REENT, name, argv, env);
}
