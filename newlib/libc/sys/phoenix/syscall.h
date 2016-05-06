/* Copyright (c) 2016 Phoenix Systems
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

#ifndef PHOENIX_SYSCALL_H
#define PHOENIX_SYSCALL_H

#include <sys/_null.h>

#define	SYS_SBRK					0
#define	SYS_READ					1
#define	SYS_WRITE					2
#define SYS_OPEN					3
#define SYS_CLOSE					4
#define SYS_FORK					5
#define SYS_NANOSLEEP       		6
#define SYS_EXECVE					7
#define SYS_GETPID					8
#define SYS_EXIT					9
#define SYS_WAITPID					10
#define SYS_GETPPID					11
#define SYS_LSEEK					12
#define SYS_FSTAT					13
#define SYS_STAT					14
#define SYS_ISATTY					15
#define SYS_PIPE					16      
#define SYS_LSTAT					17
#define SYS_FCNTL					18
#define SYS_DUP2					19
#define SYS_SOCKET					20
#define SYS_CONNECT					21
#define SYS_SELECT					22
#define SYS_GETCWD					23
#define SYS_SETCWD					24
#define SYS_BIND					25
#define SYS_LISTEN					26
#define SYS_ACCEPT					27
#define SYS_GETDENTS				28  
#define SYS_MKDIR					29
#define SYS_CHMOD					30
#define SYS_IOCTL					31
#define SYS_RMDIR					32
#define SYS_MKNOD					33
#define SYS_FSYNC					34
#define SYS_FTRUNCATE				35
#define SYS_ACCESS					36
#define SYS_SENDTO					37
#define SYS_RECVFROM				38
#define SYS_GETSOCKNAME				39
#define SYS_GETPEERNAME				40
#define SYS_POLL					41
#define SYS_CLOCK_GETTIME			42
#define SYS_UNLINK      			43
#define SYS_LINK        			44
#define SYS_SYMLINK					45
#define SYS_READLINK				46
#define SYS_GETUID					47
#define SYS_SETUID					48
#define SYS_GETEUID					49
#define SYS_SETEUID					50
#define SYS_SETREUID				51
#define SYS_GETGID					52
#define SYS_SETGID					53
#define SYS_GETEGID					54
#define SYS_SETEGID					55
#define SYS_SETREGID				56
#define SYS_GETGROUPS				57
#define SYS_SETGROUPS				58
#define SYS_GETGRGID				59
#define SYS_UMASK					60
#define SYS_CHOWN					61
#define SYS_LCHOWN					62
#define SYS_FCHOWN					63
#define SYS_FCHMOD					64
#define SYS_TRUNCATE				65
#define SYS_SCHED_GETPARAM			66
#define SYS_SCHED_GETSCHEDULER		67
#define SYS_SCHED_GET_PRIORITY_MAX	68
#define SYS_SCHED_GET_PRIORITY_MIN	69
#define SYS_SCHED_SETPARAM			70
#define SYS_SCHED_SETSCHEDULER		71
#define SYS_SCHED_YIELD				72
#define SYS_MOUNT					73
#define SYS_UMOUNT					74
#define SYS_KILL					75
#define SYS_CLOCK_SETTIME			76
#define SYS_NET_CONFIGURE			77
#define SYS_UNAME					78
#define SYS_HOSTNAME				79
#define SYS_DOMAINNAME				80
#define SYS_CHTIMES					81
#define SYS_REBOOT					82
#define SYS_SYNC					83
#define SYS_STATFS					84
#define SYS_MMAP					85
#define SYS_MUNMAP					86

void *_syscall5(unsigned syscallNo, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);

#define syscall0(rettype, no)						(rettype) _syscall5(no, NULL, NULL, NULL, NULL,NULL)
#define syscall1(rettype, no, a1)					(rettype) _syscall5(no, (void *) a1, NULL, NULL, NULL, NULL)
#define syscall2(rettype, no, a1, a2)				(rettype) _syscall5(no, (void *) a1, (void *) a2, NULL, NULL, NULL)
#define syscall3(rettype, no, a1, a2, a3)			(rettype) _syscall5(no, (void *) a1, (void *) a2, (void *) a3, NULL, NULL)
#define syscall4(rettype, no, a1, a2, a3, a4)		(rettype) _syscall5(no, (void *) a1, (void *) a2, (void *) a3, (void *) a4, NULL)
#define syscall5(rettype, no, a1, a2, a3, a4, a5)	(rettype) _syscall5(no, (void *) a1, (void *) a2, (void *) a3, (void *) a4, (void *) a5)

#endif
