/* dlfcn.cc

   Copyright 1998, 2000, 2001, 2002, 2003, 2004, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <psapi.h>
#include <stdlib.h>
#include <ctype.h>
#include "path.h"
#include "perprocess.h"
#include "dlfcn.h"
#include "cygtls.h"
#include "tls_pbuf.h"

static void __stdcall
set_dl_error (const char *str)
{
  strcpy (_my_tls.locals.dl_buffer, strerror (get_errno ()));
  _my_tls.locals.dl_error = 1;
}

/* Look for an executable file given the name and the environment
   variable to use for searching (eg., PATH); returns the full
   pathname (static buffer) if found or NULL if not. */
inline const char * __stdcall
check_path_access (const char *mywinenv, const char *name, path_conv& buf)
{
  return find_exec (name, buf, mywinenv, FE_NNF | FE_NATIVE | FE_CWD | FE_DLL);
}

/* Search LD_LIBRARY_PATH for dll, if it exists.
   Return Windows version of given path. */
static const char * __stdcall
get_full_path_of_dll (const char* str, char *name)
{
  int len = strlen (str);

  /* empty string or too long to be legal win32 pathname? */
  if (len == 0 || len >= PATH_MAX)
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
      (ret = check_path_access ("LD_LIBRARY_PATH=", name, real_filename)
	     ?: check_path_access ("/usr/lib", name, real_filename)) == NULL)
    real_filename.check (name, PC_SYM_FOLLOW | PC_NULLEMPTY);	/* Convert */

  if (!real_filename.error)
    ret = strcpy (name, real_filename.get_win32 ());
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
  void *ret;

  if (name == NULL)
    ret = (void *) GetModuleHandle (NULL); /* handle for the current module */
  else
    {
      tmp_pathbuf tp;
      char *buf = tp.c_get ();
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

  return ret;
}

void *
dlsym (void *handle, const char *name)
{
  void *ret = NULL;
  if (handle == RTLD_DEFAULT)
    { /* search all modules */
      HANDLE cur_proc = GetCurrentProcess ();
      HMODULE *modules;
      DWORD needed, i;
      if (!EnumProcessModules (cur_proc, NULL, 0, &needed))
	{
	dlsym_fail:
	  set_dl_error ("dlsym");
	  return NULL;
	}
      modules = (HMODULE*) alloca (needed);
      if (!EnumProcessModules (cur_proc, modules, needed, &needed))
	goto dlsym_fail;
      for (i = 0; i < needed / sizeof (HMODULE); i++)
	if ((ret = (void *) GetProcAddress (modules[i], name)))
	  break;
    }
  else
    ret = (void *) GetProcAddress ((HMODULE)handle, name);
  if (!ret)
    set_dl_error ("dlsym");
  debug_printf ("ret %p", ret);
  return ret;
}

int
dlclose (void *handle)
{
  int ret = -1;
  if (handle == GetModuleHandle (NULL) || FreeLibrary ((HMODULE) handle))
    ret = 0;
  if (ret)
    set_dl_error ("dlclose");
  return ret;
}

char *
dlerror ()
{
  char *res;
  if (!_my_tls.locals.dl_error)
    res = NULL;
  else
    {
      _my_tls.locals.dl_error = 0;
      res = _my_tls.locals.dl_buffer;
    }
  return res;
}
