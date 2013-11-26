/* exec.cc: exec system call support.

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008,
   2009, 2011, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <process.h>
#include "cygerrno.h"
#include "path.h"
#include "environ.h"
#include "sync.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "winf.h"

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
  return spawnve (_P_OVERLAY, path, (char * const  *) argv, cur_environ ());
}

extern "C" int
execle (const char *path, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[1024];
  const char * const *envp;

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;
  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);
  envp = va_arg (args, const char * const *);
  va_end (args);
  MALLOC_CHECK;
  return spawnve (_P_OVERLAY, path, (char * const  *) argv, envp);
}

extern "C" int
execlp (const char *file, const char *arg0, ...)
{
  int i;
  va_list args;
  const char *argv[1024];
  path_conv buf;

  va_start (args, arg0);
  argv[0] = arg0;
  i = 1;
  do
      argv[i] = va_arg (args, const char *);
  while (argv[i++] != NULL);
  va_end (args);
  MALLOC_CHECK;
  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH=", FE_NNF) ?: "",
		  (char * const  *) argv, cur_environ ());
}

extern "C" int
execv (const char *path, char * const *argv)
{
  MALLOC_CHECK;
  return spawnve (_P_OVERLAY, path, argv, cur_environ ());
}

extern "C" int
execve (const char *path, char *const argv[], char *const envp[])
{
  MALLOC_CHECK;
  return spawnve (_P_OVERLAY, path, argv, envp);
}
EXPORT_ALIAS (execve, _execve)	/* For newlib */

extern "C" int
execvp (const char *file, char * const *argv)
{
  path_conv buf;

  MALLOC_CHECK;
  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH=", FE_NNF) ?: "",
		  argv, cur_environ ());
}

extern "C" int
execvpe (const char *file, char * const *argv, char *const *envp)
{
  path_conv buf;

  MALLOC_CHECK;
  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH=", FE_NNF) ?: "",
		  argv, envp);
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

  MALLOC_CHECK;
  return spawnve (_P_OVERLAY, cfd->pc.get_win32 (), argv, envp);
}

extern "C" pid_t
sexecve_is_bad ()
{
  set_errno (ENOSYS);
  return 0;
}
