/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<system>>---execute command string

INDEX
        system
INDEX
        _system_r

SYNOPSIS
        #include <stdlib.h>
        int system(char *<[s]>);

        int _system_r(void *<[reent]>, char *<[s]>);

DESCRIPTION

Use <<system>> to pass a command string <<*<[s]>>> to <</bin/sh>> on
your system, and wait for it to finish executing.

Use ``<<system(NULL)>>'' to test whether your system has <</bin/sh>>
available.

The alternate function <<_system_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<system(NULL)>> returns a non-zero value if <</bin/sh>> is available, and
<<0>> if it is not.

With a command argument, the result of <<system>> is the exit status
returned by <</bin/sh>>.

PORTABILITY
ANSI C requires <<system>>, but leaves the nature and effects of a
command processor undefined.  ANSI C does, however, specify that
<<system(NULL)>> return zero or nonzero to report on the existence of
a command processor.

POSIX.2 requires <<system>>, and requires that it invoke a <<sh>>.
Where <<sh>> is found is left unspecified.

Supporting OS subroutines required: <<_exit>>, <<_execve>>, <<_fork_r>>,
<<_wait_r>>.
*/

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

int
system(const char *s)
{
    char *argv[4];
    int   pid, status;

    argv[0] = "sh";
    argv[1] = "-c";
    argv[2] = (char *)s;
    argv[3] = NULL;

    if ((pid = fork()) == 0) {
        execve("/bin/sh", argv, environ);
        _exit(127);
    } else if (pid == -1)
        return -1;
    else {
        (void)waitpid(pid, &status, 0);
        return status;
    }
}
