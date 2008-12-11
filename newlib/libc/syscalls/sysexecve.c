/* connector for execve */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (execve, (name, argv, env),
     _CONST char *name _AND
     char *_CONST argv[] _AND
     char *_CONST env[])
{
  return _execve_r (_REENT, name, argv, env);
}
