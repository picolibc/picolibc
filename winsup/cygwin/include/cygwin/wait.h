/* cygwin/wait.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_WAIT_H
#define _CYGWIN_WAIT_H

#define WAIT_ANY	(pid_t)-1
#define WAIT_MYPGRP	(pid_t)0

#define WNOHANG 1
#define WUNTRACED 2
#define WCONTINUED 8
#define __W_CONTINUED	0xffff

/* A status is 16 bits, and looks like:
      <1 byte info> <1 byte code>

      <code> == 0, child has exited, info is the exit value
      <code> == 1..7e, child has exited, info is the signal number.
      <code> == 7f, child has stopped, info was the signal number.
      <code> == 80, there was a core dump.
*/

#define WIFEXITED(_w)		(((_w) & 0xff) == 0)
#define WIFSIGNALED(_w)		(((_w) & 0x7f) > 0 \
				 && (((_w) & 0x7f) < 0x7f))
#define WIFSTOPPED(_w)		(((_w) & 0xff) == 0x7f)
#define WIFCONTINUED(_w)	(((_w) & 0xffff) == __W_CONTINUED)
#define WEXITSTATUS(_w)		(((_w) >> 8) & 0xff)
#define WTERMSIG(_w)		((_w) & 0x7f)
#define WSTOPSIG		WEXITSTATUS
#define WCOREDUMP(_w)		(WIFSIGNALED(_w) && ((_w) & 0x80))

#endif /* _CYGWIN_WAIT_H */
