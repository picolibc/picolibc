#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <_ansi.h>
#include <sys/types.h>
#define __need_size_t
#include <stddef.h>

extern char **environ;

void	_EXFUN(_exit, (int __status ) _ATTRIBUTE ((noreturn)));

int	_EXFUN(access,(const char *__path, int __amode ));
unsigned  _EXFUN(alarm, (unsigned __secs ));
int     _EXFUN(chdir, (const char *__path ));
int     _EXFUN(chmod, (const char *__path, mode_t __mode ));
int     _EXFUN(chown, (const char *__path, uid_t __owner, gid_t __group ));
int     _EXFUN(close, (int __fildes ));
char    _EXFUN(*ctermid, (char *__s ));
char    _EXFUN(*cuserid, (char *__s ));
int     _EXFUN(dup, (int __fildes ));
int     _EXFUN(dup2, (int __fildes, int __fildes2 ));
int     _EXFUN(execl, (const char *__path, const char *, ... ));
int     _EXFUN(execle, (const char *__path, const char *, ... ));
int     _EXFUN(execlp, (const char *__file, const char *, ... ));
int     _EXFUN(execv, (const char *__path, char * const __argv[] ));
int     _EXFUN(execve, (const char *__path, char * const __argv[], char * const __envp[] ));
int     _EXFUN(execvp, (const char *__file, char * const __argv[] ));
int     _EXFUN(fchmod, (int __fildes, mode_t __mode ));
int     _EXFUN(fchown, (int __fildes, uid_t __owner, gid_t __group ));
pid_t   _EXFUN(fork, (void ));
long    _EXFUN(fpathconf, (int __fd, int __name ));
int     _EXFUN(fsync, (int __fd));
char    _EXFUN(*getcwd, (char *__buf, size_t __size ));
gid_t   _EXFUN(getegid, (void ));
uid_t   _EXFUN(geteuid, (void ));
gid_t   _EXFUN(getgid, (void ));
int     _EXFUN(getgroups, (int __gidsetsize, gid_t __grouplist[] ));
char    _EXFUN(*getlogin, (void ));
char 	_EXFUN(*getpass, (__const char *__prompt));
size_t  _EXFUN(getpagesize, (void));
pid_t   _EXFUN(getpgrp, (void ));
pid_t   _EXFUN(getpid, (void ));
pid_t   _EXFUN(getppid, (void ));
uid_t   _EXFUN(getuid, (void ));
int     _EXFUN(isatty, (int __fildes ));
int     _EXFUN(lchown, (const char *__path, uid_t __owner, gid_t __group ));
int     _EXFUN(link, (const char *__path1, const char *__path2 ));
int	_EXFUN(nice, (int __nice_value ));
off_t   _EXFUN(lseek, (int __fildes, off_t __offset, int __whence ));
long    _EXFUN(pathconf, (const char *__path, int __name ));
int     _EXFUN(pause, (void ));
int     _EXFUN(pipe, (int __fildes[2] ));
int     _EXFUN(read, (int __fildes, void *__buf, size_t __nbyte ));
int     _EXFUN(rmdir, (const char *__path ));
void *  _EXFUN(sbrk,  (size_t __incr));
#if defined(__CYGWIN__)
int     _EXFUN(setegid, (gid_t __gid ));
int     _EXFUN(seteuid, (uid_t __uid ));
#endif
int     _EXFUN(setgid, (gid_t __gid ));
int     _EXFUN(setpgid, (pid_t __pid, pid_t __pgid ));
pid_t   _EXFUN(setsid, (void ));
int     _EXFUN(setuid, (uid_t __uid ));
unsigned _EXFUN(sleep, (unsigned int __seconds ));
void    _EXFUN(swab, (const void *, void *, ssize_t));
long    _EXFUN(sysconf, (int __name ));
pid_t   _EXFUN(tcgetpgrp, (int __fildes ));
int     _EXFUN(tcsetpgrp, (int __fildes, pid_t __pgrp_id ));
char    _EXFUN(*ttyname, (int __fildes ));
int     _EXFUN(unlink, (const char *__path ));
int     _EXFUN(write, (int __fildes, const void *__buf, size_t __nbyte ));

