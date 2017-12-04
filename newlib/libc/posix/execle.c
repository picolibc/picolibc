#ifndef _NO_EXECVE

/* execle.c */

/* This and the other exec*.c files in this directory require 
   the target to provide the _execve syscall.  */

#include <_ansi.h>
#include <unistd.h>


#include <stdarg.h>

int
execle (const char *path,
      const char *arg0, ...)


{
  int i;
  va_list args;
  const char * const *envp;
  const char *argv[256];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;
  do
    argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);
  envp = va_arg (args, const char * const *);
  va_end (args);

  return _execve (path, (char * const *) argv, (char * const *) envp);
}

#endif /* !_NO_EXECVE  */
