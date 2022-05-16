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
	char *_tmpnam_r(struct _reent *<[reent]>, char *<[s]>);
	char *_tempnam_r(struct _reent *<[reent]>, char *<[dir]>, char *<[pfx]>);

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

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <reent.h>
#include <errno.h>

#ifdef _REENT_THREAD_LOCAL
_Thread_local int _tls_inc;
_Thread_local char _tls_emergency;
#endif

/* Try to open the file specified, if it can't be opened then try
   another one.  Return nonzero if successful, otherwise zero.  */

static int
worker (struct _reent *ptr,
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
      _sprintf_r (ptr, result, "%s/%s%x.%x", part1, part2, part3, *part4);
      (*part4)++;
      t = _open_r (ptr, result, O_RDONLY, 0);
      if (t == -1)
	{
	  if (_REENT_ERRNO(ptr) == ENOSYS)
	    {
	      result[0] = '\0';
	      return 0;
	    }
	  break;
	}
      _close_r (ptr, t);
    }
  return 1;
}

char *
_tmpnam_r (struct _reent *p,
       char *s)
{
  char *result;
  int pid;

  if (s == NULL)
    {
      /* ANSI states we must use an internal static buffer if s is NULL */
      _REENT_CHECK_EMERGENCY(p);
      result = _REENT_EMERGENCY(p);
    }
  else
    {
      result = s;
    }
  pid = _getpid_r (p);

  if (worker (p, result, P_tmpdir, "t", pid, &_REENT_INC(p)))
    {
      _REENT_INC(p)++;
      return result;
    }

  return NULL;
}

char *
_tempnam_r (struct _reent *p,
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

  filename = _malloc_r (p, length);
  if (filename)
    {
      if (! worker (p, filename, dir, prefix,
		    _getpid_r (p) ^ (int) (_POINTER_INT) p, &_REENT_INC(p)))
	return NULL;
    }
  return filename;
}

#ifndef _REENT_ONLY

char *
tempnam (const char *dir,
       const char *pfx)
{
  return _tempnam_r (_REENT, dir, pfx);
}

char *
tmpnam (char *s)
{
  return _tmpnam_r (_REENT, s);
}

#endif
