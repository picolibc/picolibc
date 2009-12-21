/* exec.cc: exec system call support.

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _execve __FOO_execve_
#include "winsup.h"
#include <process.h>
#include "cygerrno.h"
#include "path.h"
#include "environ.h"
#include "sync.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#undef _execve

/* This is called _execve and not execve because the real execve is defined
   in libc/posix/execve.c.  It calls us.  */

extern "C" int
execve (const char *path, char *const argv[], char *const envp[])
{
  static char *const empty_env[] = { 0 };
  MALLOC_CHECK;
  if (!envp)
    envp = empty_env;
  return spawnve (_P_OVERLAY, path, argv, envp);
}

EXPORT_ALIAS (execve, _execve)

extern "C" int
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
  return execve (path, (char * const  *) argv, cur_environ ());
}

extern "C" int
execv (const char *path, char * const *argv)
{
  MALLOC_CHECK;
  return execve (path, (char * const *) argv, cur_environ ());
}

extern "C" pid_t
sexecve_is_bad ()
{
  set_errno (ENOSYS);
  return 0;
}

/* Copy string, until c or <nul> is encountered.
   NUL-terminate the destination string (s1).
   Return pointer to terminating byte in dst string.  */

char * __stdcall
strccpy (char *s1, const char **s2, char c)
{
  while (**s2 && **s2 != c)
    *s1++ = *((*s2)++);
  *s1 = 0;

  MALLOC_CHECK;
  return s1;
}

extern "C" int
execvp (const char *path, char * const *argv)
{
  path_conv buf;
  return execv (find_exec (path, buf, "PATH=", FE_NNF) ?: "", argv);
}

extern "C" int
execvpe (const char *path, char * const *argv, char *const *envp)
{
  path_conv buf;
  return execve (find_exec (path, buf, "PATH=", FE_NNF) ?: "", argv, envp);
}

extern "C" int
fexecve (int fd, char * const *argv, char *const *envp)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    {
      syscall_printf ("-1 = fexecve (%d, %p, %p)", fd, argv, envp);
      return -1;
    }
  return execve (cfd->pc.get_win32 (), argv, envp);
}
