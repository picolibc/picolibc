/* exec.cc: exec system call support.

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
  return spawnve (_P_OVERLAY, path, (char * const  *) argv, environ);
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
  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH", FE_NNF) ?: "",
		  (char * const  *) argv, environ);
}

extern "C" int
execv (const char *path, char * const *argv)
{
  return spawnve (_P_OVERLAY, path, argv, environ);
}

extern "C" int
execve (const char *path, char *const argv[], char *const envp[])
{
  return spawnve (_P_OVERLAY, path, argv, envp);
}
EXPORT_ALIAS (execve, _execve)	/* For newlib */

extern "C" int
execvp (const char *file, char * const *argv)
{
  path_conv buf;

  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH", FE_NNF) ?: "",
		  argv, environ);
}

extern "C" int
execvpe (const char *file, char * const *argv, char *const *envp)
{
  path_conv buf;

  return spawnve (_P_OVERLAY | _P_PATH_TYPE_EXEC,
		  find_exec (file, buf, "PATH", FE_NNF) ?: "",
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

  return spawnve (_P_OVERLAY, cfd->pc.get_win32 (), argv, envp);
}

extern "C" pid_t
sexecve_is_bad ()
{
  set_errno (ENOSYS);
  return 0;
}
