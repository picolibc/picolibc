/* connector for execve */

#include <reent.h>

int
_DEFUN (execve, (name, argv, env),
     char *name _AND
     char **argv _AND
     char **env)
{
  return _execve_r (_REENT, name, argv, env);
}
