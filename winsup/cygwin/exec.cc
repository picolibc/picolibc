/* exec.cc: exec system call support.

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <process.h>
#include "fhandler.h"
#include "path.h"

/* This is called _execve and not execve because the real execve is defined
   in libc/posix/execve.c.  It calls us.  */

extern "C"
int
_execve (const char *path, char *const argv[], char *const envp[])
{
  static char *const empty_env[] = { 0 };
  MALLOC_CHECK;
  if (!envp)
    envp = empty_env;
  return _spawnve (NULL, _P_OVERLAY, path, argv, envp);
}

extern "C"
int
execl (const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[1024];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;
  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);
  va_end (args);
  MALLOC_CHECK;
  return _execve (path, (char * const  *) argv, cur_environ ());
}

extern "C"
int
execv (const char *path, char * const *argv)
{
  MALLOC_CHECK;
  return _execve (path, (char * const *) argv, cur_environ ());
}

/* the same as a standard exec() calls family, but with NT security support */

extern "C"
pid_t
sexecve (HANDLE hToken, const char *path, const char *const argv[],
	 const char *const envp[])
{
  _spawnve (hToken, _P_OVERLAY, path, argv, envp);
  return -1;
}

extern "C"
int
sexecl (HANDLE hToken, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[1024];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  va_end (args);

  MALLOC_CHECK;
  return sexecve (hToken, path, (char * const *) argv, cur_environ ());
}

extern "C"
int
sexecle (HANDLE hToken, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char * const *envp;
  const char *argv[1024];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
    argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  envp = va_arg (args, const char * const *);
  va_end (args);

  MALLOC_CHECK;
  return sexecve(hToken, path, (char * const *) argv, (char * const *) envp);
}

extern "C"
int
sexeclp (HANDLE hToken, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[1024];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  va_end (args);

  MALLOC_CHECK;
  return sexecvpe (hToken, path, (const char * const *) argv, cur_environ ());
}

extern "C"
int
sexeclpe (HANDLE hToken, const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char * const *envp;
  const char *argv[1024];

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;

  do
    argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);

  envp = va_arg (args, const char * const *);
  va_end (args);

  MALLOC_CHECK;
  return sexecvpe (hToken, path, argv, envp);
}

extern "C"
int
sexecv (HANDLE hToken, const char *path, const char * const *argv)
{
  MALLOC_CHECK;
  return sexecve (hToken, path, argv, cur_environ ());
}

extern "C"
int
sexecp (HANDLE hToken, const char *path, const char * const *argv)
{
  MALLOC_CHECK;
  return sexecvpe (hToken, path, argv, cur_environ ());
}

/*
 * Copy string, until c or <nul> is encountered.
 * NUL-terminate the destination string (s1).
 * Return pointer to terminating byte in dst string.
 */

char * __stdcall
strccpy (char *s1, const char **s2, char c)
{
  while (**s2 && **s2 != c)
    *s1++ = *((*s2)++);
  *s1 = 0;

  MALLOC_CHECK;
  return s1;
}

extern "C"
int
sexecvpe (HANDLE hToken, const char *file, const char * const *argv,
	  const char *const *envp)
{
  path_conv buf;
  MALLOC_CHECK;
  return sexecve (hToken, find_exec (file, buf), argv, envp);
}
