/* Copyright 2002, 2011 Red Hat Inc. */
/*
FUNCTION
<<psignal>>---print a signal message on standard error

INDEX
	psignal

SYNOPSIS
	#include <stdio.h>
	void psignal(int <[signal]>, const char *<[prefix]>);

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
#include <unistd.h>

#define WRITE_STR(str) \
{ \
  const char *p = (str); \
  size_t len = strlen (p); \
  while (len) \
    { \
      ssize_t len1 = write (fileno (stderr), p, len); \
      if (len1 < 0) \
	break; \
      len -= len1; \
      p += len1; \
    } \
}

void
psignal (int sig,
       const char *s)
{
  fflush (stderr);
  if (s != NULL && *s != '\0')
    {
      WRITE_STR (s);
      WRITE_STR (": ");
    }
  WRITE_STR (strsignal (sig));

#ifdef __SCLE
  WRITE_STR ((stderr->_flags & __SCLE) ? "\r\n" : "\n");
#else
  WRITE_STR ("\n");
#endif
}
