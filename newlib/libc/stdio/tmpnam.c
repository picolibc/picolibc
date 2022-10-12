/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * tmpname.c
 * Original Author:	G. Haley
 */
/*
FUNCTION
<<tmpnam>>, <<tempnam>>---name for a temporary file

INDEX
	tmpnam
INDEX
	tempnam
INDEX
	_tmpnam_r
INDEX
	_tempnam_r

SYNOPSIS
	#include <stdio.h>
	char *tmpnam(char *<[s]>);
	char *tempnam(char *<[dir]>, char *<[pfx]>);
	char *tmpnam( char *<[s]>);
	char *tempnam( char *<[dir]>, char *<[pfx]>);

DESCRIPTION
Use either of these functions to generate a name for a temporary file.
The generated name is guaranteed to avoid collision with other files
(for up to <<TMP_MAX>> calls of either function).

<<tmpnam>> generates file names with the value of <<P_tmpdir>>
(defined in `<<stdio.h>>') as the leading directory component of the path.

You can use the <<tmpnam>> argument <[s]> to specify a suitable area
of memory for the generated filename; otherwise, you can call
<<tmpnam(NULL)>> to use an internal static buffer.

<<tempnam>> allows you more control over the generated filename: you
can use the argument <[dir]> to specify the path to a directory for
temporary files, and you can use the argument <[pfx]> to specify a
prefix for the base filename.

If <[dir]> is <<NULL>>, <<tempnam>> will attempt to use the value of
environment variable <<TMPDIR>> instead; if there is no such value,
<<tempnam>> uses the value of <<P_tmpdir>> (defined in `<<stdio.h>>').

If you don't need any particular prefix to the basename of temporary
files, you can pass <<NULL>> as the <[pfx]> argument to <<tempnam>>.

<<_tmpnam_r>> and <<_tempnam_r>> are reentrant versions of <<tmpnam>>
and <<tempnam>> respectively.  The extra argument <[reent]> is a
pointer to a reentrancy structure.

WARNINGS
The generated filenames are suitable for temporary files, but do not
in themselves make files temporary.  Files with these names must still
be explicitly removed when you no longer want them.

If you supply your own data area <[s]> for <<tmpnam>>, you must ensure
that it has room for at least <<L_tmpnam>> elements of type <<char>>.

RETURNS
Both <<tmpnam>> and <<tempnam>> return a pointer to the newly
generated filename.

PORTABILITY
ANSI C requires <<tmpnam>>, but does not specify the use of
<<P_tmpdir>>.  The System V Interface Definition (Issue 2) requires
both <<tmpnam>> and <<tempnam>>.

Supporting OS subroutines required: <<close>>,  <<fstat>>, <<getpid>>,
<<isatty>>, <<lseek>>, <<open>>, <<read>>, <<sbrk>>, <<write>>.

The global pointer <<environ>> is also required.
*/

#define _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

static NEWLIB_THREAD_LOCAL int _tls_inc;

/* Try to open the file specified, if it can't be opened then try
   another one.  Return nonzero if successful, otherwise zero.  */

static int
worker (
       char *result,
       const char *part1,
       const char *part2,
       int part3,
       int *part4)
{
  /*  Generate the filename and make sure that there isn't one called
      it already.  */

  while (1)
    {
      int t;
      sprintf ( result, "%s/%s%x.%x", part1, part2, part3, *part4);
      (*part4)++;
      t = open (result, O_RDONLY, 0);
      if (t == -1)
	{
	  if (_REENT_ERRNO(ptr) == ENOSYS)
	    {
	      result[0] = '\0';
	      return 0;
	    }
	  break;
	}
      close (t);
    }
  return 1;
}

#define _TMPNAM_SIZE 25

static NEWLIB_THREAD_LOCAL char _tmpnam_buf[_TMPNAM_SIZE];

char *
tmpnam (
       char *s)
{
  char *result;
  int pid;

  if (s == NULL)
    {
      /* ANSI states we must use an internal static buffer if s is NULL */
      result = _tmpnam_buf;
    }
  else
    {
      result = s;
    }
  pid = getpid ();

  if (worker (result, P_tmpdir, "t", pid, &_tls_inc))
    {
      _tls_inc++;
      return result;
    }

  return NULL;
}

char *
tempnam (
       const char *dir,
       const char *pfx)
{
  char *filename;
  int length;
  const char *prefix = (pfx) ? pfx : "";
  if (dir == NULL && (dir = getenv ("TMPDIR")) == NULL)
    dir = P_tmpdir;

  /* two 8 digit numbers + . / */
  length = strlen (dir) + strlen (prefix) + (4 * sizeof (int)) + 2 + 1;

  filename = malloc (length);
  if (filename)
    {
      if (! worker (filename, dir, prefix,
		    getpid (), &_tls_inc))
	return NULL;
    }
  return filename;
}
