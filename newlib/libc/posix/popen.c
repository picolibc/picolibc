/*	$NetBSD: popen.c,v 1.11 1995/06/16 07:05:33 jtc Exp $	*/

/*
 * Copyright (c) 1988, 1993, 2006
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software written by Ken Arnold and
 * published in UNIX Review, Vol. 6, No. 8.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<popen>>, <<pclose>>---tie a stream to a command string

INDEX
	popen
INDEX
	pclose

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *popen(const char *<[s]>, const char *<[mode]>);

	int pclose(FILE *<[f]>);

DESCRIPTION
Use <<popen>> to create a stream to a child process executing a
command string <<*<[s]>>> as processed by <</bin/sh>> on your system.
The argument <[mode]> must start with either `<<r>>', where the stream
reads from the child's <<stdout>>, or `<<w>>', where the stream writes
to the child's <<stdin>>.  As an extension, <[mode]> may also contain
`<<e>>' to set the close-on-exec bit of the parent's file descriptor.
The stream created by <<popen>> must be closed by <<pclose>> to avoid
resource leaks.

Streams created by prior calls to <<popen>> are not visible in
subsequent <<popen>> children, regardless of the close-on-exec bit.

Use ``<<system(NULL)>>'' to test whether your system has <</bin/sh>>
available.

RETURNS
<<popen>> returns a file stream opened with the specified <[mode]>,
or <<NULL>> if a child process could not be created.  <<pclose>>
returns -1 if the stream was not created by <<popen>> or if the
application used <<wait>> or similar to steal the status; otherwise
it returns the exit status of the child which can be interpreted
in the same manner as a status obtained by <<waitpid>>.

PORTABILITY
POSIX.2 requires <<popen>> and <<pclose>>, but only specifies a mode
of just <<r>> or <<w>>.  Where <<sh>> is found is left unspecified.

Supporting OS subroutines required: <<_exit>>, <<_execve>>, <<_fork_r>>,
<<_wait_r>>, <<pipe>>, <<fcntl>>, <<sbrk>>.
*/

#ifndef _NO_POPEN

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)popen.c	8.1 (Berkeley) 6/4/93";
#else
static char rcsid[] = "$NetBSD: popen.c,v 1.11 1995/06/16 07:05:33 jtc Exp $";
#endif
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <fcntl.h>

static struct pid {
	struct pid *next;
	FILE *fp;
	pid_t pid;
} *pidlist;

FILE *
_DEFUN(popen, (program, type),
	const char *program _AND
	const char *type)
{
	struct pid *cur;
	FILE *iop;
	int pdes[2], pid;

       if ((*type != 'r' && *type != 'w')
	   || (type[1]
#ifdef HAVE_FCNTL
	       && (type[2] || (type[1] != 'e'))
#endif
			       )) {
		errno = EINVAL;
		return (NULL);
	}

	if ((cur = malloc(sizeof(struct pid))) == NULL)
		return (NULL);

	if (pipe(pdes) < 0) {
		free(cur);
		return (NULL);
	}

	switch (pid = vfork()) {
	case -1:			/* Error. */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		free(cur);
		return (NULL);
		/* NOTREACHED */
	case 0:				/* Child. */
		if (*type == 'r') {
			if (pdes[1] != STDOUT_FILENO) {
				(void)dup2(pdes[1], STDOUT_FILENO);
				(void)close(pdes[1]);
			}
			if (pdes[0] != STDOUT_FILENO) {
				(void) close(pdes[0]);
			}
		} else {
			if (pdes[0] != STDIN_FILENO) {
				(void)dup2(pdes[0], STDIN_FILENO);
				(void)close(pdes[0]);
			}
			(void)close(pdes[1]);
		}
		/* Close all fd's created by prior popen.  */
		for (cur = pidlist; cur; cur = cur->next)
			(void)close (fileno (cur->fp));
		execl(_PATH_BSHELL, "sh", "-c", program, NULL);
		_exit(127);
		/* NOTREACHED */
	}

	/* Parent; assume fdopen can't fail. */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}

#ifdef HAVE_FCNTL
	/* Mark pipe cloexec if requested.  */
	if (type[1] == 'e')
		fcntl (fileno (iop), F_SETFD,
		       fcntl (fileno (iop), F_GETFD, 0) | FD_CLOEXEC);
#endif /* HAVE_FCNTL */

	/* Link into list of file descriptors. */
	cur->fp = iop;
	cur->pid =  pid;
	cur->next = pidlist;
	pidlist = cur;

	return (iop);
}

/*
 * pclose --
 *	Pclose returns -1 if stream is not associated with a `popened' command,
 *	if already `pclosed', or waitpid returns an error.
 */
int
_DEFUN(pclose, (iop),
	FILE *iop)
{
	register struct pid *cur, *last;
	int pstat;
	pid_t pid;

	(void)fclose(iop);

	/* Find the appropriate file pointer. */
	for (last = NULL, cur = pidlist; cur; last = cur, cur = cur->next)
		if (cur->fp == iop)
			break;
	if (cur == NULL)
		return (-1);

	do {
		pid = waitpid(cur->pid, &pstat, 0);
	} while (pid == -1 && errno == EINTR);

	/* Remove the entry from the linked list. */
	if (last == NULL)
		pidlist = cur->next;
	else
		last->next = cur->next;
	free(cur);

	return (pid == -1 ? -1 : pstat);
}

#endif  /* !_NO_POPEN  */
