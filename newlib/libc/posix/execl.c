#ifndef _NO_EXECVE

/* execl.c */

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

/* Only deal with a pointer to environ, to work around subtle bugs with shared
   libraries and/or small data systems where the user declares his own
   'environ'.  */
static char ***p_environ = &environ;


#include <stdarg.h>

int
execl (const char *path,
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
  /* POSIX.1-2008 requires execl() to be callable from signal handlers while
     malloc() isn't, so we default to using alloca(). However, unbounded stack
     allocations is at a high risk of stack overflow and undefined behavior in
     environments without virtual memory and small stacks, so we let targets
     override the default and use malloc() */
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

  i = _execve (path, (char * const  *) argv, *p_environ);
#ifdef _EXECL_USE_MALLOC
  free (argv);
#endif
  return i;
}
#endif /* !_NO_EXECVE  */
