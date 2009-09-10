/* Copyright (C) 2004, 2007 Free Software Foundation, Inc.
   This program is free software; you can redistribute it and/or modify
   it under the same terms as libgloss/syscall.h .

   Authors: Amit Bhor, Joern Rennecke .  */

#ifndef _ASM_ARC_UNISTD_H
#define _ASM_ARC_UNISTD_H

#ifndef __ASSEMBLY__
/*
 * This file contains the system call numbers.
 */
/* __NR<fun> names from linux have been replaced with SYS<fun> names for sim.
 */

#define SYS_exit		  1
#define SYS__exit SYS_exit
#define SYS_fork		  2
#define SYS_read		  3
#define SYS_write		  4
#define SYS_open		  5
#define SYS_close		  6
#define SYS_waitpid		  7
#define SYS_creat		  8
#define SYS_link		  9
#define SYS_unlink		 10
#define SYS_execve		 11
#define SYS_chdir		 12
#define SYS_time		 13
#define SYS_mknod		 14
#define SYS_chmod		 15
#define SYS_chown		 16
#define SYS_break		 17
#define SYS_oldstat		 18
#define SYS_lseek		 19
#define SYS_getpid		 20
#define SYS_mount		 21
#define SYS_umount		 22
#define SYS_setuid		 23
#define SYS_getuid		 24
#define SYS_stime		 25
#define SYS_ptrace		 26
#define SYS_alarm		 27
#define SYS_oldfstat		 28
#define SYS_pause		 29
#define SYS_utime		 30
#define SYS_stty		 31
#define SYS_gtty		 32
#define SYS_access		 33
#define SYS_nice		 34
#define SYS_ftime		 35
#define SYS_sync		 36
#define SYS_kill		 37
#define SYS_rename		 38
#define SYS_mkdir		 39
#define SYS_rmdir		 40
#define SYS_dup		 41
#define SYS_pipe		 42
#define SYS_times		 43
#define SYS_prof		 44
#define SYS_brk		 45
#define SYS_setgid		 46
#define SYS_getgid		 47
#define SYS_signal		 48
#define SYS_geteuid		 49
#define SYS_getegid		 50
#define SYS_acct		 51
#define SYS_umount2		 52
#define SYS_lock		 53
#define SYS_ioctl		 54
#define SYS_fcntl		 55
#define SYS_mpx		 56
#define SYS_setpgid		 57
#define SYS_ulimit		 58
#define SYS_oldolduname	 59
#define SYS_umask		 60
#define SYS_chroot		 61
#define SYS_ustat		 62
#define SYS_dup2		 63
#define SYS_getppid		 64
#define SYS_getpgrp		 65
#define SYS_setsid		 66
#define SYS_sigaction		 67
#define SYS_sgetmask		 68
#define SYS_ssetmask		 69
#define SYS_setreuid		 70
#define SYS_setregid		 71
#define SYS_sigsuspend		 72
#define SYS_sigpending		 73
#define SYS_sethostname	 74
#define SYS_setrlimit		 75
#define SYS_old_getrlimit	 76
#define SYS_getrusage		 77
#define SYS_gettimeofday	 78
#define SYS_settimeofday	 79
#define SYS_getgroups		 80
#define SYS_setgroups		 81
#define SYS_select		 82
#define SYS_symlink		 83
#define SYS_oldlstat		 84
#define SYS_readlink		 85
#define SYS_uselib		 86
#define SYS_swapon		 87
#define SYS_reboot		 88
#define SYS_readdir		 89
#define SYS_mmap		 90
#define SYS_munmap		 91
#define SYS_truncate		 92
#define SYS_ftruncate		 93
#define SYS_fchmod		 94
#define SYS_fchown		 95
#define SYS_getpriority	 96
#define SYS_setpriority	 97
#define SYS_profil		 98
#define SYS_statfs		 99
#define SYS_fstatfs		100
#define SYS_ioperm		101
#define SYS_socketcall		102
#define SYS_syslog		103
#define SYS_setitimer		104
#define SYS_getitimer		105
#define SYS_stat		106
#define SYS_lstat		107
#define SYS_fstat		108
#define SYS_olduname		109
#define SYS_iopl		110  /* not supported */
#define SYS_vhangup		111
#define SYS_idle		112 /* Obsolete */
#define SYS_vm86		113 /* not supported */
#define SYS_wait4		114
#define SYS_swapoff		115
#define SYS_sysinfo		116
#define SYS_ipc		117
#define SYS_fsync		118
#define SYS_sigreturn		119
#define SYS_clone		120
#define SYS_setdomainname	121
#define SYS_uname		122
#define SYS_cacheflush		123
#define SYS_adjtimex		124
#define SYS_mprotect		125
#define SYS_sigprocmask	126
#define SYS_create_module	127
#define SYS_init_module	128
#define SYS_delete_module	129
#define SYS_get_kernel_syms	130
#define SYS_quotactl		131
#define SYS_getpgid		132
#define SYS_fchdir		133
#define SYS_bdflush		134
#define SYS_sysfs		135
#define SYS_personality	136
#define SYS_afs_syscall	137 /* Syscall for Andrew File System */
#define SYS_setfsuid		138
#define SYS_setfsgid		139
#define SYS__llseek		140
#define SYS_getdents		141
#define SYS__newselect		142
#define SYS_flock		143
#define SYS_msync		144
#define SYS_readv		145
#define SYS_writev		146
#define SYS_getsid		147
#define SYS_fdatasync		148
#define SYS__sysctl		149
#define SYS_mlock		150
#define SYS_munlock		151
#define SYS_mlockall		152
#define SYS_munlockall		153
#define SYS_sched_setparam		154
#define SYS_sched_getparam		155
#define SYS_sched_setscheduler		156
#define SYS_sched_getscheduler		157
#define SYS_sched_yield		158
#define SYS_sched_get_priority_max	159
#define SYS_sched_get_priority_min	160
#define SYS_sched_rr_get_interval	161
#define SYS_nanosleep		162
#define SYS_mremap		163
#define SYS_setresuid		164
#define SYS_getresuid		165
#define SYS_query_module	167
#define SYS_poll		168
#define SYS_nfsservctl		169
#define SYS_setresgid		170
#define SYS_getresgid		171
#define SYS_prctl		172
#define SYS_rt_sigreturn	173
#define SYS_rt_sigaction	174
#define SYS_rt_sigprocmask	175
#define SYS_rt_sigpending	176
#define SYS_rt_sigtimedwait	177
#define SYS_rt_sigqueueinfo	178
#define SYS_rt_sigsuspend	179
#define SYS_pread		180
#define SYS_pwrite		181
#define SYS_lchown		182
#define SYS_getcwd		183
#define SYS_capget		184
#define SYS_capset		185
#define SYS_sigaltstack	186
#define SYS_sendfile		187
#define SYS_getpmsg		188	/* some people actually want streams */
#define SYS_putpmsg		189	/* some people actually want streams */
#define SYS_vfork		190
#define SYS_getrlimit		191
#define SYS_mmap2		192
#define SYS_truncate64		193
#define SYS_ftruncate64	194
#define SYS_stat64		195
#define SYS_lstat64		196
#define SYS_fstat64		197
#define SYS_chown32		198
#define SYS_getuid32		199
#define SYS_getgid32		200
#define SYS_geteuid32		201
#define SYS_getegid32		202
#define SYS_setreuid32		203
#define SYS_setregid32		204
#define SYS_getgroups32	205
#define SYS_setgroups32	206
#define SYS_fchown32		207
#define SYS_setresuid32	208
#define SYS_getresuid32	209
#define SYS_setresgid32	210
#define SYS_getresgid32	211
#define SYS_lchown32		212
#define SYS_setuid32		213
#define SYS_setgid32		214
#define SYS_setfsuid32		215
#define SYS_setfsgid32		216
#define SYS_pivot_root		217
#define SYS_getdents64		220
#define SYS_fcntl64            221
#define SYS_gettid		224

