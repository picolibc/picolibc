/*
FUNCTION
<<remove>>---delete a file's name

INDEX
	remove

ANSI_SYNOPSIS
	#include <stdio.h>
	int remove(char *<[filename]>);

	int _remove_r(void *<[reent]>, char *<[filename]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int remove(<[filename]>)
	char *<[filename]>;

	int _remove_r(<[reent]>, <[filename]>)
	char *<[reent]>;
	char *<[filename]>;

DESCRIPTION
Use <<remove>> to dissolve the association between a particular
filename (the string at <[filename]>) and the file it represents.
After calling <<remove>> with a particular filename, you will no
longer be able to open the file by that name.

In this implementation, you may use <<remove>> on an open file without
error; existing file descriptors for the file will continue to access
the file's data until the program using them closes the file.

The alternate function <<_remove_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<remove>> returns <<0>> if it succeeds, <<-1>> if it fails.

PORTABILITY
ANSI C requires <<remove>>, but only specifies that the result on
failure be nonzero.  The behavior of <<remove>> when you call it on an
open file may vary among implementations.

Supporting OS subroutine required: <<unlink>>.
*/

#include <stdio.h>
#include <reent.h>

int
_remove_r (ptr, filename)
     struct _reent *ptr;
     _CONST char *filename;
{
  if (_unlink_r (ptr, filename) == -1)
    return -1;

  return 0;
}

#ifndef _REENT_ONLY

int
remove (filename)
     _CONST char *filename;
{
  return _remove_r (_REENT, filename);
}

#endif
