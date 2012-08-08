/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* This is file MKTEMP.C */
/* This file may have been modified by DJ Delorie (Jan 1991).  If so,
** these modifications are Copyright (C) 1991 DJ Delorie.
*/

/*
FUNCTION
<<mktemp>>, <<mkstemp>>, <<mkostemp>>, <<mkstemps>>,
<<mkostemps>>---generate unused file name
<<mkdtemp>>---generate unused directory

INDEX
	mktemp
INDEX
	mkdtemp
INDEX
	mkstemp
INDEX
	mkstemps
INDEX
	mkostemp
INDEX
	mkostemps
INDEX
	_mktemp_r
INDEX
	_mkdtemp_r
INDEX
	_mkstemp_r
INDEX
	_mkstemps_r
INDEX
	_mkostemp_r
INDEX
	_mkostemps_r

ANSI_SYNOPSIS
	#include <stdlib.h>
	char *mktemp(char *<[path]>);
	char *mkdtemp(char *<[path]>);
	int mkstemp(char *<[path]>);
	int mkstemps(char *<[path]>, int <[suffixlen]>);
	int mkostemp(char *<[path]>, int <[flags]>);
	int mkostemps(char *<[path]>, int <[suffixlen]>, int <[flags]>);

	char *_mktemp_r(struct _reent *<[reent]>, char *<[path]>);
	char *_mkdtemp_r(struct _reent *<[reent]>, char *<[path]>);
	int *_mkstemp_r(struct _reent *<[reent]>, char *<[path]>);
	int *_mkstemps_r(struct _reent *<[reent]>, char *<[path]>, int <[len]>);
	int *_mkostemp_r(struct _reent *<[reent]>, char *<[path]>,
			 int <[flags]>);
	int *_mkostemps_r(struct _reent *<[reent]>, char *<[path]>, int <[len]>,
			  int <[flags]>);

DESCRIPTION
<<mktemp>>, <<mkstemp>>, and <<mkstemps>> attempt to generate a file name
that is not yet in use for any existing file.  <<mkstemp>> and <<mkstemps>>
create the file and open it for reading and writing; <<mktemp>> simply
generates the file name (making <<mktemp>> a security risk).  <<mkostemp>>
and <<mkostemps>> allow the addition of other <<open>> flags, such
as <<O_CLOEXEC>>, <<O_APPEND>>, or <<O_SYNC>>.  On platforms with a
separate text mode, <<mkstemp>> forces <<O_BINARY>>, while <<mkostemp>>
allows the choice between <<O_BINARY>>, <<O_TEXT>>, or 0 for default.
<<mkdtemp>> attempts to create a directory instead of a file, with a
permissions mask of 0700.

You supply a simple pattern for the generated file name, as the string
at <[path]>.  The pattern should be a valid filename (including path
information if you wish) ending with at least six `<<X>>'
characters.  The generated filename will match the leading part of the
name you supply, with the trailing `<<X>>' characters replaced by some
combination of digits and letters.  With <<mkstemps>>, the `<<X>>'
characters end <[suffixlen]> bytes before the end of the string.

The alternate functions <<_mktemp_r>>, <<_mkdtemp_r>>, <<_mkstemp_r>>,
<<_mkostemp_r>>, <<_mkostemps_r>>, and <<_mkstemps_r>> are reentrant
versions.  The extra argument <[reent]> is a pointer to a reentrancy
structure.

RETURNS
<<mktemp>> returns the pointer <[path]> to the modified string
representing an unused filename, unless it could not generate one, or
the pattern you provided is not suitable for a filename; in that case,
it returns <<NULL>>.  Be aware that there is an inherent race between
generating the name and attempting to create a file by that name;
you are advised to use <<O_EXCL|O_CREAT>>.

<<mkdtemp>> returns the pointer <[path]> to the modified string if the
directory was created, otherwise it returns <<NULL>>.

<<mkstemp>>, <<mkstemps>>, <<mkostemp>>, and <<mkostemps>> return a file
descriptor to the newly created file, unless it could not generate an
unused filename, or the pattern you provided is not suitable for a
filename; in that case, it returns <<-1>>.

NOTES
Never use <<mktemp>>.  The generated filenames are easy to guess and
there's a race between the test if the file exists and the creation
of the file.  In combination this makes <<mktemp>> prone to attacks
and using it is a security risk.  Whenever possible use <<mkstemp>>
instead.  It doesn't suffer the race condition.

PORTABILITY
ANSI C does not require either <<mktemp>> or <<mkstemp>>; the System
V Interface Definition requires <<mktemp>> as of Issue 2.  POSIX 2001
requires <<mkstemp>>, and POSIX 2008 requires <<mkdtemp>> while
deprecating <<mktemp>>.  <<mkstemps>>, <<mkostemp>>, and <<mkostemps>>
are not standardized.

Supporting OS subroutines required: <<getpid>>, <<mkdir>>, <<open>>, <<stat>>.
*/

#include <_ansi.h>
#include <stdlib.h>
#include <reent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

