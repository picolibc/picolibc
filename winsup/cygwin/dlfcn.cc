/* dlfcn.cc

   Copyright 1998, 2000, 2001, 2002, 2003, 2004, 2008, 2009 Red Hat, Inc.

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
static bool __stdcall
get_full_path_of_dll (const char* str, path_conv &real_filename)
{
  int len = strlen (str);

  /* empty string? */
  if (len == 0)
    {
      set_errno (EINVAL);
      return false;		/* Yes.  Let caller deal with it. */
    }

  tmp_pathbuf tp;
  char *name = tp.c_get ();

  strcpy (name, str);	/* Put it somewhere where we can manipulate it. */

  if (isabspath (name) ||
      (check_path_access ("LD_LIBRARY_PATH=", name, real_filename)
       ?: check_path_access ("/usr/lib", name, real_filename)) == NULL)
    real_filename.check (name, PC_SYM_FOLLOW | PC_NOFULL | PC_NULLEMPTY);

  if (!real_filename.error)
    return true;

  set_errno (real_filename.error);
  return false;
}

void *
dlopen (const char *name, int)
{
  void *ret;

  if (name == NULL)
    ret = (void *) GetModuleHandle (NULL); /* handle for the current module */
  else
    {
      /* handle for the named library */
      path_conv pc;
      if (!get_full_path_of_dll (name, pc))
	ret = NULL;
      else
	{
	  tmp_pathbuf tp;
	  wchar_t *path = tp.w_get ();

	  pc.get_wide_win32_path (path);
	  /* Check if the last path component contains a dot.  If so,
	     leave the filename alone.  Otherwise add a traiing dot
	     to override LoadLibrary's automatic adding of a ".dll" suffix. */
	  wchar_t *last_bs = wcsrchr (path, L'\\');
	  if (last_bs && !wcschr (last_bs, L'.'))
	    wcscat (last_bs, L".");

	  /* Workaround for broken DLLs built against Cygwin versions 1.7.0-49
	     up to 1.7.0-57.  They override the cxx_malloc pointer in their
	     DLL initialization code even if loaded dynamically.  This is a
	     no-no since a later dlclose lets cxx_malloc point into nirvana.
	     The below kludge "fixes" that by reverting the original cxx_malloc
	     pointer after LoadLibrary.  This implies that their overrides
	     won't be applied; that's OK.  All overrides should be present at
	     final link time, as Windows doesn't allow undefined references;
	     it would actually be wrong for a dlopen'd DLL to opportunistically
	     override functions in a way that wasn't known then.  We're not
	     going to try and reproduce the full ELF dynamic loader here!  */

	  /* Store original cxx_malloc pointer. */
	  struct per_process_cxx_malloc *tmp_malloc;
	  tmp_malloc = __cygwin_user_data.cxx_malloc;

	  ret = (void *) LoadLibraryW (path);

	  /* Restore original cxx_malloc pointer. */
	  __cygwin_user_data.cxx_malloc = tmp_malloc;

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
