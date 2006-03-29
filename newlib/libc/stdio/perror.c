/*
FUNCTION
<<perror>>---print an error message on standard error

INDEX
	perror
INDEX
	_perror_r

ANSI_SYNOPSIS
	#include <stdio.h>
	void perror(char *<[prefix]>);

	void _perror_r(void *<[reent]>, char *<[prefix]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void perror(<[prefix]>)
	char *<[prefix]>;

	void _perror_r(<[reent]>, <[prefix]>)
	char *<[reent]>;
	char *<[prefix]>;

DESCRIPTION
Use <<perror>> to print (on standard error) an error message
corresponding to the current value of the global variable <<errno>>.
Unless you use <<NULL>> as the value of the argument <[prefix]>, the
error message will begin with the string at <[prefix]>, followed by a
colon and a space (<<: >>). The remainder of the error message is one
of the strings described for <<strerror>>.

The alternate function <<_perror_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.


RETURNS
<<perror>> returns no result.

PORTABILITY
ANSI C requires <<perror>>, but the strings issued vary from one
implementation to another.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

void
_DEFUN (_perror_r, (ptr, s),
	struct _reent *ptr _AND
	_CONST char *s)
{
  char *error;

  if (s != NULL && *s != '\0')
    {
      fputs (s, _stderr_r (ptr));
      fputs (": ", _stderr_r (ptr));
    }

  if ((error = strerror (ptr->_errno)) != NULL)
    fputs (error, _stderr_r (ptr));

  fputc ('\n', _stderr_r (ptr));
}

#ifndef _REENT_ONLY

void
_DEFUN (perror, (s),
	_CONST char *s)
{
  _perror_r (_REENT, s);
}

#endif
