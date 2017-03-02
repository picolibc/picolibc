/* dlfcn.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <psapi.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <ctype.h>
#include <wctype.h>
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "perprocess.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "ntdll.h"
#include "shared_info.h"
#include "dll_init.h"
#include "pathfinder.h"

/* Dumb allocator using memory from tmp_pathbuf.w_get ().

   Does not reuse free'd memory areas.  Instead, memory
   is released when the tmp_pathbuf goes out of scope.

   ATTENTION: Requesting memory from an instance of tmp_pathbuf breaks
   when another instance on a newer stack frame has provided memory. */
class tmp_pathbuf_allocator
  : public allocator_interface
{
  tmp_pathbuf & tp_;
  union
    {
      PWCHAR wideptr;
      void * voidptr;
      char * byteptr;
    }    freemem_;
  size_t freesize_;

  /* allocator_interface */
  virtual void * alloc (size_t need)
  {
    if (need == 0)
      need = 1; /* GNU-ish */
    size_t chunksize = NT_MAX_PATH * sizeof (WCHAR);
    if (need > chunksize)
      api_fatal ("temporary buffer too small for %d bytes", need);
    if (need > freesize_)
      {
	/* skip remaining, use next chunk */
	freemem_.wideptr = tp_.w_get ();
	freesize_ = chunksize;
      }

    void * ret = freemem_.voidptr;

    /* adjust remaining, aligned at 8 byte boundary */
    need = need + 7 - (need - 1) % 8;
    freemem_.byteptr += need;
    if (need > freesize_)
      freesize_ = 0;
    else
      freesize_ -= need;

    return ret;
  }

  /* allocator_interface */
  virtual void free (void *)
  {
    /* no-op: released by tmp_pathbuf at end of scope */
  }

  tmp_pathbuf_allocator ();
  tmp_pathbuf_allocator (tmp_pathbuf_allocator const &);
  tmp_pathbuf_allocator & operator = (tmp_pathbuf_allocator const &);

public:
  /* use tmp_pathbuf of current stack frame */
  tmp_pathbuf_allocator (tmp_pathbuf & tp)
    : allocator_interface ()
    , tp_ (tp)
    , freemem_ ()
    , freesize_ (0)
  {}
};

static void
set_dl_error (const char *str)
{
  strcpy (_my_tls.locals.dl_buffer, strerror (get_errno ()));
  _my_tls.locals.dl_error = 1;
}

/* Identify basename and baselen within name,
   return true if there is a dir in name. */
static bool
spot_basename (const char * &basename, int &baselen, const char * name)
{
  basename = strrchr (name, '/');
  basename = basename ? basename + 1 : name;
  baselen = name + strlen (name) - basename;
  return basename > name;
}

/* Setup basenames using basename and baselen,
   return true if basenames do have some suffix. */
static void
collect_basenames (pathfinder::basenamelist & basenames,
		   const char * basename, int baselen)
{
  /* like strrchr (basename, '.'), but limited to baselen */
  const char *suffix = basename + baselen;
  while (--suffix >= basename)
    if (*suffix == '.')
      break;

  int suffixlen;
  if (suffix >= basename)
    suffixlen = basename + baselen - suffix;
  else
    {
      suffixlen = 0;
      suffix = NULL;
    }

  char const * ext = "";
  /* Without some suffix, reserve space for a trailing dot to override
     GetModuleHandleExA's automatic adding of the ".dll" suffix. */
  int extlen = suffix ? 0 : 1;

  /* If we have the ".so" suffix, ... */
  if (suffixlen == 3 && !strncmp (suffix, ".so", 3))
    {
      /* ... keep the basename with original suffix, before ... */
      basenames.appendv (basename, baselen, NULL);
      /* ... replacing the ".so" with the ".dll" suffix. */
      baselen -= 3;
      ext = ".dll";
      extlen = 4;
    }
  /* If the basename starts with "lib", ... */
  if (!strncmp (basename, "lib", 3))
    {
      /* ... replace "lib" with "cyg", before ... */
      basenames.appendv ("cyg", 3, basename+3, baselen-3, ext, extlen, NULL);
    }
  /* ... using original basename with new suffix. */
  basenames.appendv (basename, baselen, ext, extlen, NULL);
}

