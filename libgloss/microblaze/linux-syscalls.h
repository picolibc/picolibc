/** Linux system call interface for the MicroBlaze processor.
 * Copyright (c) 2009 Edgar E. Iglesias.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#define SYS_exit	1
#define SYS_fork	2
#define SYS_read	3
#define SYS_write	4
#define SYS_open	5
#define SYS_close	6
#define SYS_waitpid	7
#define SYS_creat	8
#define SYS_link	9
#define SYS_unlink	10
#define SYS_execve	11
#define SYS_chdir	12
#define SYS_time	13

#define SYS_lseek	19
#define SYS_getpid	20
#define SYS_kill	37
#define SYS_brk		45
#define SYS_fstat	108

#define SYS_rt_sigaction  174
