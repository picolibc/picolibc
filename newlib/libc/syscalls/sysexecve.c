/* connector for execve */

#include <reent.h>

int
execve (name, argv, env)
     char *name;
     char **argv;
     char **env;
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _execve_r (_REENT, name, argv, env);
#else
  return _execve (name, argv, env);
#endif
}
