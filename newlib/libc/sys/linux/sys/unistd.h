#ifndef _UNISTD_H
#define _UNISTD_H

#include <features.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <_ansi.h>
#include <sys/types.h>
#include <sys/_types.h>
#define __need_size_t
#define __need_ptrdiff_t
#include <stddef.h>

extern char **environ;

void	_exit (int __status) _ATTRIBUTE ((__noreturn__));

int	access (const char *__path, int __amode);
unsigned  alarm (unsigned __secs);
int     chdir (const char *__path);
int     chmod (const char *__path, mode_t __mode);
int     chown (const char *__path, uid_t __owner, gid_t __group);
int     chroot (const char *__path);
int     close (int __fildes);
char    *ctermid (char *__s);
char    *cuserid (char *__s);
int     dup (int __fildes);
int     dup2 (int __fildes, int __fildes2);
int     execl (const char *__path, const char *, ...);
int     execle (const char *__path, const char *, ...);
int     execlp (const char *__file, const char *, ...);
int     execv (const char *__path, char * const __argv[]);
int     execve (const char *__path, char * const __argv[], char * const __envp[]);
int     execvp (const char *__file, char * const __argv[]);
int     fchdir (int __fildes);
int     fchmod (int __fildes, mode_t __mode);
int     fchown (int __fildes, uid_t __owner, gid_t __group);
pid_t   fork (void);
long    fpathconf (int __fd, int __name);
int     fsync (int __fd);
int     ftruncate (int __fd, off_t __length);
char    *getcwd (char *__buf, size_t __size);
int	getdomainname  (char *__name, size_t __len);
gid_t   getegid (void);
uid_t   geteuid (void);
gid_t   getgid (void);
int     getgroups (int __gidsetsize, gid_t __grouplist[]);
int 	__gethostname (char *__name, size_t __len);
char    *getlogin (void);
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
int getlogin_r (char *name, size_t namesize) ;
#endif
char 	*getpass (__const char *__prompt);
int  getpagesize (void);
pid_t   getpgid (pid_t);
pid_t   getpgrp (void);
pid_t   getpid (void);
pid_t   getppid (void);
uid_t   getuid (void);
char *	getusershell (void);
char    *getwd (char *__buf);
int     isatty (int __fildes);
int     lchown (const char *__path, uid_t __owner, gid_t __group);
int     link (const char *__path1, const char *__path2);
int	nice (int __nice_value);
off_t   lseek (int __fildes, off_t __offset, int __whence);
long    pathconf (const char *__path, int __name);
int     pause (void);
int     pipe (int __fildes[2]);
ssize_t pread (int __fd, void *__buf, size_t __nbytes, off_t __offset);
ssize_t pwrite (int __fd, const void *__buf, size_t __nbytes, off_t __offset);
_READ_WRITE_RETURN_TYPE read (int __fd, void *__buf, size_t __nbyte);
int     readlink (const char *path, char *buf, size_t bufsiz);
int     rmdir (const char *__path);
void *  sbrk (ptrdiff_t __incr);
int     setegid (gid_t __gid);
int     seteuid (uid_t __uid);
int     setgid (gid_t __gid);
int     setpgid (pid_t __pid, pid_t __pgid);
int     setpgrp (void);
pid_t   setsid (void);
int     setuid (uid_t __uid);
unsigned sleep (unsigned int __seconds);
void    swab (const void *, void *, ssize_t);
int     symlink (const char *oldpath, const char *newpath);
long    sysconf (int __name);
pid_t   tcgetpgrp (int __fildes);
int     tcsetpgrp (int __fildes, pid_t __pgrp_id);
int     truncate (const char *, off_t __length);
char *  ttyname (int __fildes);
int     ttyname_r (int __fildes, char *__buf, size_t __len);
int     unlink (const char *__path);
int     usleep (__useconds_t __useconds);
int     vhangup (void);
_READ_WRITE_RETURN_TYPE write (int __fd, const void *__buf, size_t __nbyte);

extern char *optarg;			/* getopt(3) external variables */
extern int optind, opterr, optopt;
int	 getopt(int, char * const [], const char *);
extern int optreset;			/* getopt(3) external variable */

#ifndef        _POSIX_SOURCE
pid_t   vfork (void);

extern char *suboptarg;			/* getsubopt(3) external variable */
int	 getsubopt(char **, char * const *, char **);
#endif /* _POSIX_SOURCE */

/* Provide prototypes for most of the _<systemcall> names that are
   provided in newlib for some compilers.  */
int     _close (int __fildes);
pid_t   _fork (void);
pid_t   _getpid (void);
int     _link (const char *__path1, const char *__path2);
off_t   _lseek (int __fildes, off_t __offset, int __whence);
_READ_WRITE_RETURN_TYPE _read (int __fd, void *__buf, size_t __nbyte);
void *  _sbrk (size_t __incr);
int     _unlink (const char *__path);
_READ_WRITE_RETURN_TYPE _write (int __fd, const void *__buf, size_t __nbyte);
int     _execve (const char *__path, char * const __argv[], char * const __envp[]);

#define	F_OK	0
#define	R_OK	4
#define	W_OK	2
#define	X_OK	1

# define	SEEK_SET	0
# define	SEEK_CUR	1
# define	SEEK_END	2

#include <sys/features.h>

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */

#include <bits/environments.h>
#include <bits/confname.h>

# define        MAXPATHLEN      1024

#ifdef __cplusplus
}
#endif
#endif /* _SYS_UNISTD_H */
