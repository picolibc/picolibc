/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Built-in symbol table for Hexagon libdl.
 *
 * libc.so has 26 undefined symbols that must be resolved at load time.
 * 15 are provided by libsemihost.a + libos-fallback.a (already linked
 * into the test executable).  The remaining 11 are stubbed out here.
 */

#define _DEFAULT_SOURCE
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

/* ------------------------------------------------------------------ */
/* 11 stubs for symbols not provided by semihost or os-fallback        */
/* ------------------------------------------------------------------ */

static clock_t
stub_times(struct tms *buf)
{
    if (buf) {
        buf->tms_utime  = 0;
        buf->tms_stime  = 0;
        buf->tms_cutime = 0;
        buf->tms_cstime = 0;
    }
    return 0;
}

static int
stub_clock_gettime(clockid_t id, struct timespec *ts)
{
    (void)id; (void)ts;
    errno = ENOSYS;
    return -1;
}

static int
stub_clock_getres(clockid_t id, struct timespec *ts)
{
    (void)id; (void)ts;
    errno = ENOSYS;
    return -1;
}

static int
stub_dup2(int oldfd, int newfd)
{
    (void)oldfd; (void)newfd;
    errno = ENOSYS;
    return -1;
}

static int
stub_execve(const char *path, char *const argv[], char *const envp[])
{
    (void)path; (void)argv; (void)envp;
    errno = ENOSYS;
    return -1;
}

static pid_t
stub_fork(void)
{
    errno = ENOSYS;
    return -1;
}

static uid_t
stub_getuid(void)
{
    return 0;
}

static void
stub_arc4random_abort(void)
{
    /* no-op: arc4random abort hook not needed in bare-metal context */
}

static void
stub_arc4random_fork_detect(void)
{
    /* no-op: fork detection not applicable on bare-metal */
}

static int
stub_getentropy(void *buf, size_t len)
{
    /* Return fixed bytes — sufficient for stack canary init on bare-metal.
     * The semihost getentropy loops on sys_semihost_time() which does not
     * advance on the simulator, causing an infinite loop.
     */
    unsigned char *p = buf;
    for (size_t i = 0; i < len; i++)
        p[i] = (unsigned char)(i ^ 0xa5);
    return 0;
}

static int
stub_nanosleep(const struct timespec *req, struct timespec *rem)
{
    (void)req; (void)rem;
    return 0;  /* no-op: success */
}

static int
stub_pipe(int pipefd[2])
{
    (void)pipefd;
    errno = ENOSYS;
    return -1;
}

static int
stub_tcgetattr(int fd, struct termios *termios_p)
{
    (void)fd; (void)termios_p;
    errno = ENOSYS;
    return -1;
}

static int
stub_tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    (void)fd; (void)optional_actions; (void)termios_p;
    errno = ENOSYS;
    return -1;
}

static pid_t
stub_waitpid(pid_t pid, int *wstatus, int options)
{
    (void)pid; (void)wstatus; (void)options;
    errno = ENOSYS;
    return -1;
}

/* ------------------------------------------------------------------ */
/* External refs to symbols provided by libsemihost + libos-fallback   */
/* ------------------------------------------------------------------ */

extern void    _exit(int status);
extern int     open(const char *path, int flags, ...);
extern int     close(int fd);
extern ssize_t read(int fd, void *buf, size_t count);
extern ssize_t write(int fd, const void *buf, size_t count);
extern off_t   lseek(int fd, off_t offset, int whence);
extern int     fstat(int fd, struct stat *statbuf);
extern int     stat(const char *path, struct stat *statbuf);
extern int     unlink(const char *path);
extern int     gettimeofday(struct timeval *tv, void *tz);
extern int     sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
extern int     raise(int sig);
extern void   *sbrk(ptrdiff_t incr);

/* ------------------------------------------------------------------ */
/* Symbol table                                                         */
/* ------------------------------------------------------------------ */

void *dl_builtin_lookup(const char *name);

static const struct {
    const char *name;
    void       *addr;
} dl_builtin_syms[] = {
    /* provided by libsemihost.a */
    { "_exit",          (void *)_exit         },
    { "open",           (void *)open          },
    { "close",          (void *)close         },
    { "read",           (void *)read          },
    { "write",          (void *)write         },
    { "lseek",          (void *)lseek         },
    { "fstat",          (void *)fstat         },
    { "stat",           (void *)stat          },
    { "unlink",         (void *)unlink        },
    { "times",          (void *)stub_times    },
    { "gettimeofday",   (void *)gettimeofday  },
    { "getentropy",     (void *)stub_getentropy  },
    { "sigprocmask",    (void *)sigprocmask   },
    /* provided by libos-fallback.a */
    { "raise",          (void *)raise         },
    { "sbrk",           (void *)sbrk          },
    /* stubs defined above */
    { "clock_gettime",  (void *)stub_clock_gettime },
    { "clock_getres",   (void *)stub_clock_getres  },
    { "dup2",           (void *)stub_dup2          },
    { "execve",         (void *)stub_execve        },
    { "fork",           (void *)stub_fork          },
    { "getuid",         (void *)stub_getuid        },
    { "arc4random_abort",       (void *)stub_arc4random_abort       },
    { "arc4random_fork_detect", (void *)stub_arc4random_fork_detect },
    { "nanosleep",      (void *)stub_nanosleep     },
    { "pipe",           (void *)stub_pipe          },
    { "tcgetattr",      (void *)stub_tcgetattr     },
    { "tcsetattr",      (void *)stub_tcsetattr     },
    { "waitpid",        (void *)stub_waitpid       },
    { NULL, NULL }
};

void *
dl_builtin_lookup(const char *name)
{
    for (int i = 0; dl_builtin_syms[i].name != NULL; i++) {
        if (strcmp(dl_builtin_syms[i].name, name) == 0)
            return dl_builtin_syms[i].addr;
    }
    return NULL;
}