/* Identify dir of current executable into exedirbuf using wpathbuf buffer.
   Return length of exedirbuf on success, or zero on error. */
static int
get_exedir (char * exedirbuf, wchar_t * wpathbuf)
{
  /* Unless we have a special cygwin loader, there is no such thing like
     DT_RUNPATH on Windows we can use to search for dlls, except for the
     directory of the main executable. */
  *exedirbuf = '\0';

  wchar_t * wlastsep = wcpcpy (wpathbuf, global_progname);
  /* like wcsrchr(L'\\'), but we know the wcslen already */
  while (--wlastsep > wpathbuf)
    if (*wlastsep == L'\\')
      break;
  if (wlastsep <= wpathbuf)
    return 0;
  *wlastsep = L'\0';

  if (mount_table->conv_to_posix_path (wpathbuf, exedirbuf, 0))
    return 0;

  return strlen (exedirbuf);
}

extern "C" void *
dlopen (const char *name, int flags)
{
  void *ret = NULL;

  do
    {
      if (name == NULL || *name == '\0')
	{
	  ret = (void *) GetModuleHandle (NULL); /* handle for the current module */
	  if (!ret)
	    __seterrno ();
	  break;
	}

      DWORD gmheflags = (flags & RTLD_NODELETE)
		      ?  GET_MODULE_HANDLE_EX_FLAG_PIN
		      : 0;

      tmp_pathbuf tp; /* single one per stack frame */
      tmp_pathbuf_allocator allocator (tp);
      pathfinder::basenamelist basenames (allocator);

      const char *basename;
      int baselen;
      bool have_dir = spot_basename (basename, baselen, name);
      collect_basenames (basenames, basename, baselen);

      /* handle for the named library */
      path_conv real_filename;
      wchar_t *wpath = tp.w_get ();
      char *cpath = tp.c_get ();

      pathfinder finder (allocator, basenames); /* eats basenames */

      if (have_dir)
	{
	  int dirlen = basename - 1 - name;

	  /* if the specified dir is x/lib, and the current executable
	     dir is x/bin, do the /lib -> /bin mapping, which is the
	     same actually as adding the executable dir */
	  if (dirlen >= 4 && !strncmp (name + dirlen - 4, "/lib", 4))
	    {
	      int exedirlen = get_exedir (cpath, wpath);
	      if (exedirlen == dirlen &&
		  !strncmp (cpath, name, dirlen - 4) &&
		  !strcmp (cpath + dirlen - 4, "/bin"))
		finder.add_searchdir (cpath, exedirlen);
	    }

	  /* search the specified dir */
	  finder.add_searchdir (name, dirlen);
	}
      else
	{
	  /* NOTE: The Windows loader (for linked dlls) does
	     not use the LD_LIBRARY_PATH environment variable. */
	  finder.add_envsearchpath ("LD_LIBRARY_PATH");

	  /* Finally we better have some fallback. */
	  finder.add_searchdir ("/usr/bin", 8);
	  finder.add_searchdir ("/usr/lib", 8);
	}

      /* now search the file system */
      if (!finder.find (pathfinder::
			exists_and_not_dir (real_filename,
					    PC_SYM_FOLLOW | PC_POSIX)))
	{
	  /* If nothing worked, create a relative path from the original
	     incoming filename and let LoadLibrary search for it using the
	     system default DLL search path. */
	  real_filename.check (name, PC_SYM_FOLLOW | PC_NOFULL | PC_NULLEMPTY);
	  if (real_filename.error)
	    break;
	}

      real_filename.get_wide_win32_path (wpath);
      /* Check if the last path component contains a dot.  If so,
	 leave the filename alone.  Otherwise add a trailing dot
	 to override LoadLibrary's automatic adding of a ".dll" suffix. */
      wchar_t *last_bs = wcsrchr (wpath, L'\\') ?: wpath;
      if (last_bs && !wcschr (last_bs, L'.'))
	wcscat (last_bs, L".");

      if (flags & RTLD_NOLOAD)
	{
	  GetModuleHandleExW (gmheflags, wpath, (HMODULE *) &ret);
	  if (ret)
	    break;
	}

#ifdef __i386__
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
#endif

      ret = (void *) LoadLibraryW (wpath);
      /* reference counting */
      if (ret)
	{
	  dll *d = dlls.find (ret);
	  if (d)
	    ++d->count;
	}

#ifdef __i386__
      /* Restore original cxx_malloc pointer. */
      __cygwin_user_data.cxx_malloc = tmp_malloc;
#endif

      if (ret && gmheflags)
	GetModuleHandleExW (gmheflags, wpath, (HMODULE *) &ret);

      if (!ret)
	__seterrno ();
    }
  while (0);

  if (!ret)
    set_dl_error ("dlopen");
  debug_printf ("ret %p", ret);

  return ret;
}

