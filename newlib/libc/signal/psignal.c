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
#include <sys/uio.h>

#define ADD(str) \
{ \
  v->iov_base = (void *)(str); \
  v->iov_len = strlen (v->iov_base); \
  v ++; \
  iov_cnt ++; \
}

void
psignal (int sig,
       const char *s)
{
  struct iovec iov[4];
  struct iovec *v = iov;
  int iov_cnt = 0;

  if (s != NULL && *s != '\0')
    {
      ADD (s);
      ADD (": ");
    }
  ADD (strsignal (sig));

#ifdef __SCLE
  ADD ((stderr->_flags & __SCLE) ? "\r\n" : "\n");
#else
  ADD ("\n");
#endif

  fflush (stderr);
  writev (fileno (stderr), iov, iov_cnt);
}
