/* libc/sys/linux/sys/stat.h - Stat structure and macros */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <asm/stat.h>
#define __KERNEL__
#include <linux/stat.h>
#undef __KERNEL__

/* --- redundant stuff below --- */

#include <_ansi.h>
#include <sys/types.h>

int     _EXFUN(fstat,( int __fd, struct stat *__sbuf ));
int     _EXFUN(mkdir,( const char *_path, mode_t __mode ));
int     _EXFUN(mkfifo,( const char *__path, mode_t __mode ));
int     _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
mode_t  _EXFUN(umask,( mode_t __mask ));

#ifndef _POSIX_SOURCE
int     _EXFUN(fstat64,( int __fd, struct stat *__sbuf ));
int	_EXFUN(lstat,( const char *file_name, struct stat *buf));
int	_EXFUN(lstat64,( const char *file_name, struct stat *buf));
int     _EXFUN(stat64,( const char *__path, struct stat *__sbuf ));
#endif /* _POSIX_SOURCE */

#endif /* _SYS_STAT_H */
