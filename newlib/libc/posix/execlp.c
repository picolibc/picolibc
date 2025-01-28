#ifndef _NO_EXECVE

/* execlp.c */

/* This and the other exec*.c files in this directory require 
   the target to provide the _execve syscall.  */

#include <_ansi.h>
#include <unistd.h>
#ifdef _EXECL_USE_MALLOC
#include <errno.h>
#include <stdlib.h>
#else
#include <alloca.h>
#endif


#include <stdarg.h>

int
execlp (const char *path,
      const char *arg0, ...)


{
  int i;
  va_list args;
  const char **argv;

  i = 1;
  va_start (args, arg0);
  do
    i++;
  while (va_arg (args, const char *) != NULL);
  va_end (args);
#ifndef _EXECL_USE_MALLOC
  argv = alloca (i * sizeof(const char *));
#else
  argv = malloc (i * sizeof(const char *));
  if (argv == NULL)
  {
    errno = ENOMEM;
    return -1;
  }
#endif

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;
  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);
  va_end (args);

  i = execvp (path, (char * const *) argv);
#ifdef _EXECL_USE_MALLOC
  free (argv);
#endif
  return i;
}

#endif /* !_NO_EXECVE  */
