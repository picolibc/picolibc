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
#ifndef	_SYS_STAT_H
#define	_SYS_STAT_H

#include <sys/cdefs.h>
#include <sys/_types.h>
#include <sys/_timespec.h>

_BEGIN_STD_C

#ifndef _BLKCNT_T_DECLARED
typedef	__blkcnt_t	blkcnt_t;
#define	_BLKCNT_T_DECLARED
#endif

#ifndef _BLKSIZE_T_DECLARED
typedef	__blksize_t	blksize_t;
#define	_BLKSIZE_T_DECLARED
#endif

#ifndef _DEV_T_DECLARED
typedef	__dev_t		dev_t;		/* device number or struct cdev */
#define	_DEV_T_DECLARED
#endif

#ifndef _INO_T_DECLARED
typedef	__ino_t		ino_t;		/* inode number */
#define	_INO_T_DECLARED
#endif

#ifndef _MODE_T_DECLARED
typedef	__mode_t	mode_t;		/* permissions */
#define	_MODE_T_DECLARED
#endif

#ifndef _NLINK_T_DECLARED
typedef	__nlink_t	nlink_t;	/* link count */
#define	_NLINK_T_DECLARED
#endif

#ifndef _UID_T_DECLARED
typedef	__uid_t		uid_t;		/* user id */
#define	_UID_T_DECLARED
#endif

#ifndef _GID_T_DECLARED
typedef	__gid_t		gid_t;		/* group id */
#define	_GID_T_DECLARED
#endif

#ifndef _OFF_T_DECLARED
typedef	__off_t		off_t;		/* file offset */
#define	_OFF_T_DECLARED
#endif

#ifndef _TIME_T_DECLARED
typedef	_TIME_T_	time_t;
#define	_TIME_T_DECLARED
#endif

/* dj's stat defines _STAT_H_ */
#ifndef _STAT_H_

/* It is intended that the layout of this structure not change when the
   sizes of any of the basic types change (short, int, long) [via a compile
   time option].  */

struct	stat 
{
  dev_t		st_dev;
  ino_t		st_ino;
  mode_t	st_mode;
  nlink_t	st_nlink;
  uid_t		st_uid;
  gid_t		st_gid;
#if defined(__linux) && defined(__x86_64__)
    int __pad0;
#endif
  dev_t		st_rdev;
#if defined(__linux) && !defined(__x86_64__)
    unsigned short int __pad2;
#endif
  off_t		st_size;
#if defined(__linux)
  blksize_t	st_blksize;
  blkcnt_t	st_blocks;
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
#if defined(__linux) && defined(__x86_64__)
    __uint64_t __glibc_reserved[3];
#endif
#else
#if defined(__rtems__)
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  blksize_t     st_blksize;
  blkcnt_t	st_blocks;
#else
  /* SysV/sco doesn't have the rest... But Solaris, eabi does.  */
#if defined(__svr4__) && !defined(__PPC__) && !defined(__sun__)
  time_t	st_atime;
  time_t	st_mtime;
  time_t	st_ctime;
#else
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  blksize_t     st_blksize;
  blkcnt_t	st_blocks;
#if !defined(__rtems__)
  long		st_spare4[2];
#endif
#endif
#endif
#endif
};

