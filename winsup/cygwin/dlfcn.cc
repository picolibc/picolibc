/* dlfcn.cc

   Copyright 1998, 2000 Cygnus Solutions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "winsup.h"
#include <ctype.h>
#include "dlfcn.h"
#include "dll_init.h"

#define _dl_error _reent_winsup()->_dl_error
#define _dl_buffer _reent_winsup()->_dl_buffer

static void __stdcall
set_dl_error (const char *str)
{
  __small_sprintf (_dl_buffer, "%s: %E", str);
  _dl_error = 1;
}

/* Check for existence of a file specified by the directory
   and name components. If successful, return a pointer the
   full pathname (static buffer), else return 0. */
static const char * __stdcall
check_access (const char *dir, const char *name)
{
  static char buf[MAX_PATH];
  const char *ret = 0;

  buf[0] = 0;
  strcpy (buf, dir);
  strcat (buf, "\\");
  strcat (buf, name);

  if (!access (buf, F_OK))
    ret = buf;
  return ret;
}

/* Look for an executable file given the name and the environment
   variable to use for searching (eg., PATH); returns the full
   pathname (static buffer) if found or NULL if not. */
static const char * __stdcall
check_path_access (const char *mywinenv, const char *name)
{
  path_conv buf;
  return find_exec (name, buf, mywinenv, TRUE);
}

/* Simulate the same search as LoadLibary + check environment
   variable LD_LIBRARY_PATH. If found, return the full pathname
   (static buffer); if illegal, return the input string unchanged
   and let the caller deal with it; return NULL otherwise.

   Note that this should never be called with a NULL string, since
   that is the introspective case, and the caller should not call
   this function at all.  */
static const char * __stdcall
get_full_path_of_dll (const char* str)
{
  int len = (str) ? strlen (str) : 0;

  /* NULL or empty string or too long to be legal win32 pathname? */
  if (len == 0 || len >= MAX_PATH - 1)
    return str;

  char buf[MAX_PATH];
  static char name[MAX_PATH];
  const char *ret = 0;

  strcpy (name, str);

  /* Add extension if necessary, but leave a trailing '.', if any, alone.
     Files with trailing '.'s are handled differently by win32 API. */
  if (str[len - 1] != '.')
    {
      /* Add .dll only if no extension provided. Handle various cases:
	   ./shlib		-->	./shlib.dll
	   ./dir/shlib.so	-->	./dir/shlib.so
	   shlib		-->	shlib.dll
	   shlib.dll		-->	shlib.dll
	   shlib.so		-->	shlib.so */
      const char *p = strrchr (str, '.');
      if (!p || isdirsep (p[1]))
	strcat (name, ".dll");
    }

  /* Deal with fully qualified filename right away. Do the actual
     conversion to win32 filename just before returning however. */
  if (isabspath (str))
    ret = name;

  /* current directory */
  if (!ret)
    {
      if (GetCurrentDirectory (MAX_PATH, buf) == 0)
	small_printf ("WARNING: get_full_path_of_dll can't get current directory win32 %E\n");
      else
	ret = check_access (buf, name);
    }

  /* LD_LIBRARY_PATH */
  if (!ret)
    ret = check_path_access ("LD_LIBRARY_PATH=", name);

  if (!ret)
    {
      if (GetSystemDirectory (buf, MAX_PATH) == 0)
	small_printf ("WARNING: get_full_path_of_dll can't get system directory win32 %E\n");
      else
	ret = check_access (buf, name);
    }

  /* 16 bits system directory */
  if (!ret && (os_being_run == winNT))
    {
      /* we assume last dir was xxxxx\SYSTEM32, so we remove 32 */
      len = strlen (buf);
      buf[len - 2] = 0;
      ret = check_access (buf, name);
    }

  /* windows directory */
  if (!ret)
    {
      if (GetWindowsDirectory (buf, MAX_PATH) == 0)
	small_printf ("WARNING: get_full_path_of_dll can't get Windows directory win32 %E\n");
      else
	ret = check_access (buf, name);
    }

  /* PATH */
  if (!ret)
    ret = check_path_access ("PATH=", name);

  /* Now do a final conversion to win32 pathname. This step is necessary
     to resolve symlinks etc so that win32 API finds the underlying file.  */
  if (ret)
    {
      path_conv real_filename (ret, SYMLINK_FOLLOW, 1);
      if (real_filename.error)
	ret = 0;
      else
	{
	  strcpy (name, real_filename.get_win32 ());
	  ret = name;
	}
    }
  return ret;
}

void *
dlopen (const char *name, int)
{
  SetResourceLock(LOCK_DLL_LIST,READ_LOCK|WRITE_LOCK," dlopen");

  void *ret = 0;

  if (!name)
    {
      /* handle for the current module */
      ret = (void *) GetModuleHandle (NULL);
    }
  else
    {
      /* handle for the named library */
      const char *fullpath = get_full_path_of_dll (name);
      ret = (void *) LoadLibrary (fullpath);
    }

  if (!ret)
    set_dl_error ("dlopen");
  debug_printf ("ret %p", ret);

  ReleaseResourceLock(LOCK_DLL_LIST,READ_LOCK|WRITE_LOCK," dlopen");
  return ret;
}

void *
dlsym (void *handle, const char *name)
{
  void *ret = (void *) GetProcAddress ((HMODULE) handle, name);
  if (!ret)
    set_dl_error ("dlsym");
  debug_printf ("ret %p", ret);
  return ret;
}

int
dlclose (void *handle)
{
  SetResourceLock(LOCK_DLL_LIST,READ_LOCK|WRITE_LOCK," dlclose");

  int ret = -1;
  if (FreeLibrary ((HMODULE) handle))
    ret = 0;
  if (ret)
    set_dl_error ("dlclose");

  ReleaseResourceLock(LOCK_DLL_LIST,READ_LOCK|WRITE_LOCK," dlclose");
  return ret;
}

char *
dlerror ()
{
  char *ret = 0;
  if (_dl_error)
    ret = _dl_buffer;
  return ret;
}
