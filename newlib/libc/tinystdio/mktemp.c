/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution.
 * Neither the name of the University nor the names of its
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
<<mktemp>>, <<mkstemp>>, <<mkstemps>>,
<<mkostemps>>---generate unused file name

INDEX
	mktemp
INDEX
	mkstemp
INDEX
	mkstemps
INDEX
	mkostemps

SYNOPSIS
	#include <stdlib.h>
	char *mktemp(char *<[path]>);
	int mkstemp(char *<[path]>);
        int mkstemps(char *<[path]>, int suffixlen);
        int mkostemps(char *<[path]>, int suffixlen, int flags);

DESCRIPTION
<<mktemp>>, <<mkstemp>>, <<mkstemps>>, and <<mkostemps>> attempt to
generate a file name that is not yet in use for any existing file.
<<mkstemp>>, <<mkstemps>> and <mkostemps>> create the file and open it
for reading and writing; <<mktemp>> simply generates the file name
(making <<mktemp>> a security risk). <<mkostemps>> allow the addition
of other <<open>> flags, such as <<O_CLOEXEC>>, <<O_APPEND>>, or
<<O_SYNC>>.  On platforms with a separate text mode, <<mkstemp>>
forces <<O_BINARY>>, while <<mkostemp>> allows the choice between
<<O_BINARY>>, <<O_TEXT>>, or 0 for default.

You supply a simple pattern for the generated file name, as the string
at <[path]>.  The pattern should be a valid filename (including path
information if you wish) ending with at least six `<<X>>' characters.
The generated filename will match the leading part of the name you
supply, with the trailing `<<X>>' characters replaced by some
combination of digits and letters.  With <<mkstemps>> and
<mkostemps>>, the `<<X>>' characters end <[suffixlen]> bytes before
the end of the string.

RETURNS
<<mktemp>> returns the pointer <[path]> to the modified string
representing an unused filename, unless it could not generate one, or
the pattern you provided is not suitable for a filename; in that case,
it returns <<NULL>>.  Be aware that there is an inherent race between
generating the name and attempting to create a file by that name;
you are advised to use <<O_EXCL|O_CREAT>>.

<<mkstemp>>, <<mkstemps>> and <<mkostemps>> return a file descriptor
to the newly created file, unless it could not generate an unused
filename, or the pattern you provided is not suitable for a filename;
in that case, it returns <<-1>>.

NOTES
Never use <<mktemp>>.  The generated filenames are easy to guess and
there's a race between the test if the file exists and the creation
of the file.  In combination this makes <<mktemp>> prone to attacks
and using it is a security risk.  Whenever possible use <<mkstemp>>
instead.  It doesn't suffer the race condition.

PORTABILITY
ANSI C does not require either <<mktemp>> or <<mkstemp>>; the System V
Interface Definition requires <<mktemp>> as of Issue 2.  POSIX 2001
requires <<mkstemp>> while deprecating <<mktemp>>.  <<mkstemps>>, and
<<mkostemps>> are not standardized.

Supporting OS subroutines required: <<open>>
*/

#include "stdio_private.h"

static int
_gettemp (char *path,
          int suffixlen,
          int *doopen,
          int flags)
{
  char *start, *trv;
  char *end;

  end = path + strlen(path) - suffixlen;
  trv = end;

  /* Replace 'X' with 'a' */
  while (path < trv && *--trv == 'X')
    *trv = 'a';

  /* Make sure we got six Xs */
  if (end - trv < 6)
    {
      errno = EINVAL;
      return 0;
    }

  start = trv + 1;

  for (;;)
    {
      /*
       * Use open to check if the file exists to avoid depending on
       * stat or access. Don't rely on O_EXCL working, although if it
       * doesn't, this introduces a race condition
       */
      int fd = open(path, O_RDONLY);
      if (fd < 0) {
        if (errno != EACCES) {
          if (errno != ENOENT)
            return 0;
          if (doopen)
          {
            fd = open (path, flags | O_CREAT | O_EXCL | O_RDWR,
                       0600);
            if (fd >= 0) {
              *doopen = fd;
              return 1;
            }
            if (errno != EEXIST)
              return 0;
          } else {
            return 1;
          }
        }
      } else
        close(fd);

      /* Increment the string of letters to generate another name */
      trv = start;
      for(;;)
	{
	  if (trv == end)
	    return 0;
	  if (*trv == 'z')
	    *trv++ = 'a';
	  else {
            ++ * trv;
            break;
          }
	}
    }
  /*NOTREACHED*/
}

int
mkstemp (char *template)
{
  int fd;

  return (_gettemp (template, 0, &fd, 0) ? fd : -1);
}

char *
mktemp (char *template)
{
  return (_gettemp (template, 0, (int *) NULL, 0) ? template : (char *) NULL);
}

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif

int
mkstemps(char *template, int suffixlen)
{
  int fd;

  return (_gettemp (template, suffixlen, &fd, O_BINARY) ? fd : -1);
}

int
mkostemps(char *template, int suffixlen, int flags)
{
  int fd;

  flags &= (O_APPEND | O_CLOEXEC | O_SYNC | O_BINARY | O_TEXT);
  return (_gettemp (template, suffixlen, &fd, flags) ? fd : -1);
}