#endif	/* ! __ASSEMBLY__ */
#ifdef __ARC700__
#if 0
  			"trap_s 1 \n\t"
#endif
#define SYSCALL \
  			"trap0	\n\t"
#else
#define SYSCALL	\
  			"swi	\n\t"\
			"nop    \n\t"\
                        "nop    \n\t"
#endif /* __ARC700__ */
#define _syscall0(type, name)	                \
type _##name##_r (struct _reent *_r)	        \
{					        \
  long __res;					\
  __asm__ __volatile__ ("mov	r8, %1\n\t"	\
			SYSCALL			\
	                "mov	%0, r0"		\
			: "=r" (__res)		\
			: "i" (SYS_##name)	\
			: "cc", "r0", "r8");		\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {\
    __errno_r (_r) = -__res;				\
    __res = -1;						\
  }							\
  return (type)__res;					\
}

#define _syscall1(type, name, atype, a)	\
type _##name##_r (struct _reent *_r, atype a)				\
{									\
  long __res;								\
  __asm__ __volatile__ ("mov	r0, %2\n\t"				\
			"mov	r8, %1\n\t"				\
			SYSCALL						\
	                "mov	%0, r0"				        \
			: "=r" (__res)					\
			: "i" (SYS_##name),				\
			  "r" ((long)a)					\
			: "cc", "r0", "r8");				\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {		\
    __errno_r (_r)= -__res;						\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#define _syscall2(type, name, atype, a, btype, b)	\
type _##name##_r (struct _reent *_r, atype a, btype b)			\
{									\
  long __res;								\
  __asm__ __volatile__ ("mov	r1, %3\n\t"				\
  			"mov	r0, %2\n\t"				\
			"mov	r8, %1\n\t"				\
  			SYSCALL						\
	                "mov	%0, r0"				        \
			: "=r" (__res)					\
			: "i" (SYS_##name),				\
			  "r" ((long)a),				\
			  "r" ((long)b)					\
			: "cc", "r0", "r1", "r8");			\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {		\
    __errno_r (_r) = -__res;						\
    __res = -1;								\
  }									\
  return (type)__res;							\
}
#define _syscall3(type, name, atype, a, btype, b, ctype, c)	\
type _##name##_r (struct _reent *_r, atype a, btype b, ctype c)		\
{									\
  long __res;								\
  __asm__ __volatile__ (						\
			"mov	r2, %4\n\t"				\
			"mov	r1, %3\n\t"				\
  			"mov	r0, %2\n\t"				\
			"mov	r8, %1\n\t"				\
  			SYSCALL						\
	                "mov	%0, r0"				        \
			: "=r" (__res)					\
			: "i" (SYS_##name),				\
			  "r" ((long)a),				\
			  "r" ((long)b),				\
			  "r" ((long)c)					\
			: "cc", "r0", "r1", "r2", "r8");		\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {		\
    __errno_r (_r) = -__res;						\
    __res = -1;								\
  }									\
  return (type)__res;							\
}

#ifdef __KERNEL_SYSCALLS__

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */
static inline _syscall0(int,pause)
static inline _syscall0(int,sync)
static inline _syscall0(pid_t,setsid)
static inline _syscall3(int,write,int,fd,const char *,buf,off_t,count)
static inline _syscall3(int,read,int,fd,char *,buf,off_t,count)
static inline _syscall3(off_t,lseek,int,fd,off_t,offset,int,count)
static inline _syscall1(int,dup,int,fd)
//static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall1(int,close,int,fd)
static inline _syscall1(int,_exit,int,exitcode)
static inline _syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
static inline _syscall1(int,delete_module,const char *,name)

static inline pid_t wait(int * wait_stat)
{
	return waitpid(-1,wait_stat,0);
}

static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)

/*
 * The following two can't be eliminated yet - they rely on
 * specific conditions.
 */

#endif



#endif	/* _ASM_ARC_UNISTD_H */
