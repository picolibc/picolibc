/*
FUNCTION
        <<strsignal>>---convert signal number to string

INDEX
        strsignal

SYNOPSIS
        #include <string.h>
        char *strsignal(int <[signal]>);

DESCRIPTION
<<strsignal>> converts the signal number <[signal]> into a
string.  If <[signal]> is not a known signal number, the result
will be of the form "Unknown signal NN" where NN is the <[signal]>
is a decimal number.

RETURNS
This function returns a pointer to a string.  Your application must
not modify that string.

PORTABILITY
POSIX.1-2008 C requires <<strsignal>>, but does not specify the strings used
for each signal number.

<<strsignal>> requires no supporting OS subroutines.

QUICKREF
        strsignal pure
*/

/*
 *  Written by Joel Sherrill <joel.sherrill@OARcorp.com>.
 *
 *  COPYRIGHT (c) 2010, 2017.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Permission to use, copy, modify, and distribute this software for any
 *  purpose without fee is hereby granted, provided that this entire notice
 *  is included in all copies of any software which is or includes a copy
 *  or modification of this software.
 *
 *  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 *  WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
 *  OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
 *  SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#define _DEFAULT_SOURCE
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static const char * const signames[] = {
#ifdef SIGHUP
    [SIGHUP] = "Hangup",
#endif
#ifdef SIGINT
    [SIGINT] = "Interrupt",
#endif
#ifdef SIGQUIT
    [SIGQUIT] = "Quit",
#endif
#ifdef SIGILL
    [SIGILL] = "Illegal instruction",
#endif
#ifdef SIGTRAP
    [SIGTRAP] = "Trace/breakpoint trap",
#endif
#if defined(SIGABRT)
    [SIGABRT] = "Abort",
#endif
#if defined(SIGIOT) && SIGIOT != SIGABRT
    [SIGIOT] = "IOT trap",
#endif
#ifdef SIGEMT
    [SIGEMT] = "EMT trap",
#endif
#ifdef SIGFPE
    [SIGFPE] = "Floating point exception",
#endif
#ifdef SIGKILL
    [SIGKILL] = "Killed",
#endif
#ifdef SIGBUS
    [SIGBUS] = "Bus error",
#endif
#ifdef SIGSEGV
    [SIGSEGV] = "Segmentation fault",
#endif
#ifdef SIGSYS
    [SIGSYS] = "Bad system call",
#endif
#ifdef SIGPIPE
    [SIGPIPE] = "Broken pipe",
#endif
#ifdef SIGALRM
    [SIGALRM] = "Alarm clock",
#endif
#ifdef SIGTERM
    [SIGTERM] = "Terminated",
#endif
#ifdef SIGURG
    [SIGURG] = "Urgent I/O condition",
#endif
#ifdef SIGSTOP
    [SIGSTOP] = "Stopped (signal)",
#endif
#ifdef SIGTSTP
    [SIGTSTP] = "Stopped",
#endif
#ifdef SIGCONT
    [SIGCONT] = "Continued",
#endif
#ifdef SIGCHLD
#if defined(SIGCLD) && (SIGCHLD != SIGCLD)
    [SIGCLD] = "Child exited",
#endif
    [SIGCHLD] = "Child exited",
#endif
#ifdef SIGTTIN
    [SIGTTIN] = "Stopped (tty input)",
#endif
#ifdef SIGTTOUT
    [SIGTTOUT] = "Stopped (tty output)",
#endif
#ifdef SIGIO
#if defined(SIGPOLL) && (SIGIO != SIGPOLL)
    [SIGPOLL] = "I/O possible",
#endif
    [SIGIO] = "I/O possible",
#endif
#ifdef SIGWINCH
    [SIGWINCH] = "Window changed",
#endif
#ifdef SIGUSR1
    [SIGUSR1] = "User defined signal 1",
#endif
#ifdef SIGUSR2
    [SIGUSR2] = "User defined signal 2",
#endif
#ifdef SIGPWR
    [SIGPWR] = "Power Failure",
#endif
#ifdef SIGXCPU
    [SIGXCPU] = "CPU time limit exceeded",
#endif
#ifdef SIGXFSZ
    [SIGXFSZ] = "File size limit exceeded",
#endif
#ifdef SIGVTALRM
    [SIGVTALRM] = "Virtual timer expired",
#endif
#ifdef SIGPROF
    [SIGPROF] = "Profiling timer expired",
#endif
#if defined(SIGLOST) && SIGLOST != SIGPWR
    [SIGLOST] = "Resource lost",
#endif
};

#define NSIGNAMES (sizeof(signames) / sizeof(signames[0]))

static __THREAD_LOCAL char _signal_buf[24];

static inline char *
_intsig(const char *head, int i)
{
    char *buf = _signal_buf;
    buf = stpcpy(buf, head);
    if (i >= 100) {
        *buf++ = '?';
    } else {
        if (i >= 10) {
            *buf++ = '0' + i / 10;
            i %= 10;
        }
        *buf++ = '0' + i;
    }
    *buf++ = '\0';
    return _signal_buf;
}

char *
strsignal(int signal)
{
#if defined(SIGRTMIN) && defined(SIGRTMAX)
    if ((signal >= SIGRTMIN) && (signal <= SIGRTMAX)) {
        return _intsig("Real-time signal", signal - SIGRTMIN);
    }
#endif

    if ((unsigned)signal < NSIGNAMES)
        return (char *)signames[(unsigned)signal];

    return _intsig("Unknown signal", signal);
}