#if __LARGEFILE64_VISIBLE
struct stat64
{
  __dev_t st_dev;		/* Device.  */
#  ifdef __x86_64__
  __ino64_t st_ino;		/* File serial number.  */
  __nlink_t st_nlink;		/* Link count.  */
  __mode_t st_mode;		/* File mode.  */
#  else
  unsigned int __pad1;
  __ino_t __st_ino;			/* 32bit file serial number.	*/
  __mode_t st_mode;			/* File mode.  */
  __nlink_t st_nlink;			/* Link count.  */
#  endif
  __uid_t st_uid;		/* User ID of the file's owner.	*/
  __gid_t st_gid;		/* Group ID of the file's group.*/
#  ifdef __x86_64__
  int __pad0;
  __dev_t st_rdev;		/* Device number, if device.  */
  __off_t st_size;		/* Size of file, in bytes.  */
#  else
  __dev_t st_rdev;			/* Device number, if device.  */
  unsigned int __pad2;
  __off64_t st_size;			/* Size of file, in bytes.  */
#  endif
  __blksize_t st_blksize;	/* Optimal block size for I/O.  */
  __blkcnt64_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
#if defined(__svr4__) && !defined(__PPC__) && !defined(__sun__)
  __time_t st_atime;			/* Time of last access.  */
  __syscall_ulong_t st_atimensec;	/* Nscecs of last access.  */
  __time_t st_mtime;			/* Time of last modification.  */
  __syscall_ulong_t st_mtimensec;	/* Nsecs of last modification.  */
  __time_t st_ctime;			/* Time of last status change.  */
  __syscall_ulong_t st_ctimensec;	/* Nsecs of last status change.  */
#else
  /* Nanosecond resolution timestamps are stored in a format
     equivalent to 'struct timespec'.  This is the type used
     whenever possible but the Unix namespace rules do not allow the
     identifier 'timespec' to appear in the <sys/stat.h> header.
     Therefore we have to handle the use of this header in strictly
     standard-compliant sources special.  */
  struct timespec st_atim;		/* Time of last access.  */
  struct timespec st_mtim;		/* Time of last modification.  */
  struct timespec st_ctim;		/* Time of last status change.  */
#  endif
#  ifdef __x86_64__
  __int64_t __glibc_reserved[3];
#  else
  __ino64_t st_ino;			/* File serial number.		*/
#  endif
};
#endif

#if !(defined(__svr4__) && !defined(__PPC__) && !defined(__sun__))
#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec
#endif


#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */
#define	S_ISVTX		0001000	/* save swapped text even after use */
#if __BSD_VISIBLE
#define	S_IREAD		0000400	/* read permission, owner */
#define	S_IWRITE 	0000200	/* write permission, owner */
#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */
#endif	/* !_BSD_VISIBLE */

#define	S_IFMT		_IFMT
#define	S_IFDIR		_IFDIR
#define	S_IFCHR		_IFCHR
#define	S_IFBLK		_IFBLK
#define	S_IFREG		_IFREG
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
#define	S_IFIFO		_IFIFO

#ifdef _WIN32
/* The Windows header files define _S_ forms of these, so we do too
   for easier portability.  */
#define _S_IFMT		_IFMT
#define _S_IFDIR	_IFDIR
#define _S_IFCHR	_IFCHR
#define _S_IFIFO	_IFIFO
#define _S_IFREG	_IFREG
#define _S_IREAD	0000400
#define _S_IWRITE	0000200
#define _S_IEXEC	0000100
#endif

#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#if __BSD_VISIBLE
#define ACCESSPERMS (S_IRWXU | S_IRWXG | S_IRWXO) /* 0777 */
#define ALLPERMS (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */
#endif

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)

#if defined(__CYGWIN__) || defined(__rtems__)
/* Special tv_nsec values for futimens(2) and utimensat(2). */
#define UTIME_NOW	-2L
#define UTIME_OMIT	-1L
#endif

int	chmod (const char *__path, mode_t __mode );
int     fchmod (int __fd, mode_t __mode);
int	fstat (int __fd, struct stat *__sbuf );
int	mkdir (const char *_path, mode_t __mode );
int	mkfifo (const char *__path, mode_t __mode );
int	stat (const char *__restrict __path, struct stat *__restrict __sbuf );
mode_t	umask (mode_t __mask );

#if __LARGEFILE64_VISIBLE
int	stat64 (const char *__restrict __path, struct stat64 *__restrict __sbuf );
int	fstat64 (int __fd, struct stat64 *__sbuf );
#endif

#if defined (__SPU__) || defined(__rtems__) || defined(__CYGWIN__)
int	lstat (const char *__restrict __path, struct stat *__restrict __buf );
int	mknod (const char *__path, mode_t __mode, dev_t __dev );
#endif

#if __ATFILE_VISIBLE
int	fchmodat (int, const char *, mode_t, int);
int	fstatat (int, const char *__restrict , struct stat *__restrict, int);
int	mkdirat (int, const char *, mode_t);
int	mkfifoat (int, const char *, mode_t);
int	mknodat (int, const char *, mode_t, dev_t);
int	utimensat (int, const char *, const struct timespec [2], int);
#endif
#if __POSIX_VISIBLE >= 200809
int	futimens (int, const struct timespec [2]);
#endif

#endif /* !_STAT_H_ */

_END_STD_C

#endif /* _SYS_STAT_H */