extern "C" void *
dlsym (void *handle, const char *name)
{
  void *ret = NULL;

  if (handle == RTLD_DEFAULT)
    { /* search all modules */
      PDEBUG_BUFFER buf;
      NTSTATUS status;

      buf = RtlCreateQueryDebugBuffer (0, FALSE);
      if (!buf)
	{
	  set_errno (ENOMEM);
	  set_dl_error ("dlsym");
	  return NULL;
	}
      status = RtlQueryProcessDebugInformation (GetCurrentProcessId (),
						PDI_MODULES, buf);
      if (!NT_SUCCESS (status))
	__seterrno_from_nt_status (status);
      else
	{
	  PDEBUG_MODULE_ARRAY mods = (PDEBUG_MODULE_ARRAY)
				     buf->ModuleInformation;
	  for (ULONG i = 0; i < mods->Count; ++i)
	    if ((ret = (void *)
		       GetProcAddress ((HMODULE) mods->Modules[i].Base, name)))
	      break;
	  if (!ret)
	    set_errno (ENOENT);
	}
      RtlDestroyQueryDebugBuffer (buf);
    }
  else
    {
      ret = (void *) GetProcAddress ((HMODULE) handle, name);
      if (!ret)
	__seterrno ();
    }
  if (!ret)
    set_dl_error ("dlsym");
  debug_printf ("ret %p", ret);
  return ret;
}

extern "C" int
dlclose (void *handle)
{
  int ret = 0;
  if (handle != GetModuleHandle (NULL))
    {
      /* reference counting */
      dll *d = dlls.find (handle);
      if (!d || d->count <= 0)
	{
	  errno = ENOENT;
	  ret = -1;
	}
      else
	{
	  --d->count;
	  if (!FreeLibrary ((HMODULE) handle))
	    {
	      __seterrno ();
	      ret = -1;
	    }
	}
    }
  if (ret)
    set_dl_error ("dlclose");
  return ret;
}

extern "C" char *
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

extern "C" int
dladdr (const void *addr, Dl_info *info)
{
  HMODULE hModule;
  BOOL ret = GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCSTR) addr,
				&hModule);
  if (!ret)
    return 0;

  /* Module handle happens to be equal to it's base load address. */
  info->dli_fbase = hModule;

  /* Get the module filename.  This pathname may be in short-, long- or //?/
     format, depending on how it was specified when loaded, but we assume this
     is always an absolute pathname. */
  WCHAR fname[MAX_PATH];
  DWORD length = GetModuleFileNameW (hModule, fname, MAX_PATH);
  if ((length == 0) || (length == MAX_PATH))
    return 0;

  /* Convert to a cygwin pathname */
  ssize_t conv = cygwin_conv_path (CCP_WIN_W_TO_POSIX | CCP_ABSOLUTE, fname,
				   info->dli_fname, MAX_PATH);
  if (conv)
    return 0;

  /* Always indicate no symbol matching addr could be found. */
  info->dli_sname = NULL;
  info->dli_saddr = NULL;

  return 1;
}