static int
_DEFUN(_gettemp, (ptr, path, doopen, domkdir, suffixlen, flags),
       struct _reent *ptr _AND
       char *path         _AND
       register int *doopen _AND
       int domkdir        _AND
       size_t suffixlen   _AND
       int flags)
{
  register char *start, *trv;
  char *end;
#ifdef __USE_INTERNAL_STAT64
  struct stat64 sbuf;
#else
  struct stat sbuf;
#endif
  unsigned int pid;

  pid = _getpid_r (ptr);
  for (trv = path; *trv; ++trv)		/* extra X's get set to 0's */
    continue;
  if (trv - path < suffixlen)
    {
      ptr->_errno = EINVAL;
      return 0;
    }
  trv -= suffixlen;
  end = trv;
  while (path < trv && *--trv == 'X')
    {
      *trv = (pid % 10) + '0';
      pid /= 10;
    }
  if (end - trv < 6)
    {
      ptr->_errno = EINVAL;
      return 0;
    }

  /*
   * Check the target directory; if you have six X's and it
   * doesn't exist this runs for a *very* long time.
   */

  for (start = trv + 1;; --trv)
    {
      if (trv <= path)
	break;
      if (*trv == '/')
	{
	  *trv = '\0';
#ifdef __USE_INTERNAL_STAT64
	  if (_stat64_r (ptr, path, &sbuf))
#else
	  if (_stat_r (ptr, path, &sbuf))
#endif
	    return (0);
	  if (!(sbuf.st_mode & S_IFDIR))
	    {
	      ptr->_errno = ENOTDIR;
	      return (0);
	    }
	  *trv = '/';
	  break;
	}
    }

  for (;;)
    {
#if !defined _ELIX_LEVEL || _ELIX_LEVEL >= 4
      if (domkdir)
	{
#ifdef HAVE_MKDIR
	  if (_mkdir_r (ptr, path, 0700) == 0)
	    return 1;
	  if (ptr->_errno != EEXIST)
	    return 0;
#else /* !HAVE_MKDIR */
	  ptr->_errno = ENOSYS;
	  return 0;
#endif /* !HAVE_MKDIR */
	}
      else
#endif /* _ELIX_LEVEL */
      if (doopen)
	{
	  if ((*doopen = _open_r (ptr, path, O_CREAT | O_EXCL | O_RDWR | flags,
				  0600)) >= 0)
	    return 1;
	  if (ptr->_errno != EEXIST)
	    return 0;
	}
#ifdef __USE_INTERNAL_STAT64
      else if (_stat64_r (ptr, path, &sbuf))
#else
      else if (_stat_r (ptr, path, &sbuf))
#endif
	return (ptr->_errno == ENOENT ? 1 : 0);

      /* tricky little algorithm for backward compatibility */
      for (trv = start;;)
	{
	  if (trv == end)
	    return 0;
	  if (*trv == 'z')
	    *trv++ = 'a';
	  else
	    {
	      /* Safe, since it only encounters 7-bit characters.  */
	      if (isdigit ((unsigned char) *trv))
		*trv = 'a';
	      else
		++ * trv;
	      break;
	    }
	}
    }
  /*NOTREACHED*/
}

#ifndef O_BINARY
# define O_BINARY 0
#endif

int
_DEFUN(_mkstemp_r, (ptr, path),
       struct _reent *ptr _AND
       char *path)
{
  int fd;

  return (_gettemp (ptr, path, &fd, 0, 0, O_BINARY) ? fd : -1);
}

#if !defined _ELIX_LEVEL || _ELIX_LEVEL >= 4
char *
_DEFUN(_mkdtemp_r, (ptr, path),
       struct _reent *ptr _AND
       char *path)
{
  return (_gettemp (ptr, path, (int *) NULL, 1, 0, 0) ? path : NULL);
}

int
_DEFUN(_mkstemps_r, (ptr, path, len),
       struct _reent *ptr _AND
       char *path _AND
       int len)
{
  int fd;

  return (_gettemp (ptr, path, &fd, 0, len, O_BINARY) ? fd : -1);
}

int
_DEFUN(_mkostemp_r, (ptr, path, flags),
       struct _reent *ptr _AND
       char *path _AND
       int flags)
{
  int fd;

  return (_gettemp (ptr, path, &fd, 0, 0, flags & ~O_ACCMODE) ? fd : -1);
}

int
_DEFUN(_mkostemps_r, (ptr, path, len, flags),
       struct _reent *ptr _AND
       char *path _AND
       int len _AND
       int flags)
{
  int fd;

  return (_gettemp (ptr, path, &fd, 0, len, flags & ~O_ACCMODE) ? fd : -1);
}
#endif /* _ELIX_LEVEL */

char *
_DEFUN(_mktemp_r, (ptr, path),
       struct _reent *ptr _AND
       char *path)
{
  return (_gettemp (ptr, path, (int *) NULL, 0, 0, 0) ? path : (char *) NULL);
}

#ifndef _REENT_ONLY

int
_DEFUN(mkstemp, (path),
       char *path)
{
  int fd;

  return (_gettemp (_REENT, path, &fd, 0, 0, O_BINARY) ? fd : -1);
}

# if !defined _ELIX_LEVEL || _ELIX_LEVEL >= 4
char *
_DEFUN(mkdtemp, (path),
       char *path)
{
  return (_gettemp (_REENT, path, (int *) NULL, 1, 0, 0) ? path : NULL);
}

int
_DEFUN(mkstemps, (path, len),
       char *path _AND
       int len)
{
  int fd;

  return (_gettemp (_REENT, path, &fd, 0, len, O_BINARY) ? fd : -1);
}

int
_DEFUN(mkostemp, (path, flags),
       char *path _AND
       int flags)
{
  int fd;

  return (_gettemp (_REENT, path, &fd, 0, 0, flags & ~O_ACCMODE) ? fd : -1);
}

int
_DEFUN(mkostemps, (path, len, flags),
       char *path _AND
       int len _AND
       int flags)
{
  int fd;

  return (_gettemp (_REENT, path, &fd, 0, len, flags & ~O_ACCMODE) ? fd : -1);
}
# endif /* _ELIX_LEVEL */

char *
_DEFUN(mktemp, (path),
       char *path)
{
  return (_gettemp (_REENT, path, (int *) NULL, 0, 0, 0) ? path : (char *) NULL);
}

#endif /* ! defined (_REENT_ONLY) */
