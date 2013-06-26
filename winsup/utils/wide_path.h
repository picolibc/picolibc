/* wide_path.h  -- Define class wide_path to convert multibyte win32 path
		   to wchar_t Win32 path including long path prefix if
		   necessary.

   Copyright 2009, 2011, 2013 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <stdlib.h>
#include <wchar.h>

class wide_path
{
  wchar_t *wp;

public:
  wide_path () : wp (NULL) {}
  wide_path (const char *mb_path, bool do_prefix = true)
  {
    int len = mbstowcs (NULL, mb_path, 0) + 1;
    wp = (wchar_t *) malloc ((len + 6) * sizeof (wchar_t));
    wchar_t *wp_p = wp;
    if (do_prefix && len >= MAX_PATH && strncmp (mb_path, "\\\\?\\", 4) != 0)
      {
	wcscpy (wp_p, L"\\\\?\\");
	wp_p += 4;
	if (strncmp (mb_path, "\\\\", 2) == 0)
	  {
	    wcscpy (wp_p, L"UNC");
	    wp_p += 3;
	    ++mb_path;
	    --len;
	  }
      }
    mbstowcs (wp_p, mb_path, len);
  }
  ~wide_path () { if (wp) free (wp); }
  operator const wchar_t *() const { return wp; }
};
