/* connector for execve */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (execve, (name, argv, env),
     _CONST char *name,
     char *_CONST argv[],
     char *_CONST env[])
{
  return _execve_r (_REENT, name, argv, env);
}
