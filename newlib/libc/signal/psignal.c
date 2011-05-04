/* Copyright 2002, 2011 Red Hat Inc. */
/*
FUNCTION
<<psignal>>---print a signal message on standard error

INDEX
	psignal

ANSI_SYNOPSIS
	#include <stdio.h>
	void psignal(int <[signal]>, const char *<[prefix]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void psignal(<[signal]>, <[prefix]>)
	int <[signal]>;
	const char *<[prefix]>;

DESCRIPTION
Use <<psignal>> to print (on standard error) a signal message
corresponding to the value of the signal number <[signal]>.
Unless you use <<NULL>> as the value of the argument <[prefix]>, the
signal message will begin with the string at <[prefix]>, followed by a
colon and a space (<<: >>). The remainder of the signal message is one
of the strings described for <<strsignal>>.

RETURNS
<<psignal>> returns no result.

PORTABILITY
POSIX.1-2008 requires <<psignal>>, but the strings issued vary from one
implementation to another.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#include <_ansi.h>
#include <stdio.h>
#include <string.h>

_VOID
_DEFUN(psignal, (sig, s),
       int sig _AND
       _CONST char *s)
{
  if (s != NULL && *s != '\0')
    fprintf (stderr, "%s: %s\n", s, strsignal (sig));
  else
    fprintf (stderr, "%s\n", strsignal (sig));
}