#ifndef        _POSIX_SOURCE
pid_t   _EXFUN(vfork, (void ));
#endif /* _POSIX_SOURCE */

/* Provide prototypes for most of the _<systemcall> names that are
   provided in newlib for some compilers.  */
int     _EXFUN(_close, (int __fildes ));
pid_t   _EXFUN(_fork, (void ));
pid_t   _EXFUN(_getpid, (void ));
int     _EXFUN(_link, (const char *__path1, const char *__path2 ));
off_t   _EXFUN(_lseek, (int __fildes, off_t __offset, int __whence ));
int     _EXFUN(_read, (int __fildes, void *__buf, size_t __nbyte ));
void *  _EXFUN(_sbrk,  (size_t __incr));
int     _EXFUN(_unlink, (const char *__path ));
int     _EXFUN(_write, (int __fildes, const void *__buf, size_t __nbyte ));
int     _EXFUN(_execve, (const char *__path, char * const __argv[], char * const __envp[] ));

#if defined(__CYGWIN__) || defined(__rtems__)
unsigned _EXFUN(usleep, (unsigned int __useconds));
int     _EXFUN(ftruncate, (int __fd, off_t __length));
int     _EXFUN(truncate, (const char *, off_t __length));
int	_EXFUN(gethostname, (char *__name, size_t __len));
char *	_EXFUN(mktemp, (char *));
int     _EXFUN(sync, (void));
int     _EXFUN(readlink, (const char *__path, char *__buf, int __buflen));
int     _EXFUN(symlink, (const char *__name1, const char *__name2));
#endif

# define	F_OK	0
# define	R_OK	4
# define	W_OK	2
# define	X_OK	1

# define	SEEK_SET	0
# define	SEEK_CUR	1
# define	SEEK_END	2

/*
 *  RTEMS adheres to a later version of POSIX -- 1003.1b.
 *
 *  XXX this version string should change.
 */

#ifdef __rtems__
#ifndef _POSIX_JOB_CONTROL
# define _POSIX_JOB_CONTROL     1
#endif
#ifndef _POSIX_SAVED_IDS
# define _POSIX_SAVED_IDS       1
#endif
# define _POSIX_VERSION 199009L
#else
#ifdef __svr4__
# define _POSIX_JOB_CONTROL     1
# define _POSIX_SAVED_IDS       1
# define _POSIX_VERSION 199009L
#endif
#endif

#ifdef __CYGWIN__
# define _POSIX_JOB_CONTROL	1
# define _POSIX_SAVED_IDS	0
# define _POSIX_VERSION		199009L
#endif

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */

long _EXFUN(sysconf, (int __name));

# define	_SC_ARG_MAX	0
# define	_SC_CHILD_MAX	1
# define	_SC_CLK_TCK	2
# define	_SC_NGROUPS_MAX	3
# define	_SC_OPEN_MAX	4
/* no _SC_STREAM_MAX */
# define	_SC_JOB_CONTROL	5
# define	_SC_SAVED_IDS	6
# define	_SC_VERSION	7
# define        _SC_PAGESIZE    8

# define	_PC_LINK_MAX	0
# define	_PC_MAX_CANON	1
# define	_PC_MAX_INPUT	2
# define	_PC_NAME_MAX	3
# define	_PC_PATH_MAX	4
# define	_PC_PIPE_BUF	5
# define	_PC_CHOWN_RESTRICTED	6
# define	_PC_NO_TRUNC	7
# define	_PC_VDISABLE	8
# define	_PC_ASYNC_IO    9
# define	_PC_PRIO_IO     10
# define	_PC_SYNC_IO     11

# ifndef	_POSIX_SOURCE
#  define	MAXNAMLEN	1024
# endif		/* _POSIX_SOURCE */

/* FIXME: This is temporary until winsup gets sorted out.  */
#ifdef __CYGWIN__
#define MAXPATHLEN (260 - 1 /* NUL */)
#else
# define	MAXPATHLEN	1024
#endif

#ifdef __cplusplus
}
#endif
#endif /* _SYS_UNISTD_H */
