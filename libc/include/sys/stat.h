/*
Copyright (c) 1982, 1986, 1993
The Regents of the University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/cdefs.h>
#include <sys/_types.h>
#include <sys/_timespec.h>

_BEGIN_STD_C

#ifndef _BLKCNT_T_DECLARED
typedef __blkcnt_t blkcnt_t;
#define _BLKCNT_T_DECLARED
#endif

#ifndef _BLKSIZE_T_DECLARED
typedef __blksize_t blksize_t;
#define _BLKSIZE_T_DECLARED
#endif

#ifndef _DEV_T_DECLARED
typedef __dev_t dev_t; /* device number or struct cdev */
#define _DEV_T_DECLARED
#endif

#ifndef _INO_T_DECLARED
typedef __ino_t ino_t; /* inode number */
#define _INO_T_DECLARED
#endif

#ifndef _MODE_T_DECLARED
typedef __mode_t mode_t; /* permissions */
#define _MODE_T_DECLARED
#endif

#ifndef _NLINK_T_DECLARED
typedef __nlink_t nlink_t; /* link count */
#define _NLINK_T_DECLARED
#endif

#ifndef _UID_T_DECLARED
typedef __uid_t uid_t; /* user id */
#define _UID_T_DECLARED
#endif

#ifndef _GID_T_DECLARED
typedef __gid_t gid_t; /* group id */
#define _GID_T_DECLARED
#endif

#ifndef _OFF_T_DECLARED
typedef __off_t off_t; /* file offset */
#define _OFF_T_DECLARED
#endif

#ifndef _TIME_T_DECLARED
typedef __time_t time_t;
#define _TIME_T_DECLARED
#endif

struct stat {
    dev_t           st_dev;
    ino_t           st_ino;
    mode_t          st_mode;
    nlink_t         st_nlink;
    uid_t           st_uid;
    gid_t           st_gid;
    dev_t           st_rdev;
    off_t           st_size;
    blksize_t       st_blksize;
    blkcnt_t        st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
};

#if __BSD_VISIBLE
#define _major_dev_shift ((sizeof(dev_t) >> 1) << 3)
#define major(d)         ((d) >> _major_dev_shift)
#define minor(d)         ((d) & (((dev_t)1 << _major_dev_shift) - 1))
#define __make_dev(a, i) ((dev_t)(a) << _major_dev_shift | (i))
#endif

#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec

#define S_IFMT   0170000 /* type of file */
#define S_IFDIR  0040000 /* directory */
#define S_IFCHR  0020000 /* character special */
#define S_IFBLK  0060000 /* block special */
#define S_IFREG  0100000 /* regular */
#define S_IFLNK  0120000 /* symbolic link */
#define S_IFSOCK 0140000 /* socket */
#define S_IFIFO  0010000 /* fifo */

#define S_ISUID  0004000 /* set user id on execution */
#define S_ISGID  0002000 /* set group id on execution */
#define S_ISVTX  0001000 /* save swapped text even after use */
#if __BSD_VISIBLE
#define S_IREAD  0000400 /* read permission, owner */
#define S_IWRITE 0000200 /* write permission, owner */
#define S_IEXEC  0000100 /* execute/search permission, owner */
#define S_ENFMT  0002000 /* enforcement-mode locking */
#endif                   /* !_BSD_VISIBLE */

#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRUSR 0000400 /* read permission, owner */
#define S_IWUSR 0000200 /* write permission, owner */
#define S_IXUSR 0000100 /* execute/search permission, owner */
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRGRP 0000040 /* read permission, group */
#define S_IWGRP 0000020 /* write permission, grougroup */
#define S_IXGRP 0000010 /* execute/search permission, group */
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#define S_IROTH 0000004 /* read permission, other */
#define S_IWOTH 0000002 /* write permission, other */
#define S_IXOTH 0000001 /* execute/search permission, other */

#if __BSD_VISIBLE
#define ACCESSPERMS (S_IRWXU | S_IRWXG | S_IRWXO)                               /* 0777 */
#define ALLPERMS    (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */
#define S_BLKSIZE   512 /* blocksize for st_blocks */
#endif

#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

int    chmod(const char *__path, mode_t __mode);
int    fchmod(int __fd, mode_t __mode);
int    fstat(int __fd, struct stat *__sbuf);
int    mkdir(const char *_path, mode_t __mode);
int    mkfifo(const char *__path, mode_t __mode);
int    stat(const char    *__restrict __path, struct stat    *__restrict __sbuf);
mode_t umask(mode_t __mask);

#if defined(__SPU__) || defined(__rtems__) || defined(__CYGWIN__) || __POSIX_VISIBLE >= 200112L \
    || defined(__BSD_VISIBLE) || (_XOPEN_SOURCE - 0) >= 500
int lstat(const char * __restrict __path, struct stat * __restrict __buf);
#endif

#if defined(__SPU__) || defined(__rtems__) || defined(__CYGWIN__) || defined(__BSD_VISIBLE) \
    || (_XOPEN_SOURCE - 0) >= 500 || __SVID_VISIBLE
int mknod(const char *__path, mode_t __mode, dev_t __dev);
#endif

#if __ATFILE_VISIBLE
int fchmodat(int, const char *, mode_t, int);
int fstatat(int, const char * __restrict, struct stat * __restrict, int);
int mkdirat(int, const char *, mode_t);
int mkfifoat(int, const char *, mode_t);
int mknodat(int, const char *, mode_t, dev_t);
int utimensat(int, const char *, const struct timespec[2], int);
#endif

#if __POSIX_VISIBLE >= 200809
int futimens(int, const struct timespec[2]);
#endif

_END_STD_C

#endif /* _SYS_STAT_H */
