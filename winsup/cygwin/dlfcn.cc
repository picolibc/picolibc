/* dlfcn.cc

   Copyright 1998, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "fhandler.h"
#include "perprocess.h"
#include "path.h"
#include "thread.h"
#include "dlfcn.h"
#include "dll_init.h"
#include "cygerrno.h"

#define _dl_error _reent_winsup()->_dl_error
#define _dl_buffer _reent_winsup()->_dl_buffer

static void __stdcall
set_dl_error (const char *str)
{
  __small_sprintf (_dl_buffer, "%s: %E", str);
  _dl_error = 1;
}

/* Look for an executable file given the name and the environment
   variable to use for searching (eg., PATH); returns the full
   pathname (static buffer) if found or NULL if not. */
inline const char * __stdcall
check_path_access (const char *mywinenv, const char *name, path_conv& buf)
{
  return find_exec (name, buf, mywinenv, TRUE);
}

/* Search LD_LIBRARY_PATH for dll, if it exists.
   Return Windows version of given path. */
static const char * __stdcall
get_full_path_of_dll (const char* str, char *name)
{
  int len = strlen (str);

  /* empty string or too long to be legal win32 pathname? */
  if (len == 0 || len >= MAX_PATH - 1)
    return str;		/* Yes.  Let caller deal with it. */

  const char *ret;

  strcpy (name, str);	/* Put it somewhere where we can manipulate it. */

  /* Add extension if necessary */
  if (str[len - 1] != '.')
    {
      /* Add .dll only if no extension provided. */
      const char *p = strrchr (str, '.');
      if (!p || strpbrk (p, "\\/"))
	strcat (name, ".dll");
    }

  path_conv real_filename;

  if (isabspath (name) ||
      (ret = check_path_access ("LD_LIBRARY_PATH=", name, real_filename)) == NULL)
    real_filename.check (name);	/* Convert */

  if (!real_filename.error)
    ret = strcpy (name, real_filename);
  else
    {
      set_errno (real_filename.error);
      ret = NULL;
    }

  return ret;
}

void *
dlopen (const char *name, int)
{
  SetResourceLock(LOCK_DLL_LIST,READ_LOCK|WRITE_LOCK," dlopen");

  void *ret;

  if (name == NULL)
    ret = (void *) GetModuleHandle (NULL); /* handle for the current module */
  else
    {
      char buf[MAX_PATH];
      /* handle for the named library */
      const char *fullpath = get_full_path_of_dll (name, buf);
      if (!fullpath)
	ret = NULL;
      else
	{
	  ret = (void *) LoadLibrary (fullpath);
	  if (ret == NULL)
	    __seterrno ();
	}
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
  void *temphandle = (void *) GetModuleHandle (NULL);
  if (temphandle == handle || FreeLibrary ((HMODULE) handle))
    ret = 0;
  if (ret)
    set_dl_error ("dlclose");
  CloseHandle ((HMODULE) temphandle);

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
