/* dirent.h: define struct __DIR

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_DIRENT_H
#define _CYGWIN_DIRENT_H

#include_next <dirent.h>

#define __DIRENT_COOKIE 0xcdcd8484

struct __DIR
{
  /* This is first to set alignment in non _LIBC case.  */
  unsigned long	__d_cookie;
  struct dirent	*__d_dirent;
  char		*__d_dirname;		/* use for internal caching */
  __int32_t	 __d_position;		/* used by telldir/seekdir */
  int		 __d_fd;
  uintptr_t	 __d_internal;
  void		*__handle;
  void		*__fh;
  unsigned	 __flags;
};

#endif /* _CYGWIN_DIRENT_H */
